#include "BuildingUpgradeManager.h"
#include "VillageDataManager.h"
#include "Model/BuildingRequirements.h"
#include "Model/BuildingConfig.h" 
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

bool BuildingUpgradeManager::canUpgrade(int buildingId) {
  auto dataManager = VillageDataManager::getInstance();
  auto building = dataManager->getBuildingById(buildingId);

  if (!building) {
    CCLOG("BuildingUpgradeManager: Building %d not found", buildingId);
    return false;
  }

  if (building->state != BuildingInstance::State::BUILT) {
    CCLOG("BuildingUpgradeManager: Building %d is not in BUILT state", buildingId);
    return false;
  }

  // 修改1：获取 BuildingConfig 实例而不是 BuildingConfigData 指针
  auto configManager = BuildingConfig::getInstance();
  auto configData = configManager->getConfig(building->type);

  if (!configData) {
    return false;
  }

  // 修改2：使用 BuildingConfig 的 canUpgrade 方法
  if (!configManager->canUpgrade(building->type, building->level)) {
    CCLOG("BuildingUpgradeManager: Building %d is already max level", buildingId);
    return false;
  }

  // 检查大本营等级限制
  auto requirements = BuildingRequirements::getInstance();
  int currentTHLevel = dataManager->getTownHallLevel();

  if (!requirements->canUpgrade(building->type, building->level, currentTHLevel)) {
    int requiredTH = requirements->getRequiredTHLevel(building->type, building->level + 1);
    CCLOG("BuildingUpgradeManager: Upgrade requires TH level %d (current: %d)",
          requiredTH, currentTHLevel);
    return false;
  }

  // 修改3：使用 BuildingConfig 的 getUpgradeCost 方法
  int cost = configManager->getUpgradeCost(building->type, building->level);

  if (configData->costType == "金币") {
    if (dataManager->getGold() < cost) {
      CCLOG("BuildingUpgradeManager: Not enough gold");
      return false;
    }
  } else if (configData->costType == "圣水") {
    if (dataManager->getElixir() < cost) {
      CCLOG("BuildingUpgradeManager: Not enough elixir");
      return false;
    }
  }

  // 检查建造工人（如果有 BuilderManager）
  // if (!builderManager->hasAvailableBuilder()) {
  //     return false;
  // }

  return true;
}

std::string BuildingUpgradeManager::getUpgradeFailReason(int buildingId) const {
  auto dataManager = VillageDataManager::getInstance();
  auto building = dataManager->getBuildingById(buildingId);

  if (!building) {
    return "建筑不存在";
  }

  if (building->state != BuildingInstance::State::BUILT) {
    return "建筑未完成建造";
  }

  // 修改4：同样修改这里
  auto configManager = BuildingConfig::getInstance();
  auto configData = configManager->getConfig(building->type);

  if (!configData) {
    return "配置错误";
  }

  // 修改5：使用 configManager 调用方法
  if (!configManager->canUpgrade(building->type, building->level)) {
    return "已达最高等级";
  }

  auto requirements = BuildingRequirements::getInstance();
  int currentTHLevel = dataManager->getTownHallLevel();

  if (!requirements->canUpgrade(building->type, building->level, currentTHLevel)) {
    int requiredTH = requirements->getRequiredTHLevel(building->type, building->level + 1);
    return "需要" + std::to_string(requiredTH) + "级大本营";
  }

  // 修改6：使用 configManager 获取升级成本
  int cost = configManager->getUpgradeCost(building->type, building->level);

  if (configData->costType == "金币") {
    if (dataManager->getGold() < cost) {
      return "金币不足（需要" + std::to_string(cost) + "）";
    }
  } else if (configData->costType == "圣水") {
    if (dataManager->getElixir() < cost) {
      return "圣水不足（需要" + std::to_string(cost) + "）";
    }
  }

  return "";
}