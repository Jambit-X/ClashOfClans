#include "BattleProcessController.h"
#include "../Layer/BattleTroopLayer.h"
#include "../Manager/VillageDataManager.h"
#include "../Model/BuildingConfig.h"
#include "../Util/GridMapUtils.h"
#include "../Util/FindPathUtil.h"
#include "2d/CCParticleExamples.h"
#include <cmath>
#include <set>
#include "../Sprite/BuildingSprite.h"
#include "../Component/DefenseBuildingAnimation.h"

USING_NS_CC;

BattleProcessController* BattleProcessController::_instance = nullptr;

// ==========================================
// 静态辅助函数
// ==========================================

static float calculatePathLength(const std::vector<Vec2>& path) {
    if (path.size() < 2) return 0.0f;
    float totalDist = 0.0f;
    for (size_t i = 0; i < path.size() - 1; ++i) {
        totalDist += path[i].distance(path[i + 1]);
    }
    return totalDist;
}

static int getDamageByUnitType(UnitTypeID typeID) {
    switch (typeID) {
        case UnitTypeID::BARBARIAN:
            return TroopConfig::getInstance()->getTroopById(1001).damagePerSecond;
        case UnitTypeID::ARCHER:
            return TroopConfig::getInstance()->getTroopById(1002).damagePerSecond;
        case UnitTypeID::GOBLIN:
            return TroopConfig::getInstance()->getTroopById(1003).damagePerSecond;
        case UnitTypeID::GIANT:
            return TroopConfig::getInstance()->getTroopById(1004).damagePerSecond;
        case UnitTypeID::WALL_BREAKER:
            return TroopConfig::getInstance()->getTroopById(1005).damagePerSecond;
        case UnitTypeID::BALLOON:
            return TroopConfig::getInstance()->getTroopById(1006).damagePerSecond;
        default:
            return TroopConfig::getInstance()->getTroopById(1001).damagePerSecond;
    }
}

static int getAttackRangeByUnitType(UnitTypeID typeID) {
    switch (typeID) {
        case UnitTypeID::ARCHER:
            return 3;
        case UnitTypeID::BARBARIAN:
        case UnitTypeID::GOBLIN:
        case UnitTypeID::GIANT:
            return 1;
        case UnitTypeID::BALLOON:
        case UnitTypeID::WALL_BREAKER:
            return 0;
        default:
            return 1;
    }
}

// ==========================================
// 生命周期管理
// ==========================================

BattleProcessController* BattleProcessController::getInstance() {
    if (!_instance) {
        _instance = new BattleProcessController();
    }
    return _instance;
}

void BattleProcessController::destroyInstance() {
    if (_instance) {
        delete _instance;
        _instance = nullptr;
    }
}

// ==========================================
// 战斗状态重置
// ==========================================

void BattleProcessController::resetBattleState() {
    auto dataManager = VillageDataManager::getInstance();
    auto& buildings = const_cast<std::vector<BuildingInstance>&>(dataManager->getAllBuildings());

    int restoredCount = 0;
    for (auto& building : buildings) {
        auto config = BuildingConfig::getInstance()->getConfig(building.type);
        int maxHP = (config && config->hitPoints > 0) ? config->hitPoints : 100;

        if (building.currentHP != maxHP || building.isDestroyed) {
            building.currentHP = maxHP;
            building.isDestroyed = false;
            restoredCount++;
        }

        // 清除防御建筑的锁定目标
        building.lockedTarget = nullptr;

        // ✅ 重置攻击冷却
        building.attackCooldown = 0.0f;
    }

    dataManager->saveToFile("village.json");
}
// ==========================================
// 核心攻击逻辑（提取公共代码）
// ==========================================

