// Classes/Model/BuildingRequirements.cpp
#include "BuildingRequirements.h"
#include "cocos2d.h"

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

  // 大本营（3级）
  {
    BuildingUnlockRule rule;
    rule.buildingType = 1;
    rule.minTownHallLevel = 1;

    rule.thLevelToMaxCount[1] = 1;
    rule.thLevelToMaxCount[2] = 1;
    rule.thLevelToMaxCount[3] = 1;

    rule.buildingLevelToTH[1] = 1;
    rule.buildingLevelToTH[2] = 1;
    rule.buildingLevelToTH[3] = 2;

    addRule(rule);
  }

  // 军营（3级）
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

  // 训练营（3级）
  {
    BuildingUnlockRule rule;
    rule.buildingType = 102;
    rule.minTownHallLevel = 1;

    rule.thLevelToMaxCount[1] = 1;
    rule.thLevelToMaxCount[2] = 1;
    rule.thLevelToMaxCount[3] = 2;

    rule.buildingLevelToTH[1] = 1;
    rule.buildingLevelToTH[2] = 2;
    rule.buildingLevelToTH[3] = 3;

    addRule(rule);
  }

  // 实验室（3级）
  {
    BuildingUnlockRule rule;
    rule.buildingType = 103;
    rule.minTownHallLevel = 2;

    rule.thLevelToMaxCount[2] = 1;
    rule.thLevelToMaxCount[3] = 1;

    rule.buildingLevelToTH[1] = 2;
    rule.buildingLevelToTH[2] = 2;
    rule.buildingLevelToTH[3] = 3;

    addRule(rule);
  }

  // 金矿（3级）
  {
    BuildingUnlockRule rule;
    rule.buildingType = 202;
    rule.minTownHallLevel = 1;

    rule.thLevelToMaxCount[1] = 2;
    rule.thLevelToMaxCount[2] = 3;
    rule.thLevelToMaxCount[3] = 4;

    rule.buildingLevelToTH[1] = 1;
    rule.buildingLevelToTH[2] = 2;
    rule.buildingLevelToTH[3] = 3;

    addRule(rule);
  }

  // 圣水采集器（3级）
  {
    BuildingUnlockRule rule;
    rule.buildingType = 203;
    rule.minTownHallLevel = 1;

    rule.thLevelToMaxCount[1] = 2;
    rule.thLevelToMaxCount[2] = 3;
    rule.thLevelToMaxCount[3] = 4;

    rule.buildingLevelToTH[1] = 1;
    rule.buildingLevelToTH[2] = 2;
    rule.buildingLevelToTH[3] = 3;

    addRule(rule);
  }

  // 金币库（3级）
  {
    BuildingUnlockRule rule;
    rule.buildingType = 204;
    rule.minTownHallLevel = 1;

    rule.thLevelToMaxCount[1] = 1;
    rule.thLevelToMaxCount[2] = 2;
    rule.thLevelToMaxCount[3] = 2;

    rule.buildingLevelToTH[1] = 1;
    rule.buildingLevelToTH[2] = 2;
    rule.buildingLevelToTH[3] = 3;

    addRule(rule);
  }

  // 圣水瓶（3级）
  {
    BuildingUnlockRule rule;
    rule.buildingType = 205;
    rule.minTownHallLevel = 1;

    rule.thLevelToMaxCount[1] = 1;
    rule.thLevelToMaxCount[2] = 2;
    rule.thLevelToMaxCount[3] = 2;

    rule.buildingLevelToTH[1] = 1;
    rule.buildingLevelToTH[2] = 2;
    rule.buildingLevelToTH[3] = 3;

    addRule(rule);
  }

  // 加农炮（3级）
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

  // 箭塔（3级）
  {
    BuildingUnlockRule rule;
    rule.buildingType = 302;
    rule.minTownHallLevel = 2;

    rule.thLevelToMaxCount[2] = 1;
    rule.thLevelToMaxCount[3] = 2;

    rule.buildingLevelToTH[1] = 2;
    rule.buildingLevelToTH[2] = 2;
    rule.buildingLevelToTH[3] = 3;

    addRule(rule);
  }

  // 城墙（3级）
  {
    BuildingUnlockRule rule;
    rule.buildingType = 303;
    rule.minTownHallLevel = 2;

    rule.thLevelToMaxCount[2] = 25;
    rule.thLevelToMaxCount[3] = 50;

    rule.buildingLevelToTH[1] = 2;
    rule.buildingLevelToTH[2] = 2;
    rule.buildingLevelToTH[3] = 3;

    addRule(rule);
  }

  // 炸弹（3级）
  {
    BuildingUnlockRule rule;
    rule.buildingType = 401;
    rule.minTownHallLevel = 2;

    rule.thLevelToMaxCount[2] = 2;
    rule.thLevelToMaxCount[3] = 3;

    rule.buildingLevelToTH[1] = 2;
    rule.buildingLevelToTH[2] = 2;
    rule.buildingLevelToTH[3] = 3;

    addRule(rule);
  }

  // 弹簧陷阱（3级）
  {
    BuildingUnlockRule rule;
    rule.buildingType = 402;
    rule.minTownHallLevel = 2;

    rule.thLevelToMaxCount[2] = 1;
    rule.thLevelToMaxCount[3] = 2;

    rule.buildingLevelToTH[1] = 2;
    rule.buildingLevelToTH[2] = 2;
    rule.buildingLevelToTH[3] = 3;

    addRule(rule);
  }

  // 巨型炸弹（3级）
  {
    BuildingUnlockRule rule;
    rule.buildingType = 404;
    rule.minTownHallLevel = 3;

    rule.thLevelToMaxCount[3] = 1;

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

  if (currentTHLevel < rule.minTownHallLevel) {
    return false;
  }

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

  if (targetLevel > 3) {
    return false;
  }

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
    return "未知建筑类型";
  }

  const BuildingUnlockRule& rule = it->second;

  if (currentLevel == 0) {
    if (currentTHLevel < rule.minTownHallLevel) {
      return "需要" + std::to_string(rule.minTownHallLevel) + "级大本营解锁";
    }

    int maxCount = getMaxCount(buildingType, currentTHLevel);
    if (currentCount >= maxCount) {
      return "已达数量上限（最多" + std::to_string(maxCount) + "个）";
    }
  } else {
    int targetLevel = currentLevel + 1;

    if (targetLevel > 3) {
      return "已达最高等级（3级）";
    }

    auto upgradeIt = rule.buildingLevelToTH.find(targetLevel);
    if (upgradeIt == rule.buildingLevelToTH.end()) {
      return "已达最高等级";
    }

    int requiredTHLevel = upgradeIt->second;
    if (currentTHLevel < requiredTHLevel) {
      return "需要" + std::to_string(requiredTHLevel) + "级大本营";
    }
  }

  return "";
}