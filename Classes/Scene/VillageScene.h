#ifndef __VILLAGESCENE_H__
#define __VILLAGESCENE_H__

#include "cocos2d.h"
#include "../Layer/VillageLayer.h"

class VillageScene : public cocos2d::Scene {
public:
  static cocos2d::Scene* createScene();
  virtual bool init();
  
  // ✅ 新增：场景生命周期管理
  virtual void onEnter() override;
  virtual void onExit() override;

  CREATE_FUNC(VillageScene);

private:
  VillageLayer* _gameLayer;
  
  // ✅ 新增：音乐ID
  int _backgroundMusicID = -1;
};

#endif