void BattleProcessController::executeAttack(
    BattleUnitSprite* unit,
    BattleTroopLayer* troopLayer,
    int targetID,
    bool isForcedTarget,
    const std::function<void()>& onTargetDestroyed,
    const std::function<void()>& onContinueAttack
) {
    auto dm = VillageDataManager::getInstance();
    BuildingInstance* liveTarget = dm->getBuildingById(targetID);

    // 防止重复处理
    if (unit->isChangingTarget()) {
        return;
    }

    // 目标已摧毁
    if (!liveTarget || liveTarget->isDestroyed || liveTarget->currentHP <= 0) {
        unit->setChangingTarget(true);
        onTargetDestroyed();
        
        unit->runAction(Sequence::create(
            DelayTime::create(0.1f),
            CallFunc::create([unit]() {
                unit->setChangingTarget(false);
            }),
            nullptr
        ));
        return;
    }

    // ========== ✅ 炸弹兵自爆特判 ==========
    if (unit->getUnitTypeID() == UnitTypeID::WALL_BREAKER) {
        CCLOG("BattleProcessController: Wall Breaker executing suicide attack");

        // 执行自爆攻击
        performWallBreakerSuicideAttack(unit, liveTarget, troopLayer, [onTargetDestroyed]() {
            // 炸弹兵死后，不继续攻击，直接调用目标摧毁回调
            // 注意：这里不管目标是否被摧毁，炸弹兵都已经死了
            if (onTargetDestroyed) {
                onTargetDestroyed();
            }
        });

        // 立即返回，不执行后续的普通攻击逻辑
        return;
    }

    // 计算伤害
    int dps = getDamageByUnitType(unit->getUnitTypeID());
    
    // 二次检查
    if (liveTarget->isDestroyed || liveTarget->currentHP <= 0) {
        onTargetDestroyed();
        return;
    }

    liveTarget->currentHP -= dps;

    // 目标被摧毁
    if (liveTarget->currentHP <= 0) {
        liveTarget->isDestroyed = true;
        liveTarget->currentHP = 0;
        
        FindPathUtil::getInstance()->updatePathfindingMap();
        
        // 【新增】发送建筑摧毁事件
        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(
            "EVENT_BUILDING_DESTROYED", 
            static_cast<void*>(liveTarget)
        );
        
        onTargetDestroyed();
    }
    else {
        onContinueAttack();
    }
}

// ==========================================
// 砍城墙时检查是否有更好的路径
// ==========================================

bool BattleProcessController::shouldAbandonWallForBetterPath(BattleUnitSprite* unit, int currentWallID) {
    Vec2 unitPos = unit->getPosition();
    
    const BuildingInstance* bestTarget = nullptr;
    if (unit->getUnitTypeID() == UnitTypeID::GOBLIN) {
        bestTarget = findTargetWithResourcePriority(unitPos, unit->getUnitTypeID());
    } else if (unit->getUnitTypeID() == UnitTypeID::GIANT || unit->getUnitTypeID() == UnitTypeID::BALLOON) {
        bestTarget = findTargetWithDefensePriority(unitPos, unit->getUnitTypeID());
    } else {
        bestTarget = findTargetWithResourcePriority(unitPos, unit->getUnitTypeID());
    }
    
    if (!bestTarget) return false;
    
    Vec2 targetCenter = GridMapUtils::gridToPixelCenter(bestTarget->gridX, bestTarget->gridY);
    
    auto pathfinder = FindPathUtil::getInstance();
    int attackRange = getAttackRangeByUnitType(unit->getUnitTypeID());
    std::vector<Vec2> pathAround = pathfinder->findPathToAttackBuilding(unitPos, *bestTarget, attackRange);
    
    if (pathAround.empty()) {
        return false;
    }
    
    float pathLength = calculatePathLength(pathAround);
    float directDist = unitPos.distance(targetCenter);
    float detourCost = pathLength - directDist;
    
    if (detourCost <= PIXEL_DETOUR_THRESHOLD && pathLength <= directDist * 2.0f) {
        return true;
    }
    
    return false;
}



// ==========================================
// 核心 AI 逻辑入口
// ==========================================

