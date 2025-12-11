#include "BuildingUpgradeManager.h"
#include "VillageDataManager.h"
#include <ctime>

BuildingUpgradeManager* BuildingUpgradeManager::_instance = nullptr;

BuildingUpgradeManager::BuildingUpgradeManager() {}
BuildingUpgradeManager::~BuildingUpgradeManager() {}

BuildingUpgradeManager* BuildingUpgradeManager::getInstance() {
  if (!_instance) {
    _instance = new BuildingUpgradeManager();
  }
  return _instance;
}

void BuildingUpgradeManager::destroyInstance() {
  if (_instance) {
    delete _instance;
    _instance = nullptr;
  }
}

void BuildingUpgradeManager::update(float dt) {
  // 每秒检查一次(避免频繁检查)
  static float timer = 0;
  timer += dt;
  if (timer >= 1.0f) {
    timer = 0;
    checkFinishedUpgrades();
  }
}

void BuildingUpgradeManager::checkFinishedUpgrades() {
  auto dataManager = VillageDataManager::getInstance();
  long long currentTime = time(nullptr);

  for (const auto& building : dataManager->getAllBuildings()) {
    if (building.state == BuildingInstance::State::CONSTRUCTING) {
      if (currentTime >= building.finishTime) {
        // 升级完成
        dataManager->finishUpgradeBuilding(building.id);
        CCLOG("BuildingUpgradeManager: Building %d upgrade finished", building.id);
      }
    }
  }
}