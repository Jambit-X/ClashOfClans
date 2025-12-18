#ifndef __BUILDING_MANAGER_H__
#define __BUILDING_MANAGER_H__

#include "cocos2d.h"
#include "Sprite/BuildingSprite.h"
#include "Model/VillageData.h"
#include "../Util/GridMapUtils.h"
#include <unordered_map>

USING_NS_CC;

class BuildingSprite;
class DefenseBuildingAnimation;  // 前向声明

// 建筑管理器（负责建筑精灵的创建、更新、删除）
class BuildingManager {
public:
  BuildingManager(Layer* parentLayer, bool isBattleScene = false);  // 添加场景标识参数
  ~BuildingManager();

  // 从数据创建所有建筑
  void loadBuildingsFromData();
  
  // 从战斗地图数据创建建筑
  void loadFromBattleMapData();

  // 添加建筑精灵
  BuildingSprite* addBuilding(const BuildingInstance& building);

  // 移除建筑精灵
  void removeBuilding(int buildingId);

  // 更新建筑状态
  void updateBuilding(int buildingId, const BuildingInstance& building);

  // 根据ID获取建筑精灵
  BuildingSprite* getBuildingSprite(int buildingId) const;

  // 根据网格坐标获取建筑
  BuildingSprite* getBuildingAtGrid(int gridX, int gridY) const;

  // 根据世界坐标获取建筑（用于点击检测）
  BuildingSprite* getBuildingAtWorldPos(const cocos2d::Vec2& worldPos) const;

  // 更新所有建筑（用于定时检查建造完成）
  void update(float dt);

  // 网格坐标转世界坐标（使用 GridMapUtils）
  Vec2 gridToWorld(int gridX, int gridY) const;
  Vec2 gridToWorld(const Vec2& gridPos) const;

  // 世界坐标转网格坐标（使用 GridMapUtils）
  Vec2 worldToGrid(const Vec2& worldPos) const;
  // 在 BuildingManager 类中添加
  void removeBuildingSprite(int buildingId);

  // 获取防御建筑动画
  DefenseBuildingAnimation* getDefenseAnimation(int buildingId) const;

private:
  Layer* _parentLayer;                                      // 父层
  std::unordered_map<int, BuildingSprite*> _buildings;    // 建筑映射表 <buildingId, sprite>
  bool _isBattleScene = false;  // 是否为战斗场景

  // 防御建筑动画映射表
  std::map<int, DefenseBuildingAnimation*> _defenseAnims;

  // 统一的 Z-Order 计算函数
  static int calculateZOrder(int gridX, int gridY) {
    return GridMapUtils::calculateZOrder(gridX, gridY);
  }

  // 创建防御建筑动画
  void createDefenseAnimation(BuildingSprite* sprite, const BuildingInstance& building);
};

#endif // __BUILDING_MANAGER_H__
