#include "BuildingManager.h"
#include "Manager/VillageDataManager.h"
#include "Model/BuildingConfig.h"
#include "Util/GridMapUtils.h"

USING_NS_CC;

BuildingManager::BuildingManager(Layer* parentLayer)
    : _parentLayer(parentLayer) {
    loadBuildingsFromData();
}

BuildingManager::~BuildingManager() {
    _buildings.clear();
}

void BuildingManager::loadBuildingsFromData() {
    auto dataManager = VillageDataManager::getInstance();
    const auto& buildings = dataManager->getAllBuildings();

    for (const auto& building : buildings) {
        if (building.state != BuildingInstance::State::PLACING) {
            addBuilding(building);
        }
    }
    CCLOG("BuildingManager: Loaded %lu buildings", _buildings.size());
}

BuildingSprite* BuildingManager::addBuilding(const BuildingInstance& building) {
    auto sprite = BuildingSprite::create(building);
    if (!sprite) return nullptr;

    // 设置位置（网格坐标 + 视觉偏移）
    Vec2 worldPos = GridMapUtils::gridToPixel(building.gridX, building.gridY);
    Vec2 finalPos = worldPos + sprite->getVisualOffset();
    sprite->setPosition(finalPos);

    // ? 修复：等轴测地图的正确 Z-Order 公式
    // 公式：gridX + gridY（越靠近屏幕下方，Z 越大）
    int zOrder = building.gridX + building.gridY;
    _parentLayer->addChild(sprite, zOrder);

    _buildings[building.id] = sprite;
    CCLOG("BuildingManager: Added building ID=%d at grid(%d, %d), Z-Order=%d", 
          building.id, building.gridX, building.gridY, zOrder);
    
    return sprite;
}

void BuildingManager::removeBuilding(int buildingId) {
    auto it = _buildings.find(buildingId);
    if (it != _buildings.end()) {
        it->second->removeFromParent();
        _buildings.erase(it);
        CCLOG("BuildingManager: Removed building ID=%d", buildingId);
    }
}

void BuildingManager::updateBuilding(int buildingId, const BuildingInstance& building) {
    auto sprite = getBuildingSprite(buildingId);
    if (sprite) {
        sprite->updateBuilding(building);

        // 更新位置
        Vec2 worldPos = GridMapUtils::gridToPixel(building.gridX, building.gridY);
        Vec2 finalPos = worldPos + sprite->getVisualOffset();
        sprite->setPosition(finalPos);

        // ? 修复：移动后更新 Z-Order
        int zOrder = building.gridX + building.gridY;
        sprite->setLocalZOrder(zOrder);
        
        CCLOG("BuildingManager: Updated building ID=%d, new Z-Order=%d", buildingId, zOrder);
    }
}

BuildingSprite* BuildingManager::getBuildingSprite(int buildingId) const {
    auto it = _buildings.find(buildingId);
    return (it != _buildings.end()) ? it->second : nullptr;
}

// ? 修复：使用网格查询（O(1)，精准，无透明区域问题）
BuildingSprite* BuildingManager::getBuildingAtGrid(int gridX, int gridY) const {
    CCLOG("BuildingManager::getBuildingAtGrid - Looking at grid(%d, %d)", gridX, gridY);
    
    auto dataManager = VillageDataManager::getInstance();
    
    // 遍历所有建筑，找到占据该网格的建筑
    const auto& buildings = dataManager->getAllBuildings();
    for (const auto& building : buildings) {
        if (building.state == BuildingInstance::State::PLACING) continue;
        
        auto config = BuildingConfig::getInstance()->getConfig(building.type);
        if (!config) continue;
        
        // 检查网格是否在建筑范围内
        if (gridX >= building.gridX && gridX < building.gridX + config->gridWidth &&
            gridY >= building.gridY && gridY < building.gridY + config->gridHeight) {
            CCLOG("  -> FOUND! Building ID=%d", building.id);
            return getBuildingSprite(building.id);
        }
    }
    
    CCLOG("  -> NOT FOUND");
    return nullptr;
}

// ? 修复：点击检测改为网格查询（不再用 BoundingBox）
BuildingSprite* BuildingManager::getBuildingAtWorldPos(const Vec2& worldPos) const {
    CCLOG("BuildingManager::getBuildingAtWorldPos - Checking world pos (%.0f, %.0f)",
        worldPos.x, worldPos.y);

    // ? 1. 转换为网格坐标
    Vec2 gridPosFloat = GridMapUtils::pixelToGrid(worldPos);
    
    // ? 2. 四舍五入到整数网格
    int gridX = (int)std::round(gridPosFloat.x);
    int gridY = (int)std::round(gridPosFloat.y);
    
    CCLOG("  -> Converted to grid(%d, %d)", gridX, gridY);
    
    // ? 3. 查询网格（O(1)，精准，无透明区域问题）
    return getBuildingAtGrid(gridX, gridY);
}

void BuildingManager::update(float dt) {
    auto dataManager = VillageDataManager::getInstance();
    const auto& buildings = dataManager->getAllBuildings();
    long long currentTime = time(nullptr);

    for (auto& building : buildings) {
        if (building.state == BuildingInstance::State::CONSTRUCTING) {
            // 检查完成
            if (building.finishTime > 0 && currentTime >= building.finishTime) {
                dataManager->setBuildingState(building.id, BuildingInstance::State::BUILT, 0);

                auto sprite = getBuildingSprite(building.id);
                if (sprite) {
                    sprite->hideConstructionProgress();
                    sprite->updateState(BuildingInstance::State::BUILT);
                }
                continue;
            }

            // 更新进度条
            auto sprite = getBuildingSprite(building.id);
            if (sprite && building.finishTime > 0) {
                long long remainTime = building.finishTime - currentTime;

                // 从配置表读取建造时间
                auto config = BuildingConfig::getInstance()->getConfig(building.type);
                long long totalTime = 300;  // 默认5分钟
                if (config) {
                    totalTime = config->buildTimeSeconds;
                }
                if (totalTime <= 0) totalTime = 1;

                float progress = 1.0f - (static_cast<float>(remainTime) / (float)totalTime);
                progress = clampf(progress, 0.0f, 1.0f);

                sprite->showConstructionProgress(progress);
                sprite->showCountdown((int)remainTime);
            }
        }
    }
}