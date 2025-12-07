#include "BuildingManager.h"
#include "Manager/VillageDataManager.h"
#include "Model/BuildingConfig.h"

USING_NS_CC;

BuildingManager::BuildingManager(Layer* parentLayer)
  : _parentLayer(parentLayer) {
  // 从数据加载建筑
  loadBuildingsFromData();
}

BuildingManager::~BuildingManager() {
  _buildings.clear();
}

void BuildingManager::loadBuildingsFromData() {
  auto dataManager = VillageDataManager::getInstance();
  auto& buildings = dataManager->getAllBuildings();

  for (const auto& building : buildings) {
    // 只显示已建造或建造中的建筑（跳过待放置状态）
    if (building.state != BuildingInstance::State::PLACING) {
      addBuilding(building);
    }
  }
  
  CCLOG("BuildingManager: Loaded %lu buildings", _buildings.size());
}

BuildingSprite* BuildingManager::addBuilding(const BuildingInstance& building) {
  // 创建建筑精灵
  auto sprite = BuildingSprite::create(building);
  if (!sprite) {
    CCLOG("BuildingManager: Failed to create building sprite for ID: %d", building.id);
    return nullptr;
  }

  // 设置位置
  Vec2 worldPos = gridToWorld(building.gridX, building.gridY);
  sprite->setPosition(worldPos);

  // 添加到父层
  _parentLayer->addChild(sprite, building.gridY); // Z-order根据Y坐标实现伪3D

  // 存储映射
  _buildings[building.id] = sprite;

  CCLOG("BuildingManager: Added building ID=%d at grid(%d, %d) world(%.0f, %.0f)", 
        building.id, building.gridX, building.gridY, worldPos.x, worldPos.y);

  return sprite;
}

void BuildingManager::removeBuilding(int buildingId) {
  auto it = _buildings.find(buildingId);
  if (it != _buildings.end()) {
    // 从场景中移除
    it->second->removeFromParent();
    // 从映射表中移除
    _buildings.erase(it);
    
    CCLOG("BuildingManager: Removed building ID=%d", buildingId);
  }
}

void BuildingManager::updateBuilding(int buildingId, const BuildingInstance& building) {
  auto sprite = getBuildingSprite(buildingId);
  if (sprite) {
    sprite->updateBuilding(building);
    
    // 更新位置
    Vec2 worldPos = gridToWorld(building.gridX, building.gridY);
    sprite->setPosition(worldPos);
  }
}

BuildingSprite* BuildingManager::getBuildingSprite(int buildingId) const {
  auto it = _buildings.find(buildingId);
  if (it != _buildings.end()) {
    return it->second;
  }
  return nullptr;
}

BuildingSprite* BuildingManager::getBuildingAtGrid(int gridX, int gridY) const {
  for (const auto& pair : _buildings) {
    auto sprite = pair.second;
    Vec2 spriteGrid = sprite->getGridPos();
    Size spriteSize = sprite->getGridSize();
    
    // 检查点是否在建筑占用的网格范围内
    if (gridX >= spriteGrid.x && gridX < spriteGrid.x + spriteSize.width &&
        gridY >= spriteGrid.y && gridY < spriteGrid.y + spriteSize.height) {
      return sprite;
    }
  }
  return nullptr;
}

Vec2 BuildingManager::gridToWorld(int gridX, int gridY) const {
  // 等距视角转换公式
  float worldX = (gridX - gridY) * (GRID_WIDTH / 2.0f);
  float worldY = (gridX + gridY) * (GRID_HEIGHT / 2.0f);

  // 加上地图偏移（需要根据实际地图大小调整）
  worldX += 2000;
  worldY += 1000;

  return Vec2(worldX, worldY);
}

Vec2 BuildingManager::gridToWorld(const Vec2& gridPos) const {
  return gridToWorld(static_cast<int>(gridPos.x), static_cast<int>(gridPos.y));
}

Vec2 BuildingManager::worldToGrid(const Vec2& worldPos) const {
  // 逆向转换
  float adjustedX = worldPos.x - 2000;
  float adjustedY = worldPos.y - 1000;

  int gridX = static_cast<int>(std::round((adjustedX / (GRID_WIDTH / 2.0f) + adjustedY / (GRID_HEIGHT / 2.0f)) / 2.0f));
  int gridY = static_cast<int>(std::round((adjustedY / (GRID_HEIGHT / 2.0f) - adjustedX / (GRID_WIDTH / 2.0f)) / 2.0f));

  return Vec2(gridX, gridY);
}

void BuildingManager::update(float dt) {
  // 检查建造中的建筑是否完成
  auto dataManager = VillageDataManager::getInstance();
  auto& buildings = dataManager->getAllBuildings();

  long long currentTime = time(nullptr);

  for (auto& building : buildings) {
    if (building.state == BuildingInstance::State::CONSTRUCTING &&
        building.finishTime > 0 &&
        currentTime >= building.finishTime) {

      // 建造完成
      dataManager->setBuildingState(building.id, BuildingInstance::State::BUILT, 0);

      // 更新精灵显示
      auto sprite = getBuildingSprite(building.id);
      if (sprite) {
        sprite->hideConstructionProgress();
        sprite->updateState(BuildingInstance::State::BUILT);
        
        CCLOG("BuildingManager: Building ID=%d construction completed", building.id);
      }
    } else if (building.state == BuildingInstance::State::CONSTRUCTING) {
      // 更新建造进度
      auto sprite = getBuildingSprite(building.id);
      if (sprite && building.finishTime > 0) {
        long long remainTime = building.finishTime - currentTime;
        if (remainTime > 0) {
          // 假设总建造时间（这里简化处理，实际应从配置读取）
          long long totalTime = 300; // 默认5分钟
          float progress = 1.0f - (static_cast<float>(remainTime) / totalTime);
          progress = clampf(progress, 0.0f, 1.0f);
          
          sprite->showConstructionProgress(progress);
          sprite->showCountdown(static_cast<int>(remainTime));
        }
      }
    }
  }
}