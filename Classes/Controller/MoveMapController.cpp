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
  , _multiTouchListener(nullptr)
  , _mouseListener(nullptr)
  , _isPinching(false)
  , _initialPinchDistance(0.0f)
  , _initialPinchScale(1.0f)
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
  if (_multiTouchListener) {
    Director::getInstance()->getEventDispatcher()->removeEventListener(_multiTouchListener);
    _multiTouchListener = nullptr;
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
  _multiTouchListener = EventListenerTouchAllAtOnce::create();

  _multiTouchListener->onTouchesBegan = CC_CALLBACK_2(MoveMapController::onTouchesBegan, this);
  _multiTouchListener->onTouchesMoved = CC_CALLBACK_2(MoveMapController::onTouchesMoved, this);
  _multiTouchListener->onTouchesEnded = CC_CALLBACK_2(MoveMapController::onTouchesEnded, this);

  Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(
    _multiTouchListener, _villageLayer);
}

void MoveMapController::onTouchesBegan(const std::vector<Touch*>& touches, Event* event) {
  CCLOG("MoveMapController::onTouchesBegan - newTouchCount=%zu, currentState=%d", touches.size(), (int)_currentState);
  
  // 只在 MAP_DRAG 状态处理输入
  if (_currentState != InputState::MAP_DRAG) {
    CCLOG("MoveMapController::onTouchesBegan - Rejected: not in MAP_DRAG state");
    return;
  }

  // 添加新触点到活跃列表
  for (auto touch : touches) {
    _activeTouches[touch->getID()] = touch;
  }
  
  CCLOG("Active touches count: %zu", _activeTouches.size());

  if (_activeTouches.size() == 1) {
    // 单指触摸 - 记录起始位置
    storeTouchStartState(touches[0]);
    _isDragging = false;
    _isPinching = false;
  } else if (_activeTouches.size() >= 2) {
    // 双指触摸 - 开始捏合缩放
    _isPinching = true;
    _isDragging = false;
    
    // 从活跃触点获取前两个
    std::vector<Touch*> activeTouchList;
    for (auto& pair : _activeTouches) {
      activeTouchList.push_back(pair.second);
      if (activeTouchList.size() >= 2) break;
    }
    
    _initialPinchDistance = getTouchDistance(activeTouchList);
    _initialPinchScale = _currentScale;
    _pinchCenter = getTouchCenter(activeTouchList);
    CCLOG("Pinch began: distance=%.2f, scale=%.3f", _initialPinchDistance, _initialPinchScale);
  }
}

void MoveMapController::onTouchesMoved(const std::vector<Touch*>& touches, Event* event) {
  if (_currentState != InputState::MAP_DRAG) {
    return;
  }

  // 更新活跃触点位置
  for (auto touch : touches) {
    _activeTouches[touch->getID()] = touch;
  }

  if (_activeTouches.size() >= 2 && _isPinching) {
    // 双指捏合缩放 - 使用活跃触点
    std::vector<Touch*> activeTouchList;
    for (auto& pair : _activeTouches) {
      activeTouchList.push_back(pair.second);
      if (activeTouchList.size() >= 2) break;
    }
    
    float currentDistance = getTouchDistance(activeTouchList);
    if (_initialPinchDistance > 0) {
      float scaleFactor = currentDistance / _initialPinchDistance;
      float newScale = _initialPinchScale * scaleFactor;
      
      // 限制缩放范围
      newScale = clampf(newScale, _minScale, MAX_SCALE);
      
      if (abs(newScale - _currentScale) > 0.01f) {
        Vec2 center = getTouchCenter(activeTouchList);
        applyZoomAroundPoint(center, newScale);
      }
    }
  } else if (_activeTouches.size() == 1 && !_isPinching) {
    // 单指拖动地图
    auto it = _activeTouches.begin();
    Vec2 currentPos = it->second->getLocation();
    float distance = _touchStartPos.distance(currentPos);
    
    if (distance > TAP_THRESHOLD) {
      _isDragging = true;
      handleMapDragging(it->second);
    }
  }
}

void MoveMapController::onTouchesEnded(const std::vector<Touch*>& touches, Event* event) {
  CCLOG("MoveMapController::onTouchesEnded - endedCount=%zu, activeBefore=%zu, isDragging=%s, isPinching=%s", 
        touches.size(), _activeTouches.size(), _isDragging ? "true" : "false", _isPinching ? "true" : "false");
  
  if (_currentState != InputState::MAP_DRAG) {
    CCLOG("MoveMapController::onTouchesEnded - Rejected: not in MAP_DRAG state");
    return;
  }

  // 从活跃列表移除结束的触点
  for (auto touch : touches) {
    _activeTouches.erase(touch->getID());
  }
  
  CCLOG("Active touches after removal: %zu", _activeTouches.size());

  // 如果是捏合结束（触点少于2个），重置状态
  if (_isPinching && _activeTouches.size() < 2) {
    _isPinching = false;
    CCLOG("Pinch ended");
    
    // 如果还有一个触点，转为拖动模式
    if (_activeTouches.size() == 1) {
      auto it = _activeTouches.begin();
      storeTouchStartState(it->second);
    }
    return;
  }

  // 只有单指触摸且没有拖动才检测点击
  if (_activeTouches.size() == 0 && !_isDragging && !_isPinching) {
    Vec2 endPos = touches[0]->getLocation();
    if (isTapGesture(_touchStartPos, endPos)) {
      CCLOG("MoveMapController::onTouchesEnded - Detected TAP gesture");
      handleTap(endPos);
    }
  }

  if (_activeTouches.size() == 0) {
    _isDragging = false;
  }
}

// 计算两个触点之间的距离
float MoveMapController::getTouchDistance(const std::vector<Touch*>& touches) {
  if (touches.size() < 2) return 0;
  Vec2 p1 = touches[0]->getLocation();
  Vec2 p2 = touches[1]->getLocation();
  return p1.distance(p2);
}

// 计算两个触点的中心点
Vec2 MoveMapController::getTouchCenter(const std::vector<Touch*>& touches) {
  if (touches.size() < 2) {
    return touches.size() == 1 ? touches[0]->getLocation() : Vec2::ZERO;
  }
  Vec2 p1 = touches[0]->getLocation();
  Vec2 p2 = touches[1]->getLocation();
  return (p1 + p2) / 2;
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
