#include "InputController.h"
#include "../proj.win32/Constants.h"
#include <iostream>

USING_NS_CC;

// 静态常量定义
const float InputController::MAX_SCALE = 3.0f;
const float InputController::ZOOM_SPEED = 0.05f;

// Tap 判定阈值（像素）
const float TAP_THRESHOLD = 10.0f;

InputController::InputController(Layer* villageLayer)
  : _villageLayer(villageLayer)
  , _currentState(InputState::MAP_DRAG)
  , _minScale(1.0f)
  , _currentScale(1.0f)
  , _isDragging(false)
  , _touchListener(nullptr)
  , _mouseListener(nullptr)
  , _onStateChanged(nullptr)
  , _onTapDetection(nullptr)
  , _onBuildingSelected(nullptr)
  , _onShopOpened(nullptr)
{
  calculateMinScale();
  _currentScale = _minScale;
}

InputController::~InputController() {
  cleanup();
}

#pragma region 初始化和清理
void InputController::setupInputListeners() {
  // 初始化地图的缩放和位置
  initializeMapTransform();
  
  // 设置输入监听
  setupTouchHandling();
  setupMouseHandling();
  
  CCLOG("InputController: Input listeners initialized");
  CCLOG("  Initial state: MAP_DRAG");
  CCLOG("  Scale range: %.3f - %.3f", _minScale, MAX_SCALE);
}

void InputController::cleanup() {
  if (_touchListener) {
    Director::getInstance()->getEventDispatcher()->removeEventListener(_touchListener);
    _touchListener = nullptr;
  }
  
  if (_mouseListener) {
    Director::getInstance()->getEventDispatcher()->removeEventListener(_mouseListener);
    _mouseListener = nullptr;
  }
  
  CCLOG("InputController: Cleaned up");
}

void InputController::calculateMinScale() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  auto mapSize = _villageLayer->getContentSize();

  float scaleX = visibleSize.width / mapSize.width;
  float scaleY = visibleSize.height / mapSize.height;
  _minScale = std::max(scaleX, scaleY);

  CCLOG("InputController: Calculated MIN_SCALE: %.3f", _minScale);
}

void InputController::initializeMapTransform() {
  auto mapSize = _villageLayer->getContentSize();
  auto visibleSize = Director::getInstance()->getVisibleSize();

  // 设置初始缩放为最小缩放
  _villageLayer->setScale(_currentScale);

  // 将地图中心放在屏幕中心
  float initialX = (visibleSize.width - mapSize.width * _currentScale) / 2;
  float initialY = (visibleSize.height - mapSize.height * _currentScale) / 2;
  _villageLayer->setPosition(initialX, initialY);

  CCLOG("InputController: Map transform initialized");
  CCLOG("  Initial scale: %.3f", _currentScale);
  CCLOG("  Initial position: (%.2f, %.2f)", initialX, initialY);
}
#pragma endregion

#pragma region 状态管理
void InputController::changeState(InputState newState) {
  if (_currentState == newState) return;

  InputState oldState = _currentState;
  _currentState = newState;

  CCLOG("InputController: State changed from %d to %d", (int)oldState, (int)newState);

  // 触发状态改变回调
  if (_onStateChanged) {
    _onStateChanged(oldState, newState);
  }
}
#pragma endregion

#pragma region 触摸事件处理
void InputController::setupTouchHandling() {
  _touchListener = EventListenerTouchOneByOne::create();
  _touchListener->setSwallowTouches(true);

  _touchListener->onTouchBegan = CC_CALLBACK_2(InputController::onTouchBegan, this);
  _touchListener->onTouchMoved = CC_CALLBACK_2(InputController::onTouchMoved, this);
  _touchListener->onTouchEnded = CC_CALLBACK_2(InputController::onTouchEnded, this);

  Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(
    _touchListener, _villageLayer);
}

bool InputController::onTouchBegan(Touch* touch, Event* event) {
  // 只在 MAP_DRAG 状态处理输入
  if (_currentState != InputState::MAP_DRAG) {
    return false;
  }

  storeTouchStartState(touch);
  _isDragging = false;
  return true;
}

void InputController::onTouchMoved(Touch* touch, Event* event) {
  if (_currentState != InputState::MAP_DRAG) {
    return;
  }

  _isDragging = true;
  handleMapDragging(touch);
}

void InputController::onTouchEnded(Touch* touch, Event* event) {
  if (_currentState != InputState::MAP_DRAG) {
    return;
  }

  Vec2 endPos = touch->getLocation();

  // 判断是 Tap 还是 Drag
  if (!_isDragging && isTapGesture(_touchStartPos, endPos)) {
    handleTap(endPos);
  }

  _isDragging = false;
}

void InputController::storeTouchStartState(Touch* touch) {
  _touchStartPos = touch->getLocation();
  _layerStartPos = _villageLayer->getPosition();
  
  CCLOG("Touch began at (%.2f, %.2f)", _touchStartPos.x, _touchStartPos.y);
}

