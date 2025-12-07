#ifndef __VILLAGE_LAYER_H__
#define __VILLAGE_LAYER_H__

#include "cocos2d.h"

class InputController;  // 前向声明
class BuildingManager;  // 前置声明

class VillageLayer : public cocos2d::Layer {
private:
  // 地图精灵
  cocos2d::Sprite* _mapSprite;
  // 输入控制器
  InputController* _inputController;
  // 建筑管理
  BuildingManager* _buildingManager;  


public:
  virtual bool init() override;
  CREATE_FUNC(VillageLayer);

  // 获取输入控制器
  InputController* getInputController() const { return _inputController; }
  // 获取建筑管理器
  BuildingManager* getBuildingManager() const { return _buildingManager; }
  // 商店购买建筑后的回调
  void onBuildingPurchased(int buildingId);

  // 清理
  virtual void cleanup() override;

private:
  // ========== 初始化方法 ==========
  void initializeBasicProperties();

  // ========== 辅助方法 ==========
  cocos2d::Sprite* createMapSprite();
};

#endif