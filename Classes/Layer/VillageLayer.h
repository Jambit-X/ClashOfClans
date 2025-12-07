#ifndef __VILLAGE_LAYER_H__
#define __VILLAGE_LAYER_H__

#include "cocos2d.h"

// 前向声明
class MoveMapController;
class BuildingManager;
class MoveBuildingController;
class BuildingSprite;

/**
 * @brief 村庄场景的主要显示层
 */
class VillageLayer : public cocos2d::Layer {
private:
  // 地图精灵
  cocos2d::Sprite* _mapSprite;
  // 地图移动控制器
  MoveMapController* _inputController;
  // 建筑管理器
  BuildingManager* _buildingManager;
  // 建筑移动控制器
  MoveBuildingController* _moveBuildingController;


public:
  virtual bool init() override;
  CREATE_FUNC(VillageLayer);

  // 获取地图移动控制器
  MoveMapController* getInputController() const { return _inputController; }
  // 获取建筑管理器
  BuildingManager* getBuildingManager() const { return _buildingManager; }
  // 获取建筑移动控制器
  MoveBuildingController* getMoveBuildingController() const { return _moveBuildingController; }
  // 商店购买建筑后的回调
  void onBuildingPurchased(int buildingId);

  // 清理
  virtual void cleanup() override;

private:
  // ========== 初始化方法 ==========
  void initializeBasicProperties();
  void setupInputCallbacks();

  // ========== 辅助方法 ==========
  /** 根据屏幕坐标获取建筑（复用逻辑，避免重复代码）*/
  BuildingSprite* getBuildingAtScreenPos(const cocos2d::Vec2& screenPos);
  
  /** 创建地图精灵 */
  cocos2d::Sprite* createMapSprite();
};

#endif