void BattleProcessController::startUnitAI(BattleUnitSprite* unit, BattleTroopLayer* troopLayer) {
    if (!unit) {
        return;
    }

    Vec2 unitPos = unit->getPosition();
    
    const BuildingInstance* target = nullptr;
    if (unit->getUnitTypeID() == UnitTypeID::GOBLIN) {
        target = findTargetWithResourcePriority(unitPos, unit->getUnitTypeID());
    } else if (unit->getUnitTypeID() == UnitTypeID::GIANT || unit->getUnitTypeID() == UnitTypeID::BALLOON) {
        target = findTargetWithDefensePriority(unitPos, unit->getUnitTypeID());
    } else {
        target = findTargetWithResourcePriority(unitPos, unit->getUnitTypeID());
    }

    if (!target) {
        unit->playIdleAnimation();
        return;
    }

    // 【新增】发送目标锁定事件，通知显示 Beacon
    EventCustom event("EVENT_UNIT_TARGET_LOCKED");
    event.setUserData(reinterpret_cast<void*>(static_cast<intptr_t>(target->id)));
    Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);

    auto pathfinder = FindPathUtil::getInstance();
    Vec2 targetCenter = GridMapUtils::gridToPixelCenter(target->gridX, target->gridY);

    int attackRange = getAttackRangeByUnitType(unit->getUnitTypeID());
    
    std::vector<Vec2> pathAround = pathfinder->findPathToAttackBuilding(unitPos, *target, attackRange);
    float distAround = calculatePathLength(pathAround);
    float distDirect = unitPos.distance(targetCenter);

    if (!pathAround.empty()) {
        float detourCost = distAround - distDirect;
        
        if (detourCost <= PIXEL_DETOUR_THRESHOLD && distAround <= distDirect * 2.0f) {
            unit->followPath(pathAround, 100.0f, [this, unit, troopLayer]() {
                startCombatLoop(unit, troopLayer);
            });
            return;
        }
    }

    const BuildingInstance* wallToBreak = getFirstWallInLine(unitPos, targetCenter);

    if (wallToBreak && unit->getUnitTypeID() != UnitTypeID::BALLOON) {
        std::vector<Vec2> pathToWall = pathfinder->findPathToAttackBuilding(unitPos, *wallToBreak, attackRange);

        if (pathToWall.empty()) {
            startCombatLoopWithForcedTarget(unit, troopLayer, wallToBreak);
        }
        else {
            unit->followPath(pathToWall, 100.0f, [this, unit, troopLayer, wallToBreak]() {
                startCombatLoopWithForcedTarget(unit, troopLayer, wallToBreak);
            });
        }
    }
    else {
        std::vector<Vec2> directPath = { targetCenter };
        unit->followPath(directPath, 100.0f, [this, unit, troopLayer]() {
            startCombatLoop(unit, troopLayer);
        });
    }
}

// ==========================================
// 辅助方法
// ==========================================

const BuildingInstance* BattleProcessController::getFirstWallInLine(const Vec2& startPixel, const Vec2& endPixel) {
    auto dataManager = VillageDataManager::getInstance();
    auto pathfinder = FindPathUtil::getInstance();

    std::vector<Vec2> throughPath = pathfinder->findPathIgnoringWalls(startPixel, endPixel);
    if (!throughPath.empty()) {
        for (const auto& worldPt : throughPath) {
            Vec2 gridF = GridMapUtils::pixelToGrid(worldPt);
            int gx = static_cast<int>(std::floor(gridF.x + 0.5f));
            int gy = static_cast<int>(std::floor(gridF.y + 0.5f));

            BuildingInstance* b = dataManager->getBuildingAtGrid(gx, gy);
            if (b && b->type == 303 && !b->isDestroyed && b->currentHP > 0) {
                return b;
            }
        }
    }

    Vec2 startGrid = GridMapUtils::pixelToGrid(startPixel);
    Vec2 endGrid = GridMapUtils::pixelToGrid(endPixel);
    Vec2 diff = endGrid - startGrid;
    int steps = std::max((int)std::abs(diff.x), (int)std::abs(diff.y));
    if (steps == 0) return nullptr;
    
    Vec2 direction = diff / static_cast<float>(steps);
    Vec2 current = startGrid;
    
    for (int i = 0; i <= steps; ++i) {
        int gx = static_cast<int>(std::round(current.x));
        int gy = static_cast<int>(std::round(current.y));

        BuildingInstance* b = dataManager->getBuildingAtGrid(gx, gy);
        if (b && b->type == 303 && !b->isDestroyed && b->currentHP > 0) {
            return b;
        }
        current += direction;
    }

    return nullptr;
}

static bool isResourceBuilding(int buildingType) {
    return buildingType == 1 || buildingType == 202 || buildingType == 203 || 
           buildingType == 204 || buildingType == 205;
}

static bool isDefenseBuilding(int buildingType) {
    return buildingType == 301 || buildingType == 302;
}