void InputController::handleMapDragging(Touch* touch) {
  Vec2 currentTouchPos = touch->getLocation();
  Vec2 delta = currentTouchPos - _touchStartPos;
  Vec2 newPos = _layerStartPos + delta;

  newPos = clampMapPosition(newPos);
  _villageLayer->setPosition(newPos);
}

void InputController::handleTap(const Vec2& tapPosition) {
  CCLOG("Tap detected at (%.2f, %.2f)", tapPosition.x, tapPosition.y);

  // 如果有点击检测回调，调用它
  TapTarget target = TapTarget::NONE;
  if (_onTapDetection) {
    target = _onTapDetection(tapPosition);
  }

  // 根据点击目标执行相应操作
  switch (target) {
    case TapTarget::BUILDING:
      CCLOG("  -> Target: BUILDING");
      changeState(InputState::BUILDING_SELECTED);
      if (_onBuildingSelected) {
        _onBuildingSelected(tapPosition);
      }
      break;

    case TapTarget::SHOP:
      CCLOG("  -> Target: SHOP");
      changeState(InputState::SHOP_MODE);
      if (_onShopOpened) {
        _onShopOpened(tapPosition);
      }
      break;

    case TapTarget::NONE:
      CCLOG("  -> Target: NONE (map blank)");
      // 地图空白，无响应
      break;
  }
}

bool InputController::isTapGesture(const Vec2& startPos, const Vec2& endPos) {
  float distance = startPos.distance(endPos);
  return distance < TAP_THRESHOLD;
}

Vec2 InputController::clampMapPosition(const Vec2& position) {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  auto mapSize = _villageLayer->getContentSize() * _villageLayer->getScale();

  float minX, maxX, minY, maxY;

  if (mapSize.width <= visibleSize.width) {
    float centerX = (visibleSize.width - mapSize.width) / 2;
    minX = maxX = centerX;
  } else {
    minX = visibleSize.width - mapSize.width;
    maxX = 0;
  }

  if (mapSize.height <= visibleSize.height) {
    float centerY = (visibleSize.height - mapSize.height) / 2;
    minY = maxY = centerY;
  } else {
    minY = visibleSize.height - mapSize.height;
    maxY = 0;
  }

  return Vec2(
    clampf(position.x, minX, maxX),
    clampf(position.y, minY, maxY)
  );
}
#pragma endregion

#pragma region 鼠标事件处理（缩放）
void InputController::setupMouseHandling() {
  _mouseListener = EventListenerMouse::create();
  _mouseListener->onMouseScroll = CC_CALLBACK_1(InputController::onMouseScroll, this);
  
  Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(
    _mouseListener, _villageLayer);
}

void InputController::onMouseScroll(Event* event) {
  // 只在 MAP_DRAG 状态处理缩放
  if (_currentState != InputState::MAP_DRAG) {
    return;
  }

  EventMouse* mouseEvent = static_cast<EventMouse*>(event);
  float scrollY = mouseEvent->getScrollY();

  float newScale = calculateNewScale(scrollY);
  if (newScale == _currentScale) return;

  Vec2 mousePos = getAdjustedMousePosition(mouseEvent);
  applyZoomAroundPoint(mousePos, newScale);
}

float InputController::calculateNewScale(float scrollDelta) {
  // 反转滚轮方向：向上滚动缩小，向下滚动放大
  float scaleFactor = 1.0f + (-scrollDelta * ZOOM_SPEED);
  float newScale = _currentScale * scaleFactor;

  // 限制缩放范围
  newScale = clampf(newScale, _minScale, MAX_SCALE);

  // 避免微小抖动
  if (abs(newScale - _currentScale) < 0.01f) {
    return _currentScale;
  }

  return newScale;
}

Vec2 InputController::getAdjustedMousePosition(EventMouse* mouseEvent) {
  Vec2 mousePosInView = mouseEvent->getLocationInView();
  auto winSize = Director::getInstance()->getWinSize();

  // 从左上角原点转换为左下角原点
  mousePosInView.y = winSize.height - mousePosInView.y;
  return mousePosInView;
}

void InputController::applyZoomAroundPoint(const Vec2& zoomPoint, float newScale) {
  Vec2 oldPosition = _villageLayer->getPosition();
  float oldScale = _currentScale;

  // 计算缩放中心点在 Layer 中的相对位置
  Vec2 localPoint = zoomPoint - oldPosition;

  // 应用新的缩放
  _villageLayer->setScale(newScale);
  _currentScale = newScale;

  // 计算缩放后需要调整的位置
  float scaleRatio = newScale / oldScale;
  Vec2 newLocalPoint = localPoint * scaleRatio;
  Vec2 offset = localPoint - newLocalPoint;
  Vec2 newPos = oldPosition + offset;

  // 应用边界限制
  newPos = clampMapPosition(newPos);
  _villageLayer->setPosition(newPos);

  CCLOG("Zoom: %.3f -> %.3f around (%.0f, %.0f)",
        oldScale, newScale, zoomPoint.x, zoomPoint.y);
}
#pragma endregion
