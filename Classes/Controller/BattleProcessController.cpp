#include "BattleProcessController.h"
#include "../Layer/BattleTroopLayer.h"
#include "../Manager/VillageDataManager.h"
#include "../Model/BuildingConfig.h"
#include "../Util/GridMapUtils.h"
#include "../Util/FindPathUtil.h"
#include <cmath>

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
        
        onTargetDestroyed();
    }
    else {
        unit->runAction(Sequence::create(
            CallFunc::create([onContinueAttack]() {
                onContinueAttack();
            }),
            nullptr
        ));
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
        if (building.type == 303) continue;

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