const BuildingInstance* BattleProcessController::findTargetWithResourcePriority(const Vec2& unitWorldPos, UnitTypeID unitType) {
    auto dataManager = VillageDataManager::getInstance();
    const auto& buildings = dataManager->getAllBuildings();

    if (buildings.empty()) return nullptr;

    const BuildingInstance* bestBuilding = nullptr;
    const BuildingInstance* fallbackBuilding = nullptr;
    float minDistanceSq = FLT_MAX;
    float fallbackMinDistanceSq = FLT_MAX;

    for (const auto& building : buildings) {
        if (building.isDestroyed || building.currentHP <= 0) continue;
        if (building.state == BuildingInstance::State::PLACING) continue;
        
        // 跳过城墙（303）和陷阱（4xx）
        if (building.type == 303) continue;
        if (building.type >= 400 && building.type < 500) continue;

        auto config = BuildingConfig::getInstance()->getConfig(building.type);
        if (!config) continue;

        Vec2 bPos = GridMapUtils::gridToPixelCenter(building.gridX, building.gridY);
        float distSq = unitWorldPos.distanceSquared(bPos);

        if (unitType == UnitTypeID::GOBLIN) {
            if (isResourceBuilding(building.type)) {
                if (distSq < minDistanceSq) {
                    minDistanceSq = distSq;
                    bestBuilding = &building;
                }
            } else {
                if (distSq < fallbackMinDistanceSq) {
                    fallbackMinDistanceSq = distSq;
                    fallbackBuilding = &building;
                }
            }
        } else {
            if (distSq < minDistanceSq) {
                minDistanceSq = distSq;
                bestBuilding = &building;
            }
        }
    }

    if (unitType == UnitTypeID::GOBLIN && !bestBuilding && fallbackBuilding) {
        return fallbackBuilding;
    }

    return bestBuilding;
}

const BuildingInstance* BattleProcessController::findTargetWithDefensePriority(const Vec2& unitWorldPos, UnitTypeID unitType) {
    auto dataManager = VillageDataManager::getInstance();
    const auto& buildings = dataManager->getAllBuildings();

    if (buildings.empty()) return nullptr;

    const BuildingInstance* bestBuilding = nullptr;
    const BuildingInstance* fallbackBuilding = nullptr;
    float minDistanceSq = FLT_MAX;
    float fallbackMinDistanceSq = FLT_MAX;

    for (const auto& building : buildings) {
        if (building.isDestroyed || building.currentHP <= 0) continue;
        if (building.state == BuildingInstance::State::PLACING) continue;
        
        // 跳过城墙（303）和陷阱（4xx）
        if (building.type == 303) continue;
        if (building.type >= 400 && building.type < 500) continue;
        
        // 气球兵额外跳过城墙（已在上面处理，这里保留代码完整性）
        if (unitType == UnitTypeID::BALLOON && building.type == 303) continue;

        auto config = BuildingConfig::getInstance()->getConfig(building.type);
        if (!config) continue;

        Vec2 bPos = GridMapUtils::gridToPixelCenter(building.gridX, building.gridY);
        float distSq = unitWorldPos.distanceSquared(bPos);

        if (unitType == UnitTypeID::GIANT || unitType == UnitTypeID::BALLOON) {
            if (isDefenseBuilding(building.type)) {
                if (distSq < minDistanceSq) {
                    minDistanceSq = distSq;
                    bestBuilding = &building;
                }
            } else {
                if (building.type != 303) {
                    if (distSq < fallbackMinDistanceSq) {
                        fallbackMinDistanceSq = distSq;
                        fallbackBuilding = &building;
                    }
                }
            }
        } else {
            if (distSq < minDistanceSq) {
                minDistanceSq = distSq;
                bestBuilding = &building;
            }
        }
    }

    if ((unitType == UnitTypeID::GIANT || unitType == UnitTypeID::BALLOON) && !bestBuilding && fallbackBuilding) {
        return fallbackBuilding;
    }

    return bestBuilding;
}

const BuildingInstance* BattleProcessController::findNearestBuilding(const Vec2& unitWorldPos, UnitTypeID unitType) {
    if (unitType == UnitTypeID::GOBLIN) {
        return findTargetWithResourcePriority(unitWorldPos, unitType);
    } else if (unitType == UnitTypeID::GIANT || unitType == UnitTypeID::BALLOON) {
        return findTargetWithDefensePriority(unitWorldPos, unitType);
    } else {
        return findTargetWithResourcePriority(unitWorldPos, unitType);
    }
}

// ==========================================
// 建筑防御系统 - 查找攻击范围内的兵种
// ==========================================

