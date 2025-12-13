// Classes/Manager/BuildingSpeedupManager.cpp
#include "BuildingSpeedupManager.h"
#include "VillageDataManager.h"
#include "BuildingManager.h"  // 需要添加这个头文件

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

  // 检查1:建筑必须在建造中
  if (building->state != BuildingInstance::State::CONSTRUCTING) {
    return false;
  }

  // 检查2:必须有至少1颗宝石
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

  // 1. 消耗1颗宝石
  if (!dataManager->spendGem(1)) {
    CCLOG("BuildingSpeedupManager: Failed to spend gem");
    return false;
  }

  // 2. 立即完成建造(这会改变数据层状态为 BUILT)
  auto building = dataManager->getBuildingById(buildingId);
  if (building->isInitialConstruction) {
    // 首次建造完成
    dataManager->finishNewBuildingConstruction(buildingId);
  } else {
    // 升级完成
    dataManager->finishUpgradeBuilding(buildingId);
  }

  // ?? 关键修复:直接通知 BuildingManager 更新 UI
  // 触发自定义事件,让 VillageLayer/BuildingManager 更新 BuildingSprite
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