// BattleProcessController.cpp
// 战斗流程控制器实现，管理单位AI、寻路、攻击和战斗循环逻辑

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
#include "DestructionTracker.h"
#include "TrapSystem.h"
#include "TargetFinder.h"

USING_NS_CC;

BattleProcessController* BattleProcessController::_instance = nullptr;

// 计算路径总长度
static float calculatePathLength(const std::vector<Vec2>& path) {
    if (path.size() < 2) return 0.0f;
    float totalDist = 0.0f;
    for (size_t i = 0; i < path.size() - 1; ++i) {
        totalDist += path[i].distance(path[i + 1]);
    }
    return totalDist;
}

// 根据兵种类型获取伤害值
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

// 根据兵种类型获取攻击范围
static int getAttackRangeByUnitType(UnitTypeID typeID) {
    switch (typeID) {
        case UnitTypeID::ARCHER:
            return 3;
        case UnitTypeID::BARBARIAN:
        case UnitTypeID::GOBLIN:
        case UnitTypeID::GIANT:
        case UnitTypeID::BALLOON:
            return 1;
        case UnitTypeID::WALL_BREAKER:
            return 0;
        default:
            return 1;
    }
}

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

        // 重置攻击冷却
        building.attackCooldown = 0.0f;
    }

    // 清理陷阱触发状态
    TrapSystem::getInstance()->reset();

    dataManager->saveToFile("village.json");
}

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

    // 炸弹兵自爆特判
    if (unit->getUnitTypeID() == UnitTypeID::WALL_BREAKER) {
        CCLOG("BattleProcessController: Wall Breaker executing suicide attack");

        performWallBreakerSuicideAttack(unit, liveTarget, troopLayer, [onTargetDestroyed]() {
            if (onTargetDestroyed) {
                onTargetDestroyed();
            }
        });

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
        
        // 发送建筑摧毁事件
        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(
            "EVENT_BUILDING_DESTROYED", 
            static_cast<void*>(liveTarget)
        );

        // 更新摧毁进度
        DestructionTracker::getInstance()->updateProgress();

        onTargetDestroyed();
    }
    else {
        onContinueAttack();
    }
}

bool BattleProcessController::shouldAbandonWallForBetterPath(BattleUnitSprite* unit, int currentWallID) {
    Vec2 unitPos = unit->getPosition();
    Vec2 unitGridPos = GridMapUtils::pixelToGrid(unitPos);
    
    CCLOG("--- shouldAbandonWallForBetterPath DEBUG ---");
    CCLOG("  Unit at pixel(%.1f, %.1f), grid(%.1f, %.1f)", 
          unitPos.x, unitPos.y, unitGridPos.x, unitGridPos.y);
    CCLOG("  Current wall ID: %d", currentWallID);
    
    const BuildingInstance* bestTarget = nullptr;
    // 使用TargetFinder查找最佳目标
    bestTarget = TargetFinder::getInstance()->findTarget(unitPos, unit->getUnitTypeID());
    
    if (!bestTarget) {
        CCLOG("  No best target found, keep attacking wall");
        return false;
    }
    
    CCLOG("  Best target: ID=%d, Type=%d at grid(%d, %d)",
          bestTarget->id, bestTarget->type, bestTarget->gridX, bestTarget->gridY);
    
    Vec2 targetCenter = GridMapUtils::gridToPixelCenter(bestTarget->gridX, bestTarget->gridY);
    
    auto pathfinder = FindPathUtil::getInstance();
    int attackRange = getAttackRangeByUnitType(unit->getUnitTypeID());
    std::vector<Vec2> pathAround = pathfinder->findPathToAttackBuilding(unitPos, *bestTarget, attackRange);
    
    if (pathAround.empty()) {
        CCLOG("  No path around found, keep attacking wall");
        return false;
    }
    
    float pathLength = calculatePathLength(pathAround);
    float directDist = unitPos.distance(targetCenter);
    float detourCost = pathLength - directDist;
    
    CCLOG("  Path analysis:");
    CCLOG("    Path length: %.1f", pathLength);
    CCLOG("    Direct distance: %.1f", directDist);
    CCLOG("    Detour cost: %.1f (threshold: %.1f)", detourCost, PIXEL_DETOUR_THRESHOLD);
    CCLOG("    Path/Direct ratio: %.2f (max allowed: 2.0)", pathLength / directDist);
    
    if (detourCost <= PIXEL_DETOUR_THRESHOLD && pathLength <= directDist * 2.0f) {
        CCLOG("  ✓ ABANDON WALL - better path found!");
        return true;
    }
    
    CCLOG("  ✗ Keep attacking wall - no better path");
    CCLOG("--- END shouldAbandonWallForBetterPath ---");
    return false;
}

