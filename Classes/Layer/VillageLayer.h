#ifndef __VILLAGELAYER_H__
#define __VILLAGELAYER_H__

#include "cocos2d.h"

class VillageLayer : public cocos2d::Layer {
public:
  virtual bool init();
  CREATE_FUNC(VillageLayer);

private:
  // 触摸处理函数
  virtual bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
  virtual void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);
  virtual void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);

  cocos2d::Vec2 _touchStartPos; // 记录触摸开始位置
  cocos2d::Vec2 _layerStartPos; // 记录 Layer 自身的起始位置

  void setupTouchHandling();
  void setMapBoundary();
  // 新增：鼠标事件监听设置
  void setupMouseHandling();
  // 新增：鼠标滚轮回调函数
  void onMouseScroll(cocos2d::Event* event);

  float _currentScale; // 记录当前的缩放比例
  const float MIN_SCALE = 0.5f;
  const float MAX_SCALE = 2.0f;
  const float ZOOM_SPEED = 0.1f;
};

#endif // __VILLAGELAYER_H__