// BuildingUpgradeManager.h
// 建筑升级管理器头文件，处理建筑升级逻辑

#pragma once
#include "cocos2d.h"
#include <vector>

class BuildingUpgradeManager {
public:
  static BuildingUpgradeManager* getInstance();
  static void destroyInstance();

  // 更新所有升级中的建筑
  void update(float dt);

  // 检查并完成所有到期的升级
  void checkFinishedUpgrades();

  // 检查指定建筑是否可以升级
  bool canUpgrade(int buildingId);

  std::string getUpgradeFailReason(int buildingId) const;

private:
  BuildingUpgradeManager();
  ~BuildingUpgradeManager();

  static BuildingUpgradeManager* _instance;
};