void BattleProcessController::startUnitAI(BattleUnitSprite* unit, BattleTroopLayer* troopLayer) {
    if (!unit) {
        return;
    }

    Vec2 unitPos = unit->getPosition();
    Vec2 unitGridPos = GridMapUtils::pixelToGrid(unitPos);
    
    CCLOG("========== START UNIT AI DEBUG ==========");
    CCLOG("Unit: %s at pixel(%.1f, %.1f), grid(%.1f, %.1f)",
          unit->getUnitType().c_str(), unitPos.x, unitPos.y, unitGridPos.x, unitGridPos.y);
    
    const BuildingInstance* target = nullptr;
    
    // 炸弹兵特殊处理：只攻击城墙
    auto targetFinder = TargetFinder::getInstance();
    
    if (unit->getUnitTypeID() == UnitTypeID::WALL_BREAKER) {
        target = targetFinder->findNearestWall(unitPos);
        if (!target) {
            CCLOG("Wall Breaker: No walls found, standing idle");
            unit->playIdleAnimation();
            return;  // 没有城墙则原地待机
        }
    }
    else {
        // 使用TargetFinder的通用入口
        target = targetFinder->findTarget(unitPos, unit->getUnitTypeID());
    }

    if (!target) {
        CCLOG("No target found, playing idle animation");
        unit->playIdleAnimation();
        return;
    }

    CCLOG("Target selected: ID=%d, Type=%d at grid(%d, %d)",
          target->id, target->type, target->gridX, target->gridY);

    // 发送目标锁定事件
    EventCustom event("EVENT_UNIT_TARGET_LOCKED");
    event.setUserData(reinterpret_cast<void*>(static_cast<intptr_t>(target->id)));
    Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);

    auto pathfinder = FindPathUtil::getInstance();
    Vec2 targetCenter = GridMapUtils::gridToPixelCenter(target->gridX, target->gridY);

    int attackRange = getAttackRangeByUnitType(unit->getUnitTypeID());
    CCLOG("Attack range: %d grids", attackRange);
    
    // 气球兵飞行单位特殊处理
    if (unit->getUnitTypeID() == UnitTypeID::BALLOON) {
        // 计算建筑边缘攻击位置
        auto config = BuildingConfig::getInstance()->getConfig(target->type);
        int buildingWidth = config ? config->gridWidth : 2;
        int buildingHeight = config ? config->gridHeight : 2;
        
        // 计算建筑中心
        float buildingCenterX = target->gridX + buildingWidth / 2.0f;
        float buildingCenterY = target->gridY + buildingHeight / 2.0f;
        Vec2 buildingCenter = GridMapUtils::gridToPixelCenter(
            static_cast<int>(buildingCenterX), 
            static_cast<int>(buildingCenterY)
        );
        
        // 计算从气球到建筑中心的方向
        Vec2 direction = buildingCenter - unitPos;
        direction.normalize();
        
        // 计算建筑边缘攻击点
        float attackDistancePixels = (attackRange + buildingWidth / 2.0f) * 32.0f;
        Vec2 attackPosition = buildingCenter - direction * attackDistancePixels;
        
        // 如果攻击位置更远，直接飞到建筑中心
        if (unitPos.distance(attackPosition) > unitPos.distance(buildingCenter)) {
            attackPosition = buildingCenter;
        }
        
        std::vector<Vec2> directPath = { attackPosition };
        unit->followPath(directPath, 100.0f, [this, unit, troopLayer]() {
            startCombatLoop(unit, troopLayer);
        });
        return;
    }
    
    std::vector<Vec2> pathAround = pathfinder->findPathToAttackBuilding(unitPos, *target, attackRange);
    float distAround = calculatePathLength(pathAround);
    float distDirect = unitPos.distance(targetCenter);

    CCLOG("Path finding result: pathAround.size()=%zu, distAround=%.1f, distDirect=%.1f",
          pathAround.size(), distAround, distDirect);

    if (!pathAround.empty()) {
        float detourCost = distAround - distDirect;
        
        CCLOG("Detour cost: %.1f (threshold=%.1f)", detourCost, PIXEL_DETOUR_THRESHOLD);
        
        if (detourCost <= PIXEL_DETOUR_THRESHOLD && distAround <= distDirect * 2.0f) {
            CCLOG("✓ Using path around (detour acceptable)");
            unit->followPath(pathAround, 100.0f, [this, unit, troopLayer]() {
                startCombatLoop(unit, troopLayer);
            });
            return;
        } else {
            CCLOG("✗ Path around too long, will check for wall to break");
        }
    } else {
        CCLOG("No valid path around found, will check for wall to break");
    }

    const BuildingInstance* wallToBreak = getFirstWallInLine(unitPos, targetCenter);

    if (wallToBreak && unit->getUnitTypeID() != UnitTypeID::BALLOON) {
        CCLOG("Wall to break found: ID=%d at grid(%d, %d)", 
              wallToBreak->id, wallToBreak->gridX, wallToBreak->gridY);
        
        std::vector<Vec2> pathToWall = pathfinder->findPathToAttackBuilding(unitPos, *wallToBreak, attackRange);
        CCLOG("Path to wall: size=%zu", pathToWall.size());

        if (pathToWall.empty()) {
            CCLOG("No path to wall, starting forced combat directly");
            startCombatLoopWithForcedTarget(unit, troopLayer, wallToBreak);
        }
        else {
            CCLOG("Following path to wall");
            unit->followPath(pathToWall, 100.0f, [this, unit, troopLayer, wallToBreak]() {
                startCombatLoopWithForcedTarget(unit, troopLayer, wallToBreak);
            });
        }
    }
    else {
        CCLOG("⚠️ NO WALL FOUND in line! Going direct to target (may pass through walls!)");
        CCLOG("  Unit pos: (%.1f, %.1f)", unitPos.x, unitPos.y);
        CCLOG("  Target center: (%.1f, %.1f)", targetCenter.x, targetCenter.y);
        
        std::vector<Vec2> directPath = { targetCenter };
        unit->followPath(directPath, 100.0f, [this, unit, troopLayer]() {
            startCombatLoop(unit, troopLayer);
        });
    }
    
    CCLOG("========== END UNIT AI DEBUG ==========\n");
}

