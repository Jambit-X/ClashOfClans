#include "MoveMapController.h"
#include "../proj.win32/Constants.h"
#include <iostream>

USING_NS_CC;

// 静态常量定义
const float MoveMapController::MAX_SCALE = 3.0f;
const float MoveMapController::ZOOM_SPEED = 0.05f;

const float TAP_THRESHOLD = 15.0f;

MoveMapController::MoveMapController(Layer* villageLayer)
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
  , _onBuildingClicked(nullptr)
{
  calculateMinScale();
  _currentScale = _minScale;
}

MoveMapController::~MoveMapController() {
  cleanup();
}

#pragma region 初始化和清理
void MoveMapController::setupInputListeners() {
  // 初始化地图的缩放和位置
  initializeMapTransform();
  
  // 设置输入监听
  setupTouchHandling();
  setupMouseHandling();
  
  CCLOG("MoveMapController: Input listeners initialized");
  CCLOG("  Initial state: MAP_DRAG");
  CCLOG("  Scale range: %.3f - %.3f", _minScale, MAX_SCALE);
}

void MoveMapController::cleanup() {
  if (_touchListener) {
    Director::getInstance()->getEventDispatcher()->removeEventListener(_touchListener);
    _touchListener = nullptr;
  }
  
  if (_mouseListener) {
    Director::getInstance()->getEventDispatcher()->removeEventListener(_mouseListener);
    _mouseListener = nullptr;
  }
  
  CCLOG("MoveMapController: Cleaned up");
}

void MoveMapController::calculateMinScale() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  auto mapSize = _villageLayer->getContentSize();

  float scaleX = visibleSize.width / mapSize.width;
  float scaleY = visibleSize.height / mapSize.height;
  _minScale = std::max(scaleX, scaleY);

  CCLOG("MoveMapController: Calculated MIN_SCALE: %.3f", _minScale);
}

void MoveMapController::initializeMapTransform() {
  auto mapSize = _villageLayer->getContentSize();
  auto visibleSize = Director::getInstance()->getVisibleSize();

  // 设置初始缩放为最小缩放
  _villageLayer->setScale(_currentScale);

  // 将地图中心放在屏幕中心
  float initialX = (visibleSize.width - mapSize.width * _currentScale) / 2;
  float initialY = (visibleSize.height - mapSize.height * _currentScale) / 2;
  _villageLayer->setPosition(initialX, initialY);

  CCLOG("MoveMapController: Map transform initialized");
  CCLOG("  Initial scale: %.3f", _currentScale);
  CCLOG("  Initial position: (%.2f, %.2f)", initialX, initialY);
}
#pragma endregion

#pragma region 状态管理
void MoveMapController::changeState(InputState newState) {
  if (_currentState == newState) return;

  InputState oldState = _currentState;
  _currentState = newState;

  CCLOG("MoveMapController: State changed from %d to %d", (int)oldState, (int)newState);

  // 触发状态改变回调
  if (_onStateChanged) {
    _onStateChanged(oldState, newState);
  }
}
#pragma endregion

#pragma region 触摸事件处理
void MoveMapController::setupTouchHandling() {
  _touchListener = EventListenerTouchOneByOne::create();
  _touchListener->setSwallowTouches(true);

  _touchListener->onTouchBegan = CC_CALLBACK_2(MoveMapController::onTouchBegan, this);
  _touchListener->onTouchMoved = CC_CALLBACK_2(MoveMapController::onTouchMoved, this);
  _touchListener->onTouchEnded = CC_CALLBACK_2(MoveMapController::onTouchEnded, this);

  Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(
    _touchListener, _villageLayer);
}

bool MoveMapController::onTouchBegan(Touch* touch, Event* event) {
  CCLOG("MoveMapController::onTouchBegan - currentState=%d", (int)_currentState);
  
  // 只在 MAP_DRAG 状态处理输入
  if (_currentState != InputState::MAP_DRAG) {
    CCLOG("MoveMapController::onTouchBegan - Rejected: not in MAP_DRAG state");
    return false;
  }

  storeTouchStartState(touch);
  _isDragging = false;
  return true;
}

void MoveMapController::onTouchMoved(Touch* touch, Event* event) {
  if (_currentState != InputState::MAP_DRAG) {
    return;
  }

  // 只有拖动距离超过阈值才算拖动
  Vec2 currentPos = touch->getLocation();
  float distance = _touchStartPos.distance(currentPos);
  
  if (distance > TAP_THRESHOLD) {
    _isDragging = true;
    handleMapDragging(touch);
  }
}