BattleUnitSprite* BattleProcessController::findNearestUnitInRange(
    const BuildingInstance& building, 
    float attackRangeGrids,
    BattleTroopLayer* troopLayer) {
    
    if (!troopLayer) return nullptr;
    
    auto config = BuildingConfig::getInstance()->getConfig(building.type);
    if (!config) return nullptr;
    
    int centerX = building.gridX + config->gridWidth / 2;
    int centerY = building.gridY + config->gridHeight / 2;
    
    auto allUnits = troopLayer->getAllUnits();
    if (allUnits.empty()) return nullptr;
    
    BattleUnitSprite* nearestUnit = nullptr;
    int minGridDistance = INT_MAX;
    int attackRangeInt = static_cast<int>(attackRangeGrids);
    
    for (auto unit : allUnits) {
        if (!unit) continue;
        
        Vec2 unitGridPos = unit->getGridPosition();
        int unitGridX = static_cast<int>(unitGridPos.x);
        int unitGridY = static_cast<int>(unitGridPos.y);
        
        int gridDistance = std::max(
            std::abs(unitGridX - centerX),
            std::abs(unitGridY - centerY)
        );
        
        if (gridDistance <= attackRangeInt) {
            if (gridDistance < minGridDistance) {
                minGridDistance = gridDistance;
                nearestUnit = unit;
            }
        }
    }
    
    if (nearestUnit) {
        nearestUnit->setTargetedByBuilding(true);
    }
    
    return nearestUnit;
}

std::vector<BattleUnitSprite*> BattleProcessController::getAllUnitsInRange(
    const BuildingInstance& building, 
    float attackRangeGrids,
    BattleTroopLayer* troopLayer) {
    
    std::vector<BattleUnitSprite*> unitsInRange;
    
    if (!troopLayer) return unitsInRange;
    
    auto config = BuildingConfig::getInstance()->getConfig(building.type);
    if (!config) return unitsInRange;
    
    int centerX = building.gridX + config->gridWidth / 2;
    int centerY = building.gridY + config->gridHeight / 2;
    
    auto allUnits = troopLayer->getAllUnits();
    int attackRangeInt = static_cast<int>(attackRangeGrids);
    
    for (auto unit : allUnits) {
        if (!unit) continue;
        
        Vec2 unitGridPos = unit->getGridPosition();
        int unitGridX = static_cast<int>(unitGridPos.x);
        int unitGridY = static_cast<int>(unitGridPos.y);
        
        int gridDistance = std::max(
            std::abs(unitGridX - centerX),
            std::abs(unitGridY - centerY)
        );
        
        if (gridDistance <= attackRangeInt) {
            unitsInRange.push_back(unit);
        }
    }
    
    return unitsInRange;
}

// ==========================================
// 建筑防御自动更新系统
// ==========================================

