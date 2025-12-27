// VillageScene.h
// 村庄场景，游戏主场景

#ifndef __VILLAGESCENE_H__
#define __VILLAGESCENE_H__

#include "cocos2d.h"
#include "../Layer/VillageLayer.h"

class VillageScene : public cocos2d::Scene {
public:
  static cocos2d::Scene* createScene();
  virtual bool init();
  
  // 场景生命周期管理
  virtual void onEnter() override;
  virtual void onExit() override;

  CREATE_FUNC(VillageScene);

private:
  VillageLayer* _gameLayer;
  
  // 背景音乐ID
  int _backgroundMusicID = -1;
};

#endif
