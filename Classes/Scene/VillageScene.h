#ifndef __VILLAGESCENE_H__
#define __VILLAGESCENE_H__ // 注意宏定义的名称最好统一

#include "cocos2d.h"
#include "../Layer/HUDLayer.h"    // 注意路径可能需要调整
#include "../Layer/VillageLayer.h"
#include "../Controller/ResourceController.h" // 引入头文件
#include "../Controller/UpgradeController.h"  // 引入头文件

class VillageScene : public cocos2d::Scene {
public:
  static cocos2d::Scene* createScene();
  virtual bool init();

  // 核心循环：每帧调用
  virtual void update(float dt);

  CREATE_FUNC(VillageScene);

private:
  VillageLayer* _gameLayer;
  HUDLayer* _hudLayer;

  // 控制器实例
  ResourceController* _resourceController;
  UpgradeController* _upgradeController;
};

#endif