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
// 战斗状态重置（退出战斗时调用）
// ==========================================

void BattleProcessController::resetBattleState() {
    CCLOG("BattleProcessController: Resetting battle state - restoring all building HP");

    auto dataManager = VillageDataManager::getInstance();
    auto& buildings = const_cast<std::vector<BuildingInstance>&>(dataManager->getAllBuildings());

    int restoredCount = 0;

    for (auto& building : buildings) {
        auto config = BuildingConfig::getInstance()->getConfig(building.type);
        if (config) {
            int maxHP = config->hitPoints > 0 ? config->hitPoints : 100;

            if (building.currentHP != maxHP || building.isDestroyed) {
                CCLOG("BattleProcessController: Restoring building %d (type=%d) HP: %d -> %d",
                    building.id, building.type, building.currentHP, maxHP);
                building.currentHP = maxHP;
                building.isDestroyed = false;
                restoredCount++;
            }
        }
        else {
            if (building.currentHP != 100 || building.isDestroyed) {
                building.currentHP = 100;
                building.isDestroyed = false;
                restoredCount++;
            }
        }
    }

    CCLOG("BattleProcessController: Battle state reset complete - restored %d buildings", restoredCount);
    dataManager->saveToFile("village.json");
}

// ==========================================
// 核心 AI 逻辑入口 (startUnitAI)
// ==========================================

void BattleProcessController::startUnitAI(BattleUnitSprite* unit, BattleTroopLayer* troopLayer) {
    if (!unit) {
        return;
    }

    const BuildingInstance* target = findBestTargetBuilding(unit->getPosition());

    if (!target) {
        unit->playIdleAnimation();
        return;
    }

    auto pathfinder = FindPathUtil::getInstance();
    Vec2 unitPos = unit->getPosition();
    Vec2 targetCenter = GridMapUtils::gridToPixelCenter(target->gridX, target->gridY);

    std::vector<Vec2> pathAround = pathfinder->findPathToAttackBuilding(unitPos, *target);
    float distAround = calculatePathLength(pathAround);
    float distDirect = unitPos.distance(targetCenter);

    if (!pathAround.empty()) {
        float detourCost = distAround - distDirect;
        const float PIXEL_DETOUR_THRESHOLD = 800.0f;
        
        if (detourCost > PIXEL_DETOUR_THRESHOLD || distAround > distDirect * 2.0f) {
            // 绕路代价太大，继续到破墙逻辑
        }
        else {
            // 有路径且绕路代价可接受，直接走
            unit->followPath(pathAround, 100.0f, [this, unit, troopLayer]() {
                startCombatLoop(unit, troopLayer);
            });
            return;
        }
    }

    // 需要破墙的情况
    const BuildingInstance* wallToBreak = getFirstWallInLine(unitPos, targetCenter);

    if (wallToBreak) {
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
        // 没有墙挡住，直接走向目标
        std::vector<Vec2> directPath = { targetCenter };
        unit->followPath(directPath, 100.0f, [this, unit, troopLayer]() {
            startCombatLoop(unit, troopLayer);
        });
    }
}

// ... (其他辅助函数保持不变)
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

    if (buildings.empty()) {
        return nullptr;
    }

    const BuildingInstance* bestBuilding = nullptr;
    float minDistanceSq = FLT_MAX;

    for (const auto& building : buildings) {
        if (building.isDestroyed) continue;
        if (building.currentHP <= 0) continue;
        if (building.state == BuildingInstance::State::PLACING) continue;
        if (building.type == 303) continue;

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
        unit->playIdleAnimation();
        return;
    }

    BuildingInstance* mutableTarget = dm->getBuildingById(target->id);
    if (!mutableTarget) {
        startUnitAI(unit, troopLayer);
        return;
    }

    if (mutableTarget->isDestroyed || mutableTarget->currentHP <= 0) {
        startUnitAI(unit, troopLayer);
        return;
    }

    Vec2 buildingPos = GridMapUtils::gridToPixelCenter(mutableTarget->gridX, mutableTarget->gridY);

    unit->attackTowardPosition(buildingPos, [this, unit, troopLayer, mutableTarget]() {
        auto dmInner = VillageDataManager::getInstance();
        BuildingInstance* liveTarget = dmInner->getBuildingById(mutableTarget->id);

        if (!liveTarget || liveTarget->isDestroyed || liveTarget->currentHP <= 0) {
            startUnitAI(unit, troopLayer);
            return;
        }

        TroopInfo troopInfo = TroopConfig::getInstance()->getTroopById(1001);

        std::string ut = unit->getUnitType();
        if (ut.find("Barbarian") != std::string::npos || ut.find("barbarian") != std::string::npos || ut.find("野蛮人") != std::string::npos) {
            troopInfo = TroopConfig::getInstance()->getTroopById(1001);
        }
        else if (ut.find("Archer") != std::string::npos || ut.find("archer") != std::string::npos || ut.find("弓箭手") != std::string::npos) {
            troopInfo = TroopConfig::getInstance()->getTroopById(1002);
        }
        else if (ut.find("Goblin") != std::string::npos || ut.find("goblin") != std::string::npos || ut.find("哥布林") != std::string::npos) {
            troopInfo = TroopConfig::getInstance()->getTroopById(1003);
        }
        else if (ut.find("Giant") != std::string::npos || ut.find("giant") != std::string::npos || ut.find("巨人") != std::string::npos) {
            troopInfo = TroopConfig::getInstance()->getTroopById(1004);
        }
        else if (ut.find("Wall_Breaker") != std::string::npos || ut.find("炸弹人") != std::string::npos) {
            troopInfo = TroopConfig::getInstance()->getTroopById(1005);
        }

        int dps = troopInfo.damagePerSecond;

        if (liveTarget->isDestroyed || liveTarget->currentHP <= 0) {
            startUnitAI(unit, troopLayer);
            return;
        }

        liveTarget->currentHP -= dps;

        if (liveTarget->currentHP <= 0) {
            liveTarget->isDestroyed = true;
            liveTarget->currentHP = 0;
            FindPathUtil::getInstance()->updatePathfindingMap();
            startUnitAI(unit, troopLayer);
            return;
        }
        else {
            unit->runAction(Sequence::create(
                CallFunc::create([this, unit, troopLayer]() {
                    startCombatLoop(unit, troopLayer);
                }),
                nullptr
            ));
        }
    });
}

