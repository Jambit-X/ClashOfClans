// BuildingManager.h
// 建筑管理器头文件，负责建筑精灵的创建、更新和管理

#ifndef __BUILDING_MANAGER_H__
#define __BUILDING_MANAGER_H__

#include "cocos2d.h"
#include "Sprite/BuildingSprite.h"
#include "Model/VillageData.h"
#include "../Util/GridMapUtils.h"
#include <unordered_map>

USING_NS_CC;

class BuildingSprite;
class DefenseBuildingAnimation;

class BuildingManager {
public:
  BuildingManager(Layer* parentLayer, bool isBattleScene = false);
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

  // 根据世界坐标获取建筑
  BuildingSprite* getBuildingAtWorldPos(const cocos2d::Vec2& worldPos) const;

  // 更新所有建筑
  void update(float dt);

  // 网格坐标转世界坐标
  Vec2 gridToWorld(int gridX, int gridY) const;
  Vec2 gridToWorld(const Vec2& gridPos) const;

  // 世界坐标转网格坐标
  Vec2 worldToGrid(const Vec2& worldPos) const;

  void removeBuildingSprite(int buildingId);

  // 获取防御建筑动画
  DefenseBuildingAnimation* getDefenseAnimation(int buildingId) const;

private:
  Layer* _parentLayer;
  std::unordered_map<int, BuildingSprite*> _buildings;
  bool _isBattleScene = false;

  // 防御建筑动画映射表
  std::map<int, DefenseBuildingAnimation*> _defenseAnims;

  static int calculateZOrder(int gridX, int gridY) {
    return GridMapUtils::calculateZOrder(gridX, gridY);
  }

  void createDefenseAnimation(BuildingSprite* sprite, const BuildingInstance& building);
};

#endif
