#ifndef __BUILDING_CONFIG_H__
#define __BUILDING_CONFIG_H__

#include "cocos2d.h"
#include <string>
#include <unordered_map>

// 建筑配置数据
struct BuildingConfigData {
  // ========== 基本信息 ==========
  int type;                       // 建筑类型ID（唯一标识，如101=兵营）
  std::string name;               // 建筑名称（中文）
  std::string category;           // 建筑分类（"军队"/"资源"/"防御"/"陷阱"）

  // ========== 视觉表现 ==========
  std::string spritePathTemplate; // 精灵路径模板（如 "UI/Shop/military/Barracks{level}.png"）
  int gridWidth;                  // 占用网格宽度（单位：格子）
  int gridHeight;                 // 占用网格高度（单位：格子）
  cocos2d::Vec2 anchorOffset;     // 锚点偏移（用于调整显示位置）

  // ========== 游戏属性 ==========
  int maxLevel;                   // 最大等级
  int initialCost;                // 初始建造费用
  std::string costType;           // 费用类型（"金币"/"圣水"/"宝石"）
  int buildTimeSeconds;           // 建造时间（秒）

  // ========== 扩展属性（可选）==========
  int hitPoints;                  // 生命值（防御建筑用）
  int damagePerSecond;            // 每秒伤害（防御建筑用）
  int attackRange;                // 攻击范围（防御建筑用）
  int resourceCapacity;           // 资源容量（资源建筑用）
  int productionRate;             // 生产速率（资源建筑用，单位/小时）
};

/**
 * @brief 建筑配置管理器（单例）
 * 负责加载和提供所有建筑的静态配置
 */
class BuildingConfig {
public:
  static BuildingConfig* getInstance();
  static void destroyInstance();

  // ========== 查询接口 ==========

  /**
   * @brief 根据建筑类型获取配置
   * @param buildingType 建筑类型ID
   * @return 返回配置指针，如果不存在返回 nullptr
   */
  const BuildingConfigData* getConfig(int buildingType) const;

  /**
   * @brief 获取建筑精灵路径（根据类型和等级）
   * @param buildingType 建筑类型ID
   * @param level 建筑等级
   * @return 返回完整的精灵路径
   * @example getSpritePath(101, 2) -> "UI/Shop/military/Barracks2.png"
   */
  std::string getSpritePath(int buildingType, int level) const;

  /**
   * @brief 获取建筑升级费用
   * @param buildingType 建筑类型ID
   * @param currentLevel 当前等级
   * @return 返回升级到下一级的费用
   */
  int getUpgradeCost(int buildingType, int currentLevel) const;

  /**
   * @brief 检查建筑是否可以升级
   * @param buildingType 建筑类型ID
   * @param currentLevel 当前等级
   * @return 是否可以升级
   */
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