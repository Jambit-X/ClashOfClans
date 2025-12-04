#ifndef __VILLAGE_LAYER_H__
#define __VILLAGE_LAYER_H__

#include "cocos2d.h"

class VillageLayer : public cocos2d::Layer {
private:
  // 缩放常量
  static const float MIN_SCALE;
  static const float MAX_SCALE;
  static const float ZOOM_SPEED;

  // 地图精灵
  cocos2d::Sprite* _mapSprite;

  // 缩放状态
  float _currentScale;

  // 拖动状态
  cocos2d::Vec2 _touchStartPos;
  cocos2d::Vec2 _layerStartPos;

public:
  virtual bool init() override;
  CREATE_FUNC(VillageLayer);

private:
  // ========== 初始化方法 ==========
  void initializeBasicProperties();
  void setupEventHandling();

  // ========== 辅助方法 ==========
  cocos2d::Sprite* createMapSprite();
  cocos2d::Vec2 calculateCenterPosition();

  // ========== 触摸事件（拖动） ==========
  void setupTouchHandling();
  bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
  void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);
  void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);

  void storeTouchStartState(cocos2d::Touch* touch);
  void handleMapDragging(cocos2d::Touch* touch);
  cocos2d::Vec2 clampMapPosition(const cocos2d::Vec2& position);

  // ========== 鼠标事件（缩放） ==========
  void setupMouseHandling();
  void onMouseScroll(cocos2d::Event* event);

  float calculateNewScale(float scrollDelta);
  cocos2d::Vec2 getAdjustedMousePosition(cocos2d::EventMouse* mouseEvent);
  void applyZoomAroundPoint(const cocos2d::Vec2& screenPoint, float newScale);
};

#endif