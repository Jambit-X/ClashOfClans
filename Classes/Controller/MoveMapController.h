#ifndef __MOVE_MAP_CONTROLLER_H__
#define __MOVE_MAP_CONTROLLER_H__

#include "cocos2d.h"
#include <functional>
#include <map>

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

class MoveMapController {
public:
  // 构造函数，需要传入 VillageLayer
  MoveMapController(cocos2d::Layer* villageLayer);
  ~MoveMapController();

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

  // 设置建筑点击回调（用于触发移动）
  void setOnBuildingClickedCallback(std::function<void(int)> callback) {
    _onBuildingClicked = callback;
  }

private:
  // ========== 核心属性 ========= =
  cocos2d::Layer* _villageLayer;  // 村庄地图层
  InputState _currentState;       // 当前输入状态

  // 缩放参数
  float _minScale;
  static const float MAX_SCALE;
  static const float ZOOM_SPEED;
  float _currentScale;

  // 拖动状态
  cocos2d::Vec2 _touchStartPos;   // 触摸开始位置
  cocos2d::Vec2 _layerStartPos;   // Layer 起始位置
  bool _isDragging;               // 是否正在拖动

  // 事件监听器
  cocos2d::EventListenerTouchAllAtOnce* _multiTouchListener;  // 多点触控监听器
  cocos2d::EventListenerMouse* _mouseListener;

  // 多点触控状态
  bool _isPinching;                // 是否正在双指缩放
  float _initialPinchDistance;     // 初始双指距离
  float _initialPinchScale;        // 初始缩放比例
  cocos2d::Vec2 _pinchCenter;      // 缩放中心点
  std::map<int, cocos2d::Touch*> _activeTouches;  // 追踪所有活跃触点

  // 回调函数
  std::function<void(InputState, InputState)> _onStateChanged;  // 状态改变回调（旧状态，新状态）
  std::function<TapTarget(const cocos2d::Vec2&)> _onTapDetection;  // 点击检测回调
  std::function<void(const cocos2d::Vec2&)> _onBuildingSelected;   // 建筑选中回调
  std::function<void(const cocos2d::Vec2&)> _onShopOpened;         // 商店打开回调
  std::function<void(int)> _onBuildingClicked;                     // 建筑点击回调（传入建筑ID）

  // ========== 状态管理 ========= =
  void changeState(InputState newState);

  // ========== 初始化方法 ========= =
  void calculateMinScale();
  void initializeMapTransform();

  // ========== 触摸事件处理 ========= =
  void setupTouchHandling();
  void onTouchesBegan(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event* event);
  void onTouchesMoved(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event* event);
  void onTouchesEnded(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event* event);

  // 捏合缩放辅助方法
  float getTouchDistance(const std::vector<cocos2d::Touch*>& touches);
  cocos2d::Vec2 getTouchCenter(const std::vector<cocos2d::Touch*>& touches);

  void storeTouchStartState(cocos2d::Touch* touch);
  void handleMapDragging(cocos2d::Touch* touch);
  void handleTap(const cocos2d::Vec2& tapPosition);

  // ========== 鼠标事件处理（缩放）==========
  void setupMouseHandling();
  void onMouseScroll(cocos2d::Event* event);

  float calculateNewScale(float scrollDelta);
  cocos2d::Vec2 getAdjustedMousePosition(cocos2d::EventMouse* mouseEvent);
  void applyZoomAroundPoint(const cocos2d::Vec2& zoomPoint, float newScale);

  // ========== 辅助方法 ========= =
  cocos2d::Vec2 clampMapPosition(const cocos2d::Vec2& position);
  bool isTapGesture(const cocos2d::Vec2& startPos, const cocos2d::Vec2& endPos);
};

#endif
