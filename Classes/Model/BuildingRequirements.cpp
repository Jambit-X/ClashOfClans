// BuildingRequirements.cpp
// 建筑解锁和升级需求管理器实现

#include "BuildingRequirements.h"
#include "cocos2d.h"
#include "Manager/VillageDataManager.h"

USING_NS_CC;

BuildingRequirements* BuildingRequirements::instance = nullptr;

BuildingRequirements::BuildingRequirements() {
  initializeRules();
}

BuildingRequirements::~BuildingRequirements() {
  rules.clear();
}

BuildingRequirements* BuildingRequirements::getInstance() {
  if (!instance) {
    instance = new BuildingRequirements();
  }
  return instance;
}

void BuildingRequirements::destroyInstance() {
  if (instance) {
    delete instance;
    instance = nullptr;
  }
}

void BuildingRequirements::initializeRules() {
  rules.clear();

  // 军队建筑

  // 大本营
  {
    BuildingUnlockRule rule;
    rule.buildingType = 1;
    rule.minTownHallLevel = 1;

    rule.thLevelToMaxCount[1] = 1;
    rule.thLevelToMaxCount[2] = 1;
    rule.thLevelToMaxCount[3] = 1;

    rule.buildingLevelToTH[1] = 1;
    rule.buildingLevelToTH[2] = 2;
    rule.buildingLevelToTH[3] = 3;

    addRule(rule);
  }

  // 兵营
  {
    BuildingUnlockRule rule;
    rule.buildingType = 101;
    rule.minTownHallLevel = 1;

    rule.thLevelToMaxCount[1] = 1;
    rule.thLevelToMaxCount[2] = 2;
    rule.thLevelToMaxCount[3] = 3;

    rule.buildingLevelToTH[1] = 1;
    rule.buildingLevelToTH[2] = 2;
    rule.buildingLevelToTH[3] = 3;

    addRule(rule);
  }

  // 训练营
  {
    BuildingUnlockRule rule;
    rule.buildingType = 102;
    rule.minTownHallLevel = 1;

    rule.thLevelToMaxCount[1] = 1;
    rule.thLevelToMaxCount[2] = 1;
    rule.thLevelToMaxCount[3] = 1;

    rule.buildingLevelToTH[1] = 1;
    rule.buildingLevelToTH[2] = 2;
    rule.buildingLevelToTH[3] = 3;

    addRule(rule);
  }

  // 实验室
  {
    BuildingUnlockRule rule;
    rule.buildingType = 103;
    rule.minTownHallLevel = 1;

    rule.thLevelToMaxCount[1] = 1;
    rule.thLevelToMaxCount[2] = 1;
    rule.thLevelToMaxCount[3] = 1;

    rule.buildingLevelToTH[1] = 1;
    rule.buildingLevelToTH[2] = 2;
    rule.buildingLevelToTH[3] = 3;

    addRule(rule);
  }

  // 法术工厂（大本2解锁）
  {
    BuildingUnlockRule rule;
    rule.buildingType = 104;
    rule.minTownHallLevel = 2;

    rule.thLevelToMaxCount[2] = 1;
    rule.thLevelToMaxCount[3] = 1;

    rule.buildingLevelToTH[1] = 2;
    rule.buildingLevelToTH[2] = 2;
    rule.buildingLevelToTH[3] = 3;

    addRule(rule);
  }

  // 资源建筑

  // 建筑工人小屋
  {
    BuildingUnlockRule rule;
    rule.buildingType = 201;
    rule.minTownHallLevel = 1;

    rule.thLevelToMaxCount[1] = 5;
    rule.thLevelToMaxCount[2] = 5;
    rule.thLevelToMaxCount[3] = 5;

    rule.buildingLevelToTH[1] = 1;

    addRule(rule);
  }

  // 金矿
  {
    BuildingUnlockRule rule;
    rule.buildingType = 202;
    rule.minTownHallLevel = 1;

    rule.thLevelToMaxCount[1] = 1;
    rule.thLevelToMaxCount[2] = 2;
    rule.thLevelToMaxCount[3] = 3;

    rule.buildingLevelToTH[1] = 1;
    rule.buildingLevelToTH[2] = 2;
    rule.buildingLevelToTH[3] = 3;

    addRule(rule);
  }

  // 圣水采集器
  {
    BuildingUnlockRule rule;
    rule.buildingType = 203;
    rule.minTownHallLevel = 1;

    rule.thLevelToMaxCount[1] = 1;
    rule.thLevelToMaxCount[2] = 2;
    rule.thLevelToMaxCount[3] = 3;

    rule.buildingLevelToTH[1] = 1;
    rule.buildingLevelToTH[2] = 2;
    rule.buildingLevelToTH[3] = 3;

    addRule(rule);
  }

  // 储金罐
  {
    BuildingUnlockRule rule;
    rule.buildingType = 204;
    rule.minTownHallLevel = 1;

    rule.thLevelToMaxCount[1] = 1;
    rule.thLevelToMaxCount[2] = 1;
    rule.thLevelToMaxCount[3] = 2;

    rule.buildingLevelToTH[1] = 1;
    rule.buildingLevelToTH[2] = 2;
    rule.buildingLevelToTH[3] = 3;

    addRule(rule);
  }

  // 圣水瓶
  {
    BuildingUnlockRule rule;
    rule.buildingType = 205;
    rule.minTownHallLevel = 1;

    rule.thLevelToMaxCount[1] = 1;
    rule.thLevelToMaxCount[2] = 1;
    rule.thLevelToMaxCount[3] = 2;

    rule.buildingLevelToTH[1] = 1;
    rule.buildingLevelToTH[2] = 2;
    rule.buildingLevelToTH[3] = 3;

    addRule(rule);
  }

  // 防御建筑

  // 加农炮
  {
    BuildingUnlockRule rule;
    rule.buildingType = 301;
    rule.minTownHallLevel = 1;

    rule.thLevelToMaxCount[1] = 2;
    rule.thLevelToMaxCount[2] = 3;
    rule.thLevelToMaxCount[3] = 4;

    rule.buildingLevelToTH[1] = 1;
    rule.buildingLevelToTH[2] = 2;
    rule.buildingLevelToTH[3] = 3;

    addRule(rule);
  }

  // 箭塔
  {
    BuildingUnlockRule rule;
    rule.buildingType = 302;
    rule.minTownHallLevel = 1;

    rule.thLevelToMaxCount[1] = 1;
    rule.thLevelToMaxCount[2] = 2;
    rule.thLevelToMaxCount[3] = 3;

    rule.buildingLevelToTH[1] = 1;
    rule.buildingLevelToTH[2] = 2;
    rule.buildingLevelToTH[3] = 3;

    addRule(rule);
  }

  // 城墙
  {
    BuildingUnlockRule rule;
    rule.buildingType = 303;
    rule.minTownHallLevel = 1;

    rule.thLevelToMaxCount[1] = 25;
    rule.thLevelToMaxCount[2] = 50;
    rule.thLevelToMaxCount[3] = 75;

    rule.buildingLevelToTH[1] = 1;
    rule.buildingLevelToTH[2] = 2;
    rule.buildingLevelToTH[3] = 3;

    addRule(rule);
  }

  // 陷阱

  // 炸弹
  {
    BuildingUnlockRule rule;
    rule.buildingType = 401;
    rule.minTownHallLevel = 1;

    rule.thLevelToMaxCount[1] = 2;
    rule.thLevelToMaxCount[2] = 3;
    rule.thLevelToMaxCount[3] = 4;

    rule.buildingLevelToTH[1] = 1;
    rule.buildingLevelToTH[2] = 2;
    rule.buildingLevelToTH[3] = 3;

    addRule(rule);
  }

  // 弹簧陷阱（大本2解锁）
  {
    BuildingUnlockRule rule;
    rule.buildingType = 402;
    rule.minTownHallLevel = 2;

    rule.thLevelToMaxCount[2] = 2;
    rule.thLevelToMaxCount[3] = 3;

    rule.buildingLevelToTH[1] = 2;
    rule.buildingLevelToTH[2] = 2;
    rule.buildingLevelToTH[3] = 3;

    addRule(rule);
  }

  // 巨型炸弹（大本3解锁）
  {
    BuildingUnlockRule rule;
    rule.buildingType = 404;
    rule.minTownHallLevel = 3;

    rule.thLevelToMaxCount[3] = 2;

    rule.buildingLevelToTH[1] = 3;
    rule.buildingLevelToTH[2] = 3;
    rule.buildingLevelToTH[3] = 3;

    addRule(rule);
  }

  CCLOG("BuildingRequirements: Initialized %lu building rules", rules.size());
}

