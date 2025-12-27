// TargetFinder.cpp
// 战斗目标查找器实现，为不同兵种提供智能目标选择策略

#include "TargetFinder.h"
#include "../Manager/VillageDataManager.h"
#include "../Model/BuildingConfig.h"
#include "../Util/GridMapUtils.h"
#include "../Sprite/BattleUnitSprite.h"

USING_NS_CC;

TargetFinder* TargetFinder::_instance = nullptr;

TargetFinder* TargetFinder::getInstance() {
    if (!_instance) {
        _instance = new TargetFinder();
    }
    return _instance;
}

void TargetFinder::destroyInstance() {
    if (_instance) {
        delete _instance;
        _instance = nullptr;
    }
}

// 判断是否为资源建筑
static bool isResourceBuilding(int buildingType) {
    return buildingType == 1 || buildingType == 202 || buildingType == 203 || 
           buildingType == 204 || buildingType == 205;
}

// 判断是否为防御建筑
static bool isDefenseBuilding(int buildingType) {
    return buildingType == 301 || buildingType == 302;
}

const BuildingInstance* TargetFinder::findTarget(const Vec2& unitWorldPos, UnitTypeID unitType) {
    // 炸弹兵：只攻击城墙
    if (unitType == UnitTypeID::WALL_BREAKER) {
        return findNearestWall(unitWorldPos);
    }
    
    // 哥布林：资源优先
    if (unitType == UnitTypeID::GOBLIN) {
        return findTargetWithResourcePriority(unitWorldPos, unitType);
    }
    
    // 巨人、气球：防御优先
    if (unitType == UnitTypeID::GIANT || unitType == UnitTypeID::BALLOON) {
        return findTargetWithDefensePriority(unitWorldPos, unitType);
    }
    
    // 其他兵种：资源优先（默认行为）
    return findTargetWithResourcePriority(unitWorldPos, unitType);
}

const BuildingInstance* TargetFinder::findTargetWithResourcePriority(const Vec2& unitWorldPos, UnitTypeID unitType) {
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
        
        // 跳过城墙和陷阱
        if (building.type == 303) continue;
        if (building.type >= 400 && building.type < 500) continue;

        auto config = BuildingConfig::getInstance()->getConfig(building.type);
        if (!config) continue;

        Vec2 bPos = GridMapUtils::getBuildingCenterPixel(
            building.gridX, building.gridY, 
            config->gridWidth, config->gridHeight
        );
        float distSq = unitWorldPos.distanceSquared(bPos);

        if (unitType == UnitTypeID::GOBLIN) {
            // 哥布林优先攻击资源建筑
            if (isResourceBuilding(building.type)) {
                if (distSq < minDistanceSq) {
                    minDistanceSq = distSq;
                    bestBuilding = &building;
                }
            } else {
                // 备选目标：非资源建筑
                if (distSq < fallbackMinDistanceSq) {
                    fallbackMinDistanceSq = distSq;
                    fallbackBuilding = &building;
                }
            }
        } else {
            // 其他兵种选择最近建筑
            if (distSq < minDistanceSq) {
                minDistanceSq = distSq;
                bestBuilding = &building;
            }
        }
    }

    // 哥布林如果没找到资源建筑，使用备选目标
    if (unitType == UnitTypeID::GOBLIN && !bestBuilding && fallbackBuilding) {
        return fallbackBuilding;
    }

    return bestBuilding;
}

const BuildingInstance* TargetFinder::findTargetWithDefensePriority(const Vec2& unitWorldPos, UnitTypeID unitType) {
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
        
        // 跳过城墙和陷阱
        if (building.type == 303) continue;
        if (building.type >= 400 && building.type < 500) continue;
        
        // 气球兵额外跳过城墙
        if (unitType == UnitTypeID::BALLOON && building.type == 303) continue;

        auto config = BuildingConfig::getInstance()->getConfig(building.type);
        if (!config) continue;

        Vec2 bPos = GridMapUtils::getBuildingCenterPixel(
            building.gridX, building.gridY, 
            config->gridWidth, config->gridHeight
        );
        float distSq = unitWorldPos.distanceSquared(bPos);

        if (unitType == UnitTypeID::GIANT || unitType == UnitTypeID::BALLOON) {
            // 巨人、气球优先攻击防御建筑
            if (isDefenseBuilding(building.type)) {
                if (distSq < minDistanceSq) {
                    minDistanceSq = distSq;
                    bestBuilding = &building;
                }
            } else {
                // 备选目标：非防御建筑（排除城墙）
                if (building.type != 303) {
                    if (distSq < fallbackMinDistanceSq) {
                        fallbackMinDistanceSq = distSq;
                        fallbackBuilding = &building;
                    }
                }
            }
        } else {
            // 其他兵种选择最近建筑
            if (distSq < minDistanceSq) {
                minDistanceSq = distSq;
                bestBuilding = &building;
            }
        }
    }

    // 如果没找到防御建筑，使用备选目标
    if ((unitType == UnitTypeID::GIANT || unitType == UnitTypeID::BALLOON) && !bestBuilding && fallbackBuilding) {
        return fallbackBuilding;
    }

    return bestBuilding;
}

const BuildingInstance* TargetFinder::findNearestWall(const Vec2& unitWorldPos) {
    auto dataManager = VillageDataManager::getInstance();
    const auto& buildings = dataManager->getAllBuildings();
    
    const BuildingInstance* nearestWall = nullptr;
    float minDistanceSq = FLT_MAX;
    
    for (const auto& building : buildings) {
        // 只查找城墙（type=303）
        if (building.type != 303) continue;
        if (building.isDestroyed || building.currentHP <= 0) continue;
        if (building.state == BuildingInstance::State::PLACING) continue;
        
        Vec2 bPos = GridMapUtils::gridToPixelCenter(building.gridX, building.gridY);
        float distSq = unitWorldPos.distanceSquared(bPos);
        
        if (distSq < minDistanceSq) {
            minDistanceSq = distSq;
            nearestWall = &building;
        }
    }
    
    return nearestWall;
}
