// Classes/Manager/BuildingSpeedupManager.h
#ifndef BUILDING_SPEEDUP_MANAGER_H
#define BUILDING_SPEEDUP_MANAGER_H

#include "cocos2d.h"
#include <string>

class BuildingSpeedupManager {
public:
  static BuildingSpeedupManager* getInstance();
  static void destroyInstance();

  // 检查是否可以加速（有宝石且建筑正在建造）
  bool canSpeedup(int buildingId) const;

  // 执行加速（消耗1颗宝石，立即完成）
  bool speedupBuilding(int buildingId);

  // 获取加速失败的原因
  std::string getSpeedupFailReason(int buildingId) const;

private:
  BuildingSpeedupManager();
  ~BuildingSpeedupManager();

  static BuildingSpeedupManager* instance;
};

#endif