void BuildingRequirements::addRule(const BuildingUnlockRule& rule) {
  rules[rule.buildingType] = rule;
}

bool BuildingRequirements::canPurchase(int buildingType, int currentTHLevel, int currentCount) const {
  auto it = rules.find(buildingType);
  if (it == rules.end()) {
    CCLOG("BuildingRequirements: No rule found for building type %d", buildingType);
    return false;
  }

  const BuildingUnlockRule& rule = it->second;

  // 检查大本营等级是否足够解锁
  if (currentTHLevel < rule.minTownHallLevel) {
    return false;
  }

  // 检查数量是否已达上限
  int maxCount = getMaxCount(buildingType, currentTHLevel);
  if (currentCount >= maxCount) {
    return false;
  }

  return true;
}

bool BuildingRequirements::canUpgrade(int buildingType, int currentBuildingLevel, int currentTHLevel) const {
  auto it = rules.find(buildingType);
  if (it == rules.end()) {
    return false;
  }

  const BuildingUnlockRule& rule = it->second;
  int targetLevel = currentBuildingLevel + 1;

  // 不能超过最大等级3
  if (targetLevel > 3) {
    return false;
  }

  // 检查大本营等级要求
  auto upgradeIt = rule.buildingLevelToTH.find(targetLevel);
  if (upgradeIt == rule.buildingLevelToTH.end()) {
    return false;
  }

  int requiredTHLevel = upgradeIt->second;
  return currentTHLevel >= requiredTHLevel;
}

