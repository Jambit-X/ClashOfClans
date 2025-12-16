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
        default:
            return TroopConfig::getInstance()->getTroopById(1001).damagePerSecond;
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
    CCLOG("BattleProcessController: Resetting battle state");

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

    CCLOG("BattleProcessController: Restored %d buildings", restoredCount);
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
        CCLOG("【DEBUG】executeAttack: Unit is already changing target, skipping");
        return;
    }

    // 目标已摧毁
    if (!liveTarget || liveTarget->isDestroyed || liveTarget->currentHP <= 0) {
        CCLOG("【DEBUG】executeAttack: Target ID=%d is gone/destroyed, calling onTargetDestroyed", targetID);
        
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
    
    CCLOG("【DEBUG】executeAttack: Unit deals %d damage to building ID=%d (HP: %d -> %d)",
          dps, targetID, liveTarget->currentHP, liveTarget->currentHP - dps);

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
        
        CCLOG("【DEBUG】🔴 Building ID=%d DESTROYED!", targetID);
        FindPathUtil::getInstance()->updatePathfindingMap();
        
        onTargetDestroyed();
    }
    else {
        // 继续攻击
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
    
    // 找到当前最佳目标
    const BuildingInstance* bestTarget = findBestTargetBuilding(unitPos);
    if (!bestTarget) return false;
    
    Vec2 targetCenter = GridMapUtils::gridToPixelCenter(bestTarget->gridX, bestTarget->gridY);
    
    // 尝试寻路到目标
    auto pathfinder = FindPathUtil::getInstance();
    std::vector<Vec2> pathAround = pathfinder->findPathToAttackBuilding(unitPos, *bestTarget);
    
    if (pathAround.empty()) {
        return false;  // 还是没有路径，继续砍墙
    }
    
    float pathLength = calculatePathLength(pathAround);
    float directDist = unitPos.distance(targetCenter);
    float detourCost = pathLength - directDist;
    
    // 如果绕路代价可接受，说明有更好的路径了
    if (detourCost <= PIXEL_DETOUR_THRESHOLD && pathLength <= directDist * 2.0f) {
        CCLOG("【DEBUG】shouldAbandonWallForBetterPath: Found better path! detour=%.0f, abandoning wall ID=%d",
              detourCost, currentWallID);
        return true;
    }
    
    return false;
}



// ==========================================
// 核心 AI 逻辑入口
// ==========================================