const BuildingInstance* BattleProcessController::getFirstWallInLine(const Vec2& startPixel, const Vec2& endPixel) {
    auto dataManager = VillageDataManager::getInstance();
    auto pathfinder = FindPathUtil::getInstance();

    CCLOG("--- getFirstWallInLine DEBUG ---");
    CCLOG("  Start pixel: (%.1f, %.1f)", startPixel.x, startPixel.y);
    CCLOG("  End pixel: (%.1f, %.1f)", endPixel.x, endPixel.y);
    
    Vec2 startGridF = GridMapUtils::pixelToGrid(startPixel);
    Vec2 endGridF = GridMapUtils::pixelToGrid(endPixel);
    CCLOG("  Start grid: (%.1f, %.1f)", startGridF.x, startGridF.y);
    CCLOG("  End grid: (%.1f, %.1f)", endGridF.x, endGridF.y);

    // 方法1：使用寻路器找穿墙路径
    std::vector<Vec2> throughPath = pathfinder->findPathIgnoringWalls(startPixel, endPixel);
    CCLOG("  findPathIgnoringWalls returned %zu points", throughPath.size());
    
    if (!throughPath.empty()) {
        for (size_t i = 0; i < throughPath.size(); ++i) {
            const auto& worldPt = throughPath[i];
            Vec2 gridF = GridMapUtils::pixelToGrid(worldPt);
            int gx = static_cast<int>(std::floor(gridF.x + 0.5f));
            int gy = static_cast<int>(std::floor(gridF.y + 0.5f));

            BuildingInstance* b = dataManager->getBuildingAtGrid(gx, gy);
            if (b) {
                CCLOG("    Path point %zu: grid(%d, %d) has building ID=%d, type=%d, destroyed=%s",
                      i, gx, gy, b->id, b->type, b->isDestroyed ? "true" : "false");
                      
                if (b->type == 303 && !b->isDestroyed && b->currentHP > 0) {
                    CCLOG("  ✓ Found wall at grid(%d, %d)!", gx, gy);
                    return b;
                }
            }
        }
    }

    CCLOG("  No wall found via pathfinder, trying enhanced line scan...");
    
    // 方法2：线性扫描，使用更多采样点
    Vec2 startGrid = GridMapUtils::pixelToGrid(startPixel);
    Vec2 endGrid = GridMapUtils::pixelToGrid(endPixel);
    Vec2 diff = endGrid - startGrid;
    
    // 使用更细步长确保检测到每个格子
    float maxDiff = std::max(std::abs(diff.x), std::abs(diff.y));
    int steps = static_cast<int>(std::ceil(maxDiff)) * 2;
    if (steps < 1) steps = 1;
    
    CCLOG("  Enhanced line scan: %d steps from grid(%.1f, %.1f) to grid(%.1f, %.1f)",
          steps, startGrid.x, startGrid.y, endGrid.x, endGrid.y);
    
    Vec2 direction = diff / static_cast<float>(steps);
    Vec2 current = startGrid;
    
    // 避免重复检测同一格子
    std::set<std::pair<int, int>> checkedGrids;
    
    for (int i = 0; i <= steps; ++i) {
        int gx = static_cast<int>(std::floor(current.x));
        int gy = static_cast<int>(std::floor(current.y));
        
        auto gridKey = std::make_pair(gx, gy);
        if (checkedGrids.find(gridKey) == checkedGrids.end()) {
            checkedGrids.insert(gridKey);

            BuildingInstance* b = dataManager->getBuildingAtGrid(gx, gy);
            if (b) {
                CCLOG("    Step %d: grid(%d, %d) has building ID=%d, type=%d",
                      i, gx, gy, b->id, b->type);
                      
                if (b->type == 303 && !b->isDestroyed && b->currentHP > 0) {
                    CCLOG("  ✓ Found wall at grid(%d, %d) via line scan!", gx, gy);
                    return b;
                }
            }
        }
        current += direction;
    }
    
    // 方法3：遍历所有城墙检查路径交叉
    CCLOG("  Line scan failed, checking all walls for intersection...");
    const auto& buildings = dataManager->getAllBuildings();
    
    for (const auto& building : buildings) {
        if (building.type != 303) continue;
        if (building.isDestroyed || building.currentHP <= 0) continue;
        
        int wallX = building.gridX;
        int wallY = building.gridY;
        
        // 计算城墙是否在路径包围盒内
        int minX = static_cast<int>(std::min(startGrid.x, endGrid.x)) - 1;
        int maxX = static_cast<int>(std::max(startGrid.x, endGrid.x)) + 1;
        int minY = static_cast<int>(std::min(startGrid.y, endGrid.y)) - 1;
        int maxY = static_cast<int>(std::max(startGrid.y, endGrid.y)) + 1;
        
        if (wallX >= minX && wallX <= maxX && wallY >= minY && wallY <= maxY) {
            // 使用点到线段距离判断城墙是否在路径上
            Vec2 wallPos(wallX + 0.5f, wallY + 0.5f);
            Vec2 lineDir = endGrid - startGrid;
            float lineLen = lineDir.length();
            
            if (lineLen > 0.01f) {
                lineDir.normalize();
                Vec2 toWall = wallPos - startGrid;
                float proj = toWall.dot(lineDir);
                
                // 检查投影点是否在线段范围内
                if (proj >= 0 && proj <= lineLen) {
                    Vec2 projPoint = startGrid + lineDir * proj;
                    float dist = wallPos.distance(projPoint);
                    
                    // 距离小于1.5格认为城墙在路径上
                    if (dist < 1.5f) {
                        CCLOG("  ✓ Found wall ID=%d at grid(%d, %d) via intersection check! (dist=%.2f)",
                              building.id, wallX, wallY, dist);
                        return &building;
                    }
                }
            }
        }
    }

    CCLOG("  ✗ NO WALL FOUND in line between unit and target!");
    CCLOG("--- END getFirstWallInLine ---");
    return nullptr;
}