int BuildingRequirements::getMaxCount(int buildingType, int townHallLevel) const {
  auto it = rules.find(buildingType);
  if (it == rules.end()) {
    return 0;
  }

  const BuildingUnlockRule& rule = it->second;

  // 查找当前大本营等级下的最大数量
  int maxCount = 0;
  for (const auto& pair : rule.thLevelToMaxCount) {
    if (pair.first <= townHallLevel) {
      maxCount = std::max(maxCount, pair.second);
    }
  }

  return maxCount;
}

int BuildingRequirements::getRequiredTHLevel(int buildingType, int targetBuildingLevel) const {
  auto it = rules.find(buildingType);
  if (it == rules.end()) {
    return 1;
  }

  const BuildingUnlockRule& rule = it->second;
  auto upgradeIt = rule.buildingLevelToTH.find(targetBuildingLevel);

  if (upgradeIt != rule.buildingLevelToTH.end()) {
    return upgradeIt->second;
  }

  return 1;
}

int BuildingRequirements::getMinTHLevel(int buildingType) const {
  auto it = rules.find(buildingType);
  if (it != rules.end()) {
    return it->second.minTownHallLevel;
  }
  return 1;
}

std::string BuildingRequirements::getRestrictionReason(int buildingType, int currentLevel, int currentTHLevel, int currentCount) const {
  auto it = rules.find(buildingType);
  if (it == rules.end()) {
    return "未知的建筑类型";
  }

  const BuildingUnlockRule& rule = it->second;

  // 购买新建筑
  if (currentLevel == 0) {
    if (currentTHLevel < rule.minTownHallLevel) {
      return "需要" + std::to_string(rule.minTownHallLevel) + "级大本营解锁";
    }

    int maxCount = getMaxCount(buildingType, currentTHLevel);
    if (currentCount >= maxCount) {
      return "已达到数量上限（最多" + std::to_string(maxCount) + "个）";
    }
  }
  // 升级建筑
  else {
    int targetLevel = currentLevel + 1;

    if (targetLevel > 3) {
      return "已达到最高等级（3级）";
    }

    auto upgradeIt = rule.buildingLevelToTH.find(targetLevel);
    if (upgradeIt == rule.buildingLevelToTH.end()) {
      return "已达到最高等级";
    }

    int requiredTHLevel = upgradeIt->second;
    if (currentTHLevel < requiredTHLevel) {
      return "需要" + std::to_string(requiredTHLevel) + "级大本营";
    }
  }

  return "";
}