void BattleProcessController::updateBuildingDefense(BattleTroopLayer* troopLayer) {
    if (!troopLayer) return;

    auto dataManager = VillageDataManager::getInstance();
    auto& buildings = const_cast<std::vector<BuildingInstance>&>(dataManager->getAllBuildings());

    std::set<BattleUnitSprite*> targetedUnitsThisFrame;
    float deltaTime = Director::getInstance()->getDeltaTime();

    for (auto& building : buildings) {
        // 跳过非防御建筑
        if (building.isDestroyed || building.currentHP <= 0) continue;
        if (building.state == BuildingInstance::State::PLACING) continue;
        if (building.type != 301 && building.type != 302) continue;

        auto config = BuildingConfig::getInstance()->getConfig(building.type);
        if (!config) continue;

        float attackRange = config->attackRange;
        float attackSpeed = config->attackSpeed;

        BattleUnitSprite* currentTarget = static_cast<BattleUnitSprite*>(building.lockedTarget);

        // ========== 目标有效性检查 ==========
        bool targetValid = false;

        if (currentTarget) {
            auto allUnits = troopLayer->getAllUnits();

            // 检查1: 目标是否还存活
            for (auto unit : allUnits) {
                if (unit == currentTarget && !unit->isDead()) {
                    targetValid = true;
                    break;
                }
            }

            // 检查2: 是否还在范围内
            if (targetValid) {
                int centerX = building.gridX + config->gridWidth / 2;
                int centerY = building.gridY + config->gridHeight / 2;

                Vec2 unitGridPos = currentTarget->getGridPosition();
                int gridDistance = std::max(
                    std::abs((int)unitGridPos.x - centerX),
                    std::abs((int)unitGridPos.y - centerY)
                );

                if (gridDistance > static_cast<int>(attackRange)) {
                    targetValid = false;
                }
            }

            // 目标无效，清除锁定
            if (!targetValid) {
                building.lockedTarget = nullptr;
                currentTarget = nullptr;
            }
        }

        // ========== 寻找新目标 ==========
        if (!currentTarget) {
            BattleUnitSprite* newTarget = findNearestUnitInRange(building, attackRange, troopLayer);
            if (newTarget && !newTarget->isDead()) {
                building.lockedTarget = static_cast<void*>(newTarget);
                currentTarget = newTarget;
                building.attackCooldown = 0.0f;
            }
        }

        // ========== 攻击逻辑 ==========
        if (currentTarget) {
            targetedUnitsThisFrame.insert(currentTarget);

            building.attackCooldown -= deltaTime;

            if (building.attackCooldown <= 0.0f) {
                int damagePerShot = static_cast<int>(config->damagePerSecond * attackSpeed);
                currentTarget->takeDamage(damagePerShot);

                // ✅✅✅ 关键修复：转换到 MapLayer 坐标系
                auto mapLayer = troopLayer->getParent();
                if (mapLayer) {
                    std::string spriteName = "Building_" + std::to_string(building.id);
                    auto buildingSprite = dynamic_cast<BuildingSprite*>(mapLayer->getChildByName(spriteName));

                    if (buildingSprite) {
                        auto defenseAnim = dynamic_cast<DefenseBuildingAnimation*>(
                            buildingSprite->getChildByName("DefenseAnim")
                            );

                        if (defenseAnim) {
                            // ✅ 核心修复：转换到 MapLayer 坐标系（而非世界坐标）
                            // 1. 获取兵种在 TroopLayer 中的位置
                            Vec2 unitPosInTroopLayer = currentTarget->getPosition();

                            // 2. 转换到 MapLayer 坐标系
                            Vec2 targetPosInMapLayer = troopLayer->convertToNodeSpace(
                                troopLayer->getParent()->convertToWorldSpace(unitPosInTroopLayer)
                            );

                            // 更简洁的写法（推荐）
                            // Vec2 targetPosInMapLayer = mapLayer->convertToNodeSpace(
                            //     troopLayer->convertToWorldSpace(currentTarget->getPosition())
                            // );

                            CCLOG("BattleProcessController: Aiming at target - Unit pos in TroopLayer: (%.1f, %.1f), Target pos in MapLayer: (%.1f, %.1f)",
                                  unitPosInTroopLayer.x, unitPosInTroopLayer.y,
                                  targetPosInMapLayer.x, targetPosInMapLayer.y);

                            defenseAnim->playAttackAnimation(targetPosInMapLayer);
                        }
                    }
                }

                building.attackCooldown = attackSpeed;

                // ✅ 修复：目标死亡，立即清除锁定并停止所有动作
                if (currentTarget->isDead()) {
                    // 1. 立即清除建筑的锁定引用
                    building.lockedTarget = nullptr;
                    targetedUnitsThisFrame.erase(currentTarget);

                    // 2. 恢复兵种颜色
                    currentTarget->setTargetedByBuilding(false);

                    // 3. 停止兵种所有正在进行的动作（包括攻击动画）
                    currentTarget->stopAllActions();

                    // 4. 播放死亡动画
                    currentTarget->playDeathAnimation([troopLayer, currentTarget]() {
                        troopLayer->removeUnit(currentTarget);
                    });

                    CCLOG("BattleProcessController: Unit killed, stopped all actions and playing death animation");
                }
            }
        }
    }

    // ========== 更新兵种锁定状态 ==========
    auto allUnits = troopLayer->getAllUnits();
    for (auto unit : allUnits) {
        if (!unit || unit->isDead()) continue;

        bool shouldBeTargeted = (targetedUnitsThisFrame.find(unit) != targetedUnitsThisFrame.end());
        if (unit->isTargetedByBuilding() != shouldBeTargeted) {
            unit->setTargetedByBuilding(shouldBeTargeted);
        }
    }
}

// ==========================================
// 战斗循环逻辑
// ==========================================