void BattleProcessController::startCombatLoop(BattleUnitSprite* unit, BattleTroopLayer* troopLayer) {
    if (!unit || !troopLayer) return;

    Vec2 unitPos = unit->getPosition();
    auto dm = VillageDataManager::getInstance();
    const BuildingInstance* target = TargetFinder::getInstance()->findTarget(unitPos, unit->getUnitTypeID());

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

    CCLOG("--- startCombatLoop DEBUG ---");
    CCLOG("  Unit %s at grid(%d, %d)", unit->getUnitType().c_str(), unitGridX, unitGridY);
    CCLOG("  Target ID=%d Type=%d at grid(%d, %d), size(%d x %d)",
          mutableTarget->id, mutableTarget->type, bX, bY, bW, bH);

    // 计算到建筑的网格距离
    int gridDistX = 0;
    int gridDistY = 0;

    if (unitGridX < bX) {
        gridDistX = bX - unitGridX;
    } else if (unitGridX >= bX + bW) {
        gridDistX = unitGridX - (bX + bW - 1);
    }

    if (unitGridY < bY) {
        gridDistY = bY - unitGridY;
    } else if (unitGridY >= bY + bH) {
        gridDistY = unitGridY - (bY + bH - 1);
    }

    int gridDistance = std::max(gridDistX, gridDistY);
    int attackRangeGrid = getAttackRangeByUnitType(unit->getUnitTypeID());

    CCLOG("  Distance: X=%d, Y=%d, Max=%d, AttackRange=%d",
          gridDistX, gridDistY, gridDistance, attackRangeGrid);

    // 气球兵使用像素距离判定
    if (unit->getUnitTypeID() == UnitTypeID::BALLOON) {
        Vec2 buildingCenter = GridMapUtils::gridToPixelCenter(
            bX + bW / 2,
            bY + bH / 2
        );
        float pixelDistance = unitPos.distance(buildingCenter);
        float maxAttackDistance = (std::max(bW, bH) + 1) * 32.0f;

        if (pixelDistance > maxAttackDistance) {
            CCLOG("  Balloon too far (%.1f > %.1f), restarting AI", pixelDistance, maxAttackDistance);
            startUnitAI(unit, troopLayer);
            return;
        }
    } else if (gridDistance > attackRangeGrid) {
        CCLOG("  Out of range (%d > %d), restarting AI", gridDistance, attackRangeGrid);
        startUnitAI(unit, troopLayer);
        return;
    }

    CCLOG("  ✓ In range, attacking target!");
    CCLOG("--- END startCombatLoop ---");

    // 执行攻击
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

    // 持续检查：如果正在攻击城墙，检查是否有更好的路径
    if (liveTarget->type == 303 && shouldAbandonWallForBetterPath(unit, targetID)) {
        CCLOG("BattleProcessController: Found better path! Abandoning wall attack.");
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
                    // 每次攻击后都检查是否有更好的路径
                    if (t->type == 303 && shouldAbandonWallForBetterPath(unit, targetID)) {
                        CCLOG("BattleProcessController: Better path found after attack! Switching target.");
                        startUnitAI(unit, troopLayer);
                    } else {
                        startCombatLoopWithForcedTarget(unit, troopLayer, t);
                    }
                } else {
                    startUnitAI(unit, troopLayer);
                }
            }
        );
    });
}

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

    // 获取炸弹兵伤害值
    int damage = getDamageByUnitType(unit->getUnitTypeID());
    
    // 对城墙造成10倍伤害
    if (target->type == 303) {
        damage *= 10;
        CCLOG("BattleProcessController: Wall Breaker deals 10x damage to wall!");
    }

    // 对目标建筑造成伤害
    target->currentHP -= damage;
    CCLOG("BattleProcessController: Target HP: %d (damage: %d)", target->currentHP, damage);

    // 检查目标是否被摧毁
    if (target->currentHP <= 0) {
        target->isDestroyed = true;
        target->currentHP = 0;

        FindPathUtil::getInstance()->updatePathfindingMap();

        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(
            "EVENT_BUILDING_DESTROYED",
            static_cast<void*>(target)
        );

        DestructionTracker::getInstance()->updateProgress();
        CCLOG("BattleProcessController: Target destroyed!");
    }

    // 播放爆炸特效
    auto explosion = ParticleExplosion::create();
    explosion->setPosition(unit->getPosition());
    explosion->setDuration(0.2f);
    explosion->setScale(0.3f);
    explosion->setAutoRemoveOnFinish(true);
    troopLayer->getParent()->addChild(explosion, 1000);

    // 屏幕震动
    auto camera = Camera::getDefaultCamera();
    auto shake = Sequence::create(
        MoveBy::create(0.05f, Vec3(5, 0, 0)),
        MoveBy::create(0.05f, Vec3(-10, 0, 0)),
        MoveBy::create(0.05f, Vec3(5, 0, 0)),
        nullptr
    );
    camera->runAction(shake);

    // 炸弹兵自杀
    unit->takeDamage(9999);
    unit->setColor(Color3B::WHITE);

    // 播放死亡动画并移除
    unit->playDeathAnimation([troopLayer, unit, onComplete]() {
        CCLOG("BattleProcessController: Wall Breaker death animation completed");

        Vec2 tombstonePos = unit->getPosition();
        UnitTypeID unitType = unit->getUnitTypeID();

        troopLayer->removeUnit(unit);
        troopLayer->spawnTombstone(tombstonePos, unitType);

        if (onComplete) {
            onComplete();
        }
    });
}
