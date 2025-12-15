#include "BuildingPlacementController.h"
#include "Manager/VillageDataManager.h"
#include "Manager/BuildingManager.h"
#include "Model/BuildingConfig.h"
#include "Sprite/BuildingSprite.h"

BuildingPlacementController::BuildingPlacementController()
  : _currentBuildingId(-1)
  , _isPlacing(false) {}

BuildingPlacementController::~BuildingPlacementController() {}

void BuildingPlacementController::startPlacement(int buildingId) {
  _currentBuildingId = buildingId;
  _isPlacing = true;

  CCLOG("BuildingPlacementController: Started placement for building ID=%d", buildingId);
}

void BuildingPlacementController::cancelPlacement() {
  if (!_isPlacing) return;

  CCLOG("BuildingPlacementController: Placement cancelled for building ID=%d", _currentBuildingId);

  // 从数据层删除建筑数据
  auto dataManager = VillageDataManager::getInstance();
  dataManager->removeBuilding(_currentBuildingId);

  // 通知删除成功（需要通过回调）
  if (_placementCallback) {
    _placementCallback(false, _currentBuildingId);
  }

  _isPlacing = false;
  _currentBuildingId = -1;
}

bool BuildingPlacementController::confirmPlacement() {
  if (!_isPlacing) return false;

  auto dataManager = VillageDataManager::getInstance();
  auto building = dataManager->getBuildingById(_currentBuildingId);

  if (!building) {
    CCLOG("BuildingPlacementController: ERROR - Building not found");
    return false;
  }

  if (!canPlaceAtCurrentPosition()) {
    CCLOG("BuildingPlacementController: Cannot place building at current position");
    return false;
  }

  // ========== 调用统一的放置完成方法 ==========
  // 这个方法会自动处理：
  //   - 工人小屋（ID=201）→ 瞬间完成 → BUILT 状态
  //   - 其他建筑 → 进入 CONSTRUCTING 状态
  bool success = dataManager->startConstructionAfterPlacement(_currentBuildingId);

  if (!success) {
    CCLOG("BuildingPlacementController: ERROR - Failed to start construction");
    return false;
  }
  // =====================================================

  CCLOG("BuildingPlacementController: Building placed successfully");

  if (_placementCallback) {
    _placementCallback(true, _currentBuildingId);
  }

  _isPlacing = false;
  _currentBuildingId = -1;

  return true;
}

bool BuildingPlacementController::canPlaceAtCurrentPosition() const {
  if (!_isPlacing) return false;

  auto dataManager = VillageDataManager::getInstance();
  auto building = dataManager->getBuildingById(_currentBuildingId);

  if (!building) return false;

  auto config = BuildingConfig::getInstance()->getConfig(building->type);
  if (!config) return false;

  // 检查区域是否占用
  return !dataManager->isAreaOccupied(
    building->gridX,
    building->gridY,
    config->gridWidth,
    config->gridHeight,
    _currentBuildingId  // 忽略自己
  );
}

void BuildingPlacementController::setPlacementCallback(PlacementCallback callback) {
  _placementCallback = callback;
}