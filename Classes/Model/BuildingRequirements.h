// Classes/Model/BuildingRequirements.h
#ifndef BUILDING_REQUIREMENTS_H
#define BUILDING_REQUIREMENTS_H

#include <map>
#include <vector>
#include <string>

// 建筑解锁规则数据结构
struct BuildingUnlockRule {
  int buildingType;                       // 建筑类型ID
  int minTownHallLevel;                   // 最低解锁大本营等级
  std::map<int, int> thLevelToMaxCount;   // 大本营等级 -> 最大数量
  std::map<int, int> buildingLevelToTH;   // 建筑等级 -> 需要的大本营等级

  BuildingUnlockRule()
    : buildingType(0)
    , minTownHallLevel(1) {}
};

// 建筑需求管理器（单例模式）
class BuildingRequirements {
public:
  static BuildingRequirements* getInstance();
  static void destroyInstance();

  // ========== 核心查询接口 ==========

  // 检查是否可以购买建筑（检查解锁等级和数量限制）
  bool canPurchase(int buildingType, int currentTHLevel, int currentCount) const;

  // 检查是否可以升级建筑（检查大本营等级限制）
  bool canUpgrade(int buildingType, int currentBuildingLevel, int currentTHLevel) const;

  // 获取建筑在指定大本营等级下的最大数量
  int getMaxCount(int buildingType, int townHallLevel) const;

  // 获取建筑升级到目标等级所需的大本营等级
  int getRequiredTHLevel(int buildingType, int targetBuildingLevel) const;

  // 获取建筑的最小解锁大本营等级
  int getMinTHLevel(int buildingType) const;

  // 获取限制失败的原因（用于UI提示）
  std::string getRestrictionReason(int buildingType, int currentLevel, int currentTHLevel, int currentCount) const;

private:
  BuildingRequirements();
  ~BuildingRequirements();

  void initializeRules();                 // 初始化所有建筑规则
  void addRule(const BuildingUnlockRule& rule);  // 添加单条规则

  static BuildingRequirements* instance;
  std::map<int, BuildingUnlockRule> rules;  // 建筑类型 -> 解锁规则
};

#endif