void BattleProcessController::startCombatLoop(BattleUnitSprite* unit, BattleTroopLayer* troopLayer) {
    if (!unit || !troopLayer) return;

    Vec2 unitPos = unit->getPosition();
    auto dm = VillageDataManager::getInstance();
    const BuildingInstance* target = findNearestBuilding(unitPos, unit->getUnitTypeID());

    if (!target) {
        unit->playIdleAnimation();
        return;
    }

    BuildingInstance* mutableTarget = dm->getBuildingById(target->id);
    if (!mutableTarget || mutableTarget->isDestroyed || mutableTarget->currentHP <= 0) {
        startUnitAI(unit, troopLayer);
        return;
    }

    auto config = BuildingConfig::getInstance()->getConfig(mutableTarget->type);
    if (!config) {
        startUnitAI(unit, troopLayer);
        return;
    }

    Vec2 unitGridPos = GridMapUtils::pixelToGrid(unitPos);
    int unitGridX = static_cast<int>(std::floor(unitGridPos.x));
    int unitGridY = static_cast<int>(std::floor(unitGridPos.y));

    int bX = mutableTarget->gridX;
    int bY = mutableTarget->gridY;
    int bW = config->gridWidth;
    int bH = config->gridHeight;

    int gridDistX = 0;
    int gridDistY = 0;

    if (unitGridX < bX) {
        gridDistX = bX - unitGridX;
    } else if (unitGridX >= bX + bW) {
        gridDistX = unitGridX - (bX + bW) + 1;
    }

    if (unitGridY < bY) {
        gridDistY = bY - unitGridY;
    } else if (unitGridY >= bY + bH) {
        gridDistY = unitGridY - (bY + bH) + 1;
    }

    int gridDistance = std::max(gridDistX, gridDistY);
    int attackRangeGrid = getAttackRangeByUnitType(unit->getUnitTypeID());
    
    if (gridDistance > attackRangeGrid) {
        startUnitAI(unit, troopLayer);
        return;
    }
    
    Vec2 buildingPos = GridMapUtils::gridToPixelCenter(mutableTarget->gridX, mutableTarget->gridY);
    int targetID = mutableTarget->id;
    
    unit->attackTowardPosition(buildingPos, [this, unit, troopLayer, targetID]() {
        executeAttack(unit, troopLayer, targetID, false,
            [this, unit, troopLayer]() { 
                startUnitAI(unit, troopLayer); 
            },
            [this, unit, troopLayer]() { 
                startCombatLoop(unit, troopLayer); 
            }
        );
    });
}

void BattleProcessController::startCombatLoopWithForcedTarget(BattleUnitSprite* unit, BattleTroopLayer* troopLayer, const BuildingInstance* forcedTarget) {
    if (!unit || !troopLayer || !forcedTarget) return;

    Vec2 unitPos = unit->getPosition();
    auto dm = VillageDataManager::getInstance();
    int targetID = forcedTarget->id;

    BuildingInstance* liveTarget = dm->getBuildingById(targetID);
    if (!liveTarget || liveTarget->isDestroyed || liveTarget->currentHP <= 0) {
        startUnitAI(unit, troopLayer);
        return;
    }

    if (liveTarget->type == 303 && shouldAbandonWallForBetterPath(unit, targetID)) {
        startUnitAI(unit, troopLayer);
        return;
    }

    auto config = BuildingConfig::getInstance()->getConfig(liveTarget->type);
    if (!config) {
        startUnitAI(unit, troopLayer);
        return;
    }

    Vec2 unitGridPos = GridMapUtils::pixelToGrid(unitPos);
    int unitGridX = static_cast<int>(std::floor(unitGridPos.x));
    int unitGridY = static_cast<int>(std::floor(unitGridPos.y));

    int bX = liveTarget->gridX;
    int bY = liveTarget->gridY;
    int bW = config->gridWidth;
    int bH = config->gridHeight;

    int gridDistX = 0;
    int gridDistY = 0;

    if (unitGridX < bX) {
        gridDistX = bX - unitGridX;
    } else if (unitGridX >= bX + bW) {
        gridDistX = unitGridX - (bX + bW) + 1;
    }

    if (unitGridY < bY) {
        gridDistY = bY - unitGridY;
    } else if (unitGridY >= bY + bH) {
        gridDistY = unitGridY - (bY + bH) + 1;
    }

    int gridDistance = std::max(gridDistX, gridDistY);
    int attackRangeGrid = getAttackRangeByUnitType(unit->getUnitTypeID());
    
    if (gridDistance > attackRangeGrid) {
        
        auto pathfinder = FindPathUtil::getInstance();
        std::vector<Vec2> pathToTarget = pathfinder->findPathToAttackBuilding(unitPos, *liveTarget, attackRangeGrid);
        
        if (!pathToTarget.empty()) {
            unit->followPath(pathToTarget, 100.0f, [this, unit, troopLayer, forcedTarget]() {
                startCombatLoopWithForcedTarget(unit, troopLayer, forcedTarget);
            });
        } else {
            Vec2 targetPos = GridMapUtils::gridToPixelCenter(liveTarget->gridX, liveTarget->gridY);
            std::vector<Vec2> directPath = { targetPos };
            unit->followPath(directPath, 100.0f, [this, unit, troopLayer, forcedTarget]() {
                startCombatLoopWithForcedTarget(unit, troopLayer, forcedTarget);
            });
        }
        return;
    }
    
    Vec2 targetPos = GridMapUtils::gridToPixelCenter(liveTarget->gridX, liveTarget->gridY);
    
    unit->attackTowardPosition(targetPos, [this, unit, troopLayer, targetID]() {
        executeAttack(unit, troopLayer, targetID, true,
            [this, unit, troopLayer]() {
                startUnitAI(unit, troopLayer);
            },
            [this, unit, troopLayer, targetID]() {
                auto dm = VillageDataManager::getInstance();
                auto t = dm->getBuildingById(targetID);
                if (t && !t->isDestroyed && t->currentHP > 0) {
                    startCombatLoopWithForcedTarget(unit, troopLayer, t);
                } else {
                    startUnitAI(unit, troopLayer);
                }
            }
        );
    });
}