void BattleProcessController::startCombatLoopWithForcedTarget(BattleUnitSprite* unit, BattleTroopLayer* troopLayer, const BuildingInstance* forcedTarget) {
    if (!unit || !troopLayer || !forcedTarget) {
        return;
    }

    auto dataManager = VillageDataManager::getInstance();
    int targetID = forcedTarget->id;

    auto maybe = dataManager->getBuildingById(targetID);
    if (!maybe || maybe->isDestroyed || maybe->currentHP <= 0) {
        unit->playIdleAnimation();
        startUnitAI(unit, troopLayer);
        return;
    }

    const BuildingInstance* safeTarget = dataManager->getBuildingById(targetID);
    Vec2 targetPos = GridMapUtils::gridToPixelCenter(safeTarget->gridX, safeTarget->gridY);

    unit->attackTowardPosition(targetPos, [this, unit, troopLayer, targetID]() {
        auto dm = VillageDataManager::getInstance();
        BuildingInstance* liveTarget = dm->getBuildingById(targetID);

        if (!liveTarget || liveTarget->isDestroyed || liveTarget->currentHP <= 0) {
            startUnitAI(unit, troopLayer);
            return;
        }

        TroopInfo troopInfo = TroopConfig::getInstance()->getTroopById(1001);

        std::string ut = unit->getUnitType();
        if (ut.find("Barbarian") != std::string::npos || ut.find("barbarian") != std::string::npos || ut.find("野蛮人") != std::string::npos) {
            troopInfo = TroopConfig::getInstance()->getTroopById(1001);
        }
        else if (ut.find("Archer") != std::string::npos || ut.find("archer") != std::string::npos || ut.find("弓箭手") != std::string::npos) {
            troopInfo = TroopConfig::getInstance()->getTroopById(1002);
        }
        else if (ut.find("Goblin") != std::string::npos || ut.find("goblin") != std::string::npos || ut.find("哥布林") != std::string::npos) {
            troopInfo = TroopConfig::getInstance()->getTroopById(1003);
        }
        else if (ut.find("Giant") != std::string::npos || ut.find("giant") != std::string::npos || ut.find("巨人") != std::string::npos) {
            troopInfo = TroopConfig::getInstance()->getTroopById(1004);
        }
        else if (ut.find("Wall_Breaker") != std::string::npos || ut.find("炸弹人") != std::string::npos) {
            troopInfo = TroopConfig::getInstance()->getTroopById(1005);
        }

        int dps = troopInfo.damagePerSecond;

        if (liveTarget->isDestroyed || liveTarget->currentHP <= 0) {
            startUnitAI(unit, troopLayer);
            return;
        }

        liveTarget->currentHP -= dps;

        if (liveTarget->currentHP <= 0) {
            liveTarget->isDestroyed = true;
            liveTarget->currentHP = 0;
            
            // 关键：更新寻路地图，移除被摧毁的墙
            FindPathUtil::getInstance()->updatePathfindingMap();
            
            startUnitAI(unit, troopLayer);
            return;
        }
        else {
            unit->runAction(Sequence::create(
                CallFunc::create([this, unit, troopLayer, targetID]() {
                    auto dm = VillageDataManager::getInstance();
                    auto t = dm->getBuildingById(targetID);
                    if (t && !t->isDestroyed && t->currentHP > 0) {
                        startCombatLoopWithForcedTarget(unit, troopLayer, t);
                    }
                    else {
                        startUnitAI(unit, troopLayer);
                    }
                }),
                nullptr
            ));
        }
    });
}