void MoveMapController::onTouchEnded(Touch* touch, Event* event) {
  CCLOG("MoveMapController::onTouchEnded - currentState=%d, isDragging=%s", 
        (int)_currentState, _isDragging ? "true" : "false");
  
  if (_currentState != InputState::MAP_DRAG) {
    CCLOG("MoveMapController::onTouchEnded - Rejected: not in MAP_DRAG state");
    return;
  }

  Vec2 endPos = touch->getLocation();

  // 判断是 Tap 还是 Drag
  if (!_isDragging && isTapGesture(_touchStartPos, endPos)) {
    CCLOG("MoveMapController::onTouchEnded - Detected TAP gesture");
    handleTap(endPos);
  } else {
    CCLOG("MoveMapController::onTouchEnded - Not a tap: isDragging=%s, distance=%.2f", 
          _isDragging ? "true" : "false", _touchStartPos.distance(endPos));
  }

  _isDragging = false;
}

void MoveMapController::storeTouchStartState(Touch* touch) {
  _touchStartPos = touch->getLocation();
  _layerStartPos = _villageLayer->getPosition();
  
  CCLOG("Touch began at (%.2f, %.2f)", _touchStartPos.x, _touchStartPos.y);
}

void MoveMapController::handleMapDragging(Touch* touch) {
  Vec2 currentTouchPos = touch->getLocation();
  Vec2 delta = currentTouchPos - _touchStartPos;
  Vec2 newPos = _layerStartPos + delta;

  newPos = clampMapPosition(newPos);
  _villageLayer->setPosition(newPos);
}

void MoveMapController::handleTap(const Vec2& tapPosition) {
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
      
      // 触发建筑选中回调（不改变状态，保持 MAP_DRAG）
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

bool MoveMapController::isTapGesture(const Vec2& startPos, const Vec2& endPos) {
  float distance = startPos.distance(endPos);
  return distance < TAP_THRESHOLD;
}

Vec2 MoveMapController::clampMapPosition(const Vec2& position) {
  auto visibleSize = Director::getInstance()->getVisibleSize();

  // 使用 getBoundingBox 获取实际显示大小
  Rect mapBounds = _villageLayer->getBoundingBox();

  float minX, maxX, minY, maxY;

  // 如果地图比屏幕小，居中
  if (mapBounds.size.width <= visibleSize.width) {
    float centerX = (visibleSize.width - mapBounds.size.width) / 2;
    minX = maxX = centerX;
  } else {
    minX = visibleSize.width - mapBounds.size.width;
    maxX = 0;
  }

  if (mapBounds.size.height <= visibleSize.height) {
    float centerY = (visibleSize.height - mapBounds.size.height) / 2;
    minY = maxY = centerY;
  } else {
    minY = visibleSize.height - mapBounds.size.height;
    maxY = 0;
  }

  return Vec2(
    clampf(position.x, minX, maxX),
    clampf(position.y, minY, maxY)
  );
}
#pragma endregion

#pragma region 鼠标事件处理（缩放）
void MoveMapController::setupMouseHandling() {
  _mouseListener = EventListenerMouse::create();
  _mouseListener->onMouseScroll = CC_CALLBACK_1(MoveMapController::onMouseScroll, this);
  
  Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(
    _mouseListener, _villageLayer);
}

void MoveMapController::onMouseScroll(Event* event) {
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

float MoveMapController::calculateNewScale(float scrollDelta) {
  // 向上滚动缩小，向下滚动放大
  float zoomFactor = powf(0.9f, scrollDelta);
  float newScale = _currentScale * zoomFactor;

  // 限制缩放范围
  newScale = clampf(newScale, _minScale, MAX_SCALE);

  // 避免微小抖动
  if (abs(newScale - _currentScale) < 0.01f) {
    return _currentScale;
  }

  return newScale;
}

Vec2 MoveMapController::getAdjustedMousePosition(EventMouse* mouseEvent) {
  // 直接使用 getLocation()，无需手动转换
  return mouseEvent->getLocation();
}

void MoveMapController::applyZoomAroundPoint(const Vec2& zoomPoint, float newScale) {
  float oldScale = _currentScale;

  // 1. 将屏幕坐标转换为 Layer 内部坐标
  Vec2 pointInLayer = _villageLayer->convertToNodeSpace(zoomPoint);

  // 2. 应用新的缩放
  _villageLayer->setScale(newScale);
  _currentScale = newScale;

  // 3. 将同一个 Layer 内部点转回屏幕坐标
  Vec2 newPointOnScreen = _villageLayer->convertToWorldSpace(pointInLayer);

  // 4. 计算位置偏移
  Vec2 offset = zoomPoint - newPointOnScreen;
  Vec2 newPos = _villageLayer->getPosition() + offset;

  // 应用边界限制
  newPos = clampMapPosition(newPos);
  _villageLayer->setPosition(newPos);

  CCLOG("Zoom: %.3f -> %.3f around (%.0f, %.0f)",
        oldScale, newScale, zoomPoint.x, zoomPoint.y);
}
#pragma endregion
