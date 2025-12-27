// BuildingConfig.h
// 建筑配置管理器，提供所有建筑的静态配置数据

#ifndef __BUILDING_CONFIG_H__
#define __BUILDING_CONFIG_H__

#include "cocos2d.h"
#include <string>
#include <unordered_map>

// 建筑配置数据
struct BuildingConfigData {
  // 基本信息
  int type;                       // 建筑类型ID
  std::string name;               // 建筑名称
  std::string category;           // 建筑分类（军队/资源/防御/陷阱）

  // 视觉表现
  std::string spritePathTemplate; // 精灵路径模板
  int gridWidth;                  // 占用网格宽度
  int gridHeight;                 // 占用网格高度
  cocos2d::Vec2 anchorOffset;     // 锚点偏移

  // 游戏属性
  int maxLevel;                   // 最大等级
  int initialCost;                // 初始建造费用
  std::string costType;           // 费用类型（金币/圣水/宝石）
  int buildTimeSeconds;           // 建造时间（秒）

  // 扩展属性（可选）
  int hitPoints;                  // 生命值（防御建筑用）
  int damagePerSecond;            // 每秒伤害（防御建筑用）
  int attackRange;                // 攻击范围（防御建筑用）
  int resourceCapacity;           // 资源容量（资源建筑用）
  int productionRate;             // 生产速率（资源建筑用，单位/小时）
  float attackSpeed = 1.0f;       // 攻击速度（秒/次）
};

// 建筑配置管理器（单例）
class BuildingConfig {
public:
  static BuildingConfig* getInstance();
  static void destroyInstance();

  // 根据建筑类型获取配置
  const BuildingConfigData* getConfig(int buildingType) const;

  // 获取建筑精灵路径（根据类型和等级）
  std::string getSpritePath(int buildingType, int level) const;

  // 获取建筑升级费用
  int getUpgradeCost(int buildingType, int currentLevel) const;

  // 检查建筑是否可以升级
  bool canUpgrade(int buildingType, int currentLevel) const;
  
  // 根据等级获取存储容量
  int getStorageCapacityByLevel(int buildingType, int level) const;

private:
  BuildingConfig();
  ~BuildingConfig();

  void initConfigs();  // 初始化所有建筑配置

  static BuildingConfig* _instance;
  std::unordered_map<int, BuildingConfigData> _configs;  // <buildingType, config>
};

#endif // __BUILDING_CONFIG_H__
