#ifndef __VILLAGE_LAYER_H__
#define __VILLAGE_LAYER_H__

#include "cocos2d.h"

class InputController;  // 前向声明

class VillageLayer : public cocos2d::Layer {
private:
  // 地图精灵
  cocos2d::Sprite* _mapSprite;

  // 输入控制器
  InputController* _inputController;

public:
  virtual bool init() override;
  CREATE_FUNC(VillageLayer);

  // 获取输入控制器
  InputController* getInputController() const { return _inputController; }

  // 清理
  virtual void cleanup() override;

private:
  // ========== 初始化方法 ==========
  void initializeBasicProperties();

  // ========== 辅助方法 ==========
  cocos2d::Sprite* createMapSprite();
};

#endif