void BattleProcessController::startUnitAI(BattleUnitSprite* unit, BattleTroopLayer* troopLayer) {
    if (!unit) {
        CCLOG("【DEBUG】startUnitAI: Unit is null!");
        return;
    }

    CCLOG("【DEBUG】🔵 startUnitAI called for unit at (%.0f, %.0f)",
          unit->getPosition().x, unit->getPosition().y);

    const BuildingInstance* target = findBestTargetBuilding(unit->getPosition());

    if (!target) {
        CCLOG("【DEBUG】startUnitAI: No valid target found, playing idle");
        unit->playIdleAnimation();
        return;
    }

    CCLOG("【DEBUG】startUnitAI: Found target building ID=%d (HP=%d) at grid(%d, %d)",
          target->id, target->currentHP, target->gridX, target->gridY);

    auto pathfinder = FindPathUtil::getInstance();
    Vec2 unitPos = unit->getPosition();
    Vec2 targetCenter = GridMapUtils::gridToPixelCenter(target->gridX, target->gridY);

    std::vector<Vec2> pathAround = pathfinder->findPathToAttackBuilding(unitPos, *target);
    float distAround = calculatePathLength(pathAround);
    float distDirect = unitPos.distance(targetCenter);

    CCLOG("【DEBUG】startUnitAI: Direct dist=%.0f, Path dist=%.0f", distDirect, distAround);

    if (!pathAround.empty()) {
        float detourCost = distAround - distDirect;
        
        if (detourCost <= PIXEL_DETOUR_THRESHOLD && distAround <= distDirect * 2.0f) {
            CCLOG("【DEBUG】startUnitAI: Path acceptable, following path");
            unit->followPath(pathAround, 100.0f, [this, unit, troopLayer]() {
                startCombatLoop(unit, troopLayer);
            });
            return;
        }
        CCLOG("【DEBUG】startUnitAI: Detour too costly (%.0f), checking for walls", detourCost);
    }

    // 需要破墙
    const BuildingInstance* wallToBreak = getFirstWallInLine(unitPos, targetCenter);

    if (wallToBreak) {
        CCLOG("【DEBUG】startUnitAI: Found wall ID=%d blocking path", wallToBreak->id);
        std::vector<Vec2> pathToWall = pathfinder->findPathToAttackBuilding(unitPos, *wallToBreak);

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
        CCLOG("【DEBUG】startUnitAI: No wall blocking, moving directly");
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

const BuildingInstance* BattleProcessController::findBestTargetBuilding(const Vec2& unitWorldPos) {
    auto dataManager = VillageDataManager::getInstance();
    const auto& buildings = dataManager->getAllBuildings();

    if (buildings.empty()) return nullptr;

    const BuildingInstance* bestBuilding = nullptr;
    float minDistanceSq = FLT_MAX;

    for (const auto& building : buildings) {
        if (building.isDestroyed || building.currentHP <= 0) continue;
        if (building.state == BuildingInstance::State::PLACING) continue;
        if (building.type == 303) continue;  // 跳过城墙

        auto config = BuildingConfig::getInstance()->getConfig(building.type);
        if (!config) continue;

        Vec2 bPos = GridMapUtils::gridToPixelCenter(building.gridX, building.gridY);
        float distSq = unitWorldPos.distanceSquared(bPos);

        if (distSq < minDistanceSq) {
            minDistanceSq = distSq;
            bestBuilding = &building;
        }
    }

    return bestBuilding;
}

const BuildingInstance* BattleProcessController::findNearestBuilding(const Vec2& unitWorldPos) {
    return findBestTargetBuilding(unitWorldPos);
}

// ==========================================
// 战斗循环逻辑
// ==========================================

void BattleProcessController::startCombatLoop(BattleUnitSprite* unit, BattleTroopLayer* troopLayer) {
    if (!unit || !troopLayer) return;

    auto dm = VillageDataManager::getInstance();
    const BuildingInstance* target = findNearestBuilding(unit->getPosition());

    if (!target) {
        CCLOG("【DEBUG】startCombatLoop: No target found, playing idle");
        unit->playIdleAnimation();
        return;
    }

    BuildingInstance* mutableTarget = dm->getBuildingById(target->id);
    if (!mutableTarget || mutableTarget->isDestroyed || mutableTarget->currentHP <= 0) {
        CCLOG("【DEBUG】startCombatLoop: Target destroyed, restarting AI");
        startUnitAI(unit, troopLayer);
        return;
    }

    Vec2 buildingPos = GridMapUtils::gridToPixelCenter(mutableTarget->gridX, mutableTarget->gridY);
    Vec2 unitPos = unit->getPosition();
    float distance = unitPos.distance(buildingPos);
    
    // 检查攻击范围
    if (distance > ATTACK_RANGE) {
        CCLOG("【DEBUG】startCombatLoop: Too far (dist=%.0f), need to move closer", distance);
        startUnitAI(unit, troopLayer);
        return;
    }
    
    CCLOG("【DEBUG】startCombatLoop: Attacking building ID=%d (HP=%d), dist=%.0f",
          mutableTarget->id, mutableTarget->currentHP, distance);

    int targetID = mutableTarget->id;
    
    unit->attackTowardPosition(buildingPos, [this, unit, troopLayer, targetID]() {
        executeAttack(unit, troopLayer, targetID, false,
            [this, unit, troopLayer]() { startUnitAI(unit, troopLayer); },
            [this, unit, troopLayer]() { startCombatLoop(unit, troopLayer); }
        );
    });
}

void BattleProcessController::startCombatLoopWithForcedTarget(BattleUnitSprite* unit, BattleTroopLayer* troopLayer, const BuildingInstance* forcedTarget) {
    if (!unit || !troopLayer || !forcedTarget) return;

    auto dm = VillageDataManager::getInstance();
    int targetID = forcedTarget->id;

    BuildingInstance* liveTarget = dm->getBuildingById(targetID);
    if (!liveTarget || liveTarget->isDestroyed || liveTarget->currentHP <= 0) {
        CCLOG("【DEBUG】startCombatLoopWithForcedTarget: Target destroyed, restarting AI");
        startUnitAI(unit, troopLayer);
        return;
    }

    // 每次攻击前检查是否有更好的路径
    if (shouldAbandonWallForBetterPath(unit, targetID)) {
        CCLOG("【DEBUG】startCombatLoopWithForcedTarget: Better path found! Abandoning wall ID=%d", targetID);
        startUnitAI(unit, troopLayer);
        return;
    }

    Vec2 targetPos = GridMapUtils::gridToPixelCenter(liveTarget->gridX, liveTarget->gridY);
    Vec2 unitPos = unit->getPosition();
    float distance = unitPos.distance(targetPos);
    
    // 检查攻击范围
    if (distance > ATTACK_RANGE) {
        CCLOG("【DEBUG】startCombatLoopWithForcedTarget: Too far (dist=%.0f), moving closer", distance);
        
        auto pathfinder = FindPathUtil::getInstance();
        std::vector<Vec2> pathToTarget = pathfinder->findPathToAttackBuilding(unitPos, *liveTarget);
        
        if (!pathToTarget.empty()) {
            unit->followPath(pathToTarget, 100.0f, [this, unit, troopLayer, forcedTarget]() {
                startCombatLoopWithForcedTarget(unit, troopLayer, forcedTarget);
            });
        } else {
            std::vector<Vec2> directPath = { targetPos };
            unit->followPath(directPath, 100.0f, [this, unit, troopLayer, forcedTarget]() {
                startCombatLoopWithForcedTarget(unit, troopLayer, forcedTarget);
            });
        }
        return;
    }
    
    CCLOG("【DEBUG】startCombatLoopWithForcedTarget: Attacking wall ID=%d (HP=%d), dist=%.0f",
          targetID, liveTarget->currentHP, distance);

    unit->attackTowardPosition(targetPos, [this, unit, troopLayer, targetID]() {
        executeAttack(unit, troopLayer, targetID, true,
            [this, unit, troopLayer]() {
                CCLOG("【DEBUG】Wall destroyed, restarting AI");
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