bool BuildingRequirements::canUpgradeBuilding(int buildingType, int currentLevel, int townHallLevel) const {
  // 大本营特殊处理
  if (buildingType == 1) {
    CCLOG("BuildingRequirements: Checking Town Hall upgrade");
    return canUpgradeTownHall(currentLevel);
  }

  // 其他建筑等级不能超过大本营等级
  if (currentLevel >= townHallLevel) {
    CCLOG("BuildingRequirements: Building level %d cannot exceed Town Hall level %d",
          currentLevel, townHallLevel);
    return false;
  }

  // 检查最大等级
  auto it = rules.find(buildingType);
  if (it == rules.end()) {
    CCLOG("BuildingRequirements: No requirements found for building type %d", buildingType);
    return false;
  }

  int maxLevel = 3;
  if (currentLevel >= maxLevel) {
    CCLOG("BuildingRequirements: Building already at max level %d", maxLevel);
    return false;
  }

  return true;
}

bool BuildingRequirements::canUpgradeTownHall(int currentTownHallLevel) const {
  auto dataManager = VillageDataManager::getInstance();

  // 获取当前大本营等级要求的防御建筑类型
  std::vector<int> requiredDefenses = getRequiredDefenseTypes(currentTownHallLevel);

  CCLOG("BuildingRequirements: Checking Town Hall level %d upgrade conditions", currentTownHallLevel);
  CCLOG("BuildingRequirements: Required defense types: %lu", requiredDefenses.size());

  // 检查每种防御建筑是否达到要求数量
  for (int defenseType : requiredDefenses) {
    int requiredCount = getMaxCount(defenseType, currentTownHallLevel);
    int currentCount = 0;

    // 统计已建造完成的防御建筑
    for (const auto& building : dataManager->getAllBuildings()) {
      if (building.type == defenseType && building.state == BuildingInstance::State::BUILT) {
        currentCount++;
      }
    }

    if (currentCount < requiredCount) {
      CCLOG("BuildingRequirements: Missing defense - Type %d: %d/%d built",
            defenseType, currentCount, requiredCount);
      return false;
    } else {
      CCLOG("BuildingRequirements: Defense Type %d: %d/%d built ?",
            defenseType, currentCount, requiredCount);
    }
  }

  CCLOG("BuildingRequirements: All defense requirements met! Town Hall can upgrade ?");
  return true;
}

std::vector<int> BuildingRequirements::getRequiredDefenseTypes(int townHallLevel) const {
  std::vector<int> defenseTypes;

  // 防御建筑ID范围：301-399（排除城墙303）
  for (const auto& pair : rules) {
    int buildingType = pair.first;
    // 排除城墙作为升级大本营的必要条件
    if (buildingType >= 301 && buildingType <= 399 && buildingType != 303) {
      const BuildingUnlockRule& rule = pair.second;
      if (rule.minTownHallLevel <= townHallLevel && getMaxCount(buildingType, townHallLevel) > 0) {
        defenseTypes.push_back(buildingType);
      }
    }
  }

  return defenseTypes;
}
