#ifndef __INPUT_CONTROLLER_H__
#define __INPUT_CONTROLLER_H__

#include "cocos2d.h"
#include <functional>

// 输入状态枚举
enum class InputState {
  MAP_DRAG,           // 默认状态：地图拖动
  BUILDING_SELECTED,  // 建筑选中模式
  SHOP_MODE          // 商店模式
};

// 点击目标类型
enum class TapTarget {
  NONE,      // 无目标（地图空白）
  BUILDING,  // 建筑
  SHOP       // 商店图标
};

class InputController {
public:
  // 构造函数：传入需要控制的 VillageLayer
  InputController(cocos2d::Layer* villageLayer);
  ~InputController();

  // 初始化输入监听
  void setupInputListeners();

  // 清理输入监听
  void cleanup();

  // 获取当前输入状态
  InputState getCurrentState() const { return _currentState; }

  // 设置状态切换回调
  void setOnStateChangedCallback(std::function<void(InputState, InputState)> callback) {
    _onStateChanged = callback;
  }

  // 设置点击检测回调
  void setOnTapCallback(std::function<TapTarget(const cocos2d::Vec2&)> callback) {
    _onTapDetection = callback;
  }

  // 设置建筑选中回调
  void setOnBuildingSelectedCallback(std::function<void(const cocos2d::Vec2&)> callback) {
    _onBuildingSelected = callback;
  }

  // 设置商店打开回调
  void setOnShopOpenedCallback(std::function<void(const cocos2d::Vec2&)> callback) {
    _onShopOpened = callback;
  }

private:
  // ========== 基本属性 ==========
  cocos2d::Layer* _villageLayer;  // 村庄地图层
  InputState _currentState;       // 当前输入状态

  // 缩放参数
  float _minScale;
  static const float MAX_SCALE;
  static const float ZOOM_SPEED;
  float _currentScale;

  // 拖动状态
  cocos2d::Vec2 _touchStartPos;   // 触摸开始位置
  cocos2d::Vec2 _layerStartPos;   // Layer 开始位置
  bool _isDragging;               // 是否正在拖动

  // 事件监听器
  cocos2d::EventListenerTouchOneByOne* _touchListener;
  cocos2d::EventListenerMouse* _mouseListener;

  // 回调函数
  std::function<void(InputState, InputState)> _onStateChanged;  // 状态改变回调（旧状态，新状态）
  std::function<TapTarget(const cocos2d::Vec2&)> _onTapDetection;  // 点击检测回调
  std::function<void(const cocos2d::Vec2&)> _onBuildingSelected;   // 建筑选中回调
  std::function<void(const cocos2d::Vec2&)> _onShopOpened;         // 商店打开回调

  // ========== 状态管理 ==========
  void changeState(InputState newState);

  // ========== 初始化相关 ==========
  void calculateMinScale();
  void initializeMapTransform();

  // ========== 触摸事件处理 ==========
  void setupTouchHandling();
  bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
  void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);
  void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);

  void storeTouchStartState(cocos2d::Touch* touch);
  void handleMapDragging(cocos2d::Touch* touch);
  void handleTap(const cocos2d::Vec2& tapPosition);

  // ========== 鼠标事件处理（缩放） ==========
  void setupMouseHandling();
  void onMouseScroll(cocos2d::Event* event);

  float calculateNewScale(float scrollDelta);
  cocos2d::Vec2 getAdjustedMousePosition(cocos2d::EventMouse* mouseEvent);
  void applyZoomAroundPoint(const cocos2d::Vec2& zoomPoint, float newScale);

  // ========== 辅助方法 ==========
  cocos2d::Vec2 clampMapPosition(const cocos2d::Vec2& position);
  bool isTapGesture(const cocos2d::Vec2& startPos, const cocos2d::Vec2& endPos);
};

#endif
