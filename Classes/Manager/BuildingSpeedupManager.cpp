// BuildingSpeedupManager.cpp
// 建筑加速管理器实现，处理建筑加速逻辑

#include "BuildingSpeedupManager.h"
#include "VillageDataManager.h"
#include "BuildingManager.h"

BuildingSpeedupManager* BuildingSpeedupManager::instance = nullptr;

BuildingSpeedupManager::BuildingSpeedupManager() {}

BuildingSpeedupManager::~BuildingSpeedupManager() {}

BuildingSpeedupManager* BuildingSpeedupManager::getInstance() {
  if (!instance) {
    instance = new BuildingSpeedupManager();
  }
  return instance;
}

void BuildingSpeedupManager::destroyInstance() {
  if (instance) {
    delete instance;
    instance = nullptr;
  }
}

bool BuildingSpeedupManager::canSpeedup(int buildingId) const {
  auto dataManager = VillageDataManager::getInstance();
  auto building = dataManager->getBuildingById(buildingId);

  if (!building) {
    return false;
  }

  // 检查建筑必须在建造中
  if (building->state != BuildingInstance::State::CONSTRUCTING) {
    return false;
  }

  // 检查必须有至少1颗宝石
  if (dataManager->getGem() < 1) {
    return false;
  }

  return true;
}

bool BuildingSpeedupManager::speedupBuilding(int buildingId) {
  if (!canSpeedup(buildingId)) {
    return false;
  }

  auto dataManager = VillageDataManager::getInstance();

  // 消耗1颗宝石
  if (!dataManager->spendGem(1)) {
    CCLOG("BuildingSpeedupManager: Failed to spend gem");
    return false;
  }

  // 立即完成建造
  auto building = dataManager->getBuildingById(buildingId);
  if (building->isInitialConstruction) {
    dataManager->finishNewBuildingConstruction(buildingId);
  } else {
    dataManager->finishUpgradeBuilding(buildingId);
  }

  // 通知BuildingManager更新UI
  cocos2d::EventCustom event("EVENT_BUILDING_SPEEDUP_COMPLETE");
  int* data = new int(buildingId);
  event.setUserData(data);
  cocos2d::Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
  delete data;

  CCLOG("BuildingSpeedupManager: Building %d speedup completed (1 gem consumed)", buildingId);
  return true;
}

std::string BuildingSpeedupManager::getSpeedupFailReason(int buildingId) const {
  auto dataManager = VillageDataManager::getInstance();
  auto building = dataManager->getBuildingById(buildingId);

  if (!building) {
    return "建筑不存在";
  }

  if (building->state != BuildingInstance::State::CONSTRUCTING) {
    return "建筑未在建造中";
  }

  if (dataManager->getGem() < 1) {
    return "宝石不足(需要1颗)";
  }

  return "";
}