// ==========================================
// 炸弹兵自爆攻击逻辑
// ==========================================
void BattleProcessController::performWallBreakerSuicideAttack(
    BattleUnitSprite* unit,
    BuildingInstance* target,
    BattleTroopLayer* troopLayer,
    const std::function<void()>& onComplete
) {
    if (!unit || !target || !troopLayer) {
        if (onComplete) onComplete();
        return;
    }

    CCLOG("BattleProcessController: Wall Breaker suicide attack on building %d", target->id);

    // 1. 获取炸弹兵伤害值
    int damage = getDamageByUnitType(unit->getUnitTypeID());

    // 2. 对目标建筑造成伤害
    target->currentHP -= damage;

    CCLOG("BattleProcessController: Target HP: %d (damage: %d)", target->currentHP, damage);

    // 3. 检查目标是否被摧毁
    if (target->currentHP <= 0) {
        target->isDestroyed = true;
        target->currentHP = 0;

        // 更新寻路地图
        FindPathUtil::getInstance()->updatePathfindingMap();

        // 发送建筑摧毁事件
        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(
            "EVENT_BUILDING_DESTROYED",
            static_cast<void*>(target)
        );

        CCLOG("BattleProcessController: Target destroyed!");
    }

    // 4. 播放爆炸特效
    auto explosion = ParticleExplosion::create();
    explosion->setPosition(unit->getPosition());
    explosion->setDuration(0.2f);
    explosion->setScale(0.3f);
    explosion->setAutoRemoveOnFinish(true);
    troopLayer->getParent()->addChild(explosion, 1000);

    // 5. 屏幕震动（可选）
    auto camera = Camera::getDefaultCamera();
    auto shake = Sequence::create(
        MoveBy::create(0.05f, Vec3(5, 0, 0)),
        MoveBy::create(0.05f, Vec3(-10, 0, 0)),
        MoveBy::create(0.05f, Vec3(5, 0, 0)),
        nullptr
    );
    camera->runAction(shake);

    // 6. 炸弹兵自杀
    unit->takeDamage(9999);

    // ✅ 恢复正常颜色（移除红色锁定状态）
    unit->setColor(Color3B::WHITE);

    // 7. 播放死亡动画并移除
    unit->playDeathAnimation([troopLayer, unit, onComplete]() {
        CCLOG("BattleProcessController: Wall Breaker death animation completed");

        // ✅ 在移除前生成墓碑
        Vec2 tombstonePos = unit->getPosition();
        UnitTypeID unitType = unit->getUnitTypeID();

        // 移除兵种
        troopLayer->removeUnit(unit);

        // 生成墓碑（会自动淡出）
        troopLayer->spawnTombstone(tombstonePos, unitType);

        if (onComplete) {
            onComplete();
        }
    });
}
