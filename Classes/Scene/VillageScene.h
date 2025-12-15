#ifndef __VILLAGESCENE_H__
#define __VILLAGESCENE_H__

#include "cocos2d.h"
#include "../Layer/VillageLayer.h"

class VillageScene : public cocos2d::Scene {
public:
  static cocos2d::Scene* createScene();
  virtual bool init();

  CREATE_FUNC(VillageScene);

private:
  VillageLayer* _gameLayer;
};

#endif