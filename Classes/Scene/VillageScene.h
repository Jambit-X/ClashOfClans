#ifndef __VILLAGESCENE_H__
#define __VILLAGESCENE_H__

#include "cocos2d.h"
#include "Layer/HUDLayer.h"
#include "Layer/VillageLayer.h"

class VillageScene : public cocos2d::Scene {
public:
  // 静态方法：创建场景
  static cocos2d::Scene* createScene();

  // 实例方法：场景初始化
  virtual bool init();

  // 创建宏，允许通过 VillageScene::create() 创建实例
  CREATE_FUNC(VillageScene);

private:
  VillageLayer* _villageLayer;
  HUDLayer* _hudLayer;
};

#endif // __VILLAGESCENE_H__