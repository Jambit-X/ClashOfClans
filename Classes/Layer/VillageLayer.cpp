#include "VillageLayer.h"
#include "../proj.win32/Constants.h"
#include <iostream>
USING_NS_CC;

// 静态常量定义
const float VillageLayer::MAX_SCALE = 3.0f;  // 最大放大倍率
const float VillageLayer::ZOOM_SPEED = 0.05f; // 更平滑的缩放速度

bool VillageLayer::init() {
  if (!Layer::init()) {
    return false;
  }

  // 1. 创建地图精灵
  _mapSprite = createMapSprite();
  if (!_mapSprite) {
    return false;
  }
  this->addChild(_mapSprite);

  // 2. 初始化位置
  initializeBasicProperties();

  // 3. 设置事件处理
  setupEventHandling();

  return true;
}

#pragma region 初始化方法
void VillageLayer::calculateMinScale() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  auto mapSize = this->getContentSize();

  // 计算使地图完全覆盖屏幕所需的最小缩放值（宽/高方向取最大）
  float scaleX = visibleSize.width / mapSize.width;
  float scaleY = visibleSize.height / mapSize.height;
  _minScale = std::max(scaleX, scaleY);

  CCLOG("Screen: %.0fx%.0f, Map: %.0fx%.0f",
        visibleSize.width, visibleSize.height,
        mapSize.width, mapSize.height);
  CCLOG("Scale ratios - X: %.3f, Y: %.3f", scaleX, scaleY);
  CCLOG("Calculated MIN_SCALE: %.3f (地图始终覆盖屏幕)", _minScale);
}

void VillageLayer::initializeBasicProperties() {
  auto mapSize = _mapSprite->getContentSize();
  this->setContentSize(mapSize);

  // 使用左下角锚点
  this->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);

  // 先计算最小缩放
  calculateMinScale();
  _currentScale = _minScale;
  this->setScale(_currentScale);

  // 将地图中心放在屏幕中心
  auto visibleSize = Director::getInstance()->getVisibleSize();
  float initialX = (visibleSize.width - mapSize.width * _currentScale) / 2;
  float initialY = (visibleSize.height - mapSize.height * _currentScale) / 2;
  this->setPosition(initialX, initialY);

  CCLOG("VillageLayer initialized:");
  CCLOG("  Map size: %.0fx%.0f", mapSize.width, mapSize.height);
  CCLOG("  Scale: %.3f (range: %.3f - %.3f)", _currentScale, _minScale, MAX_SCALE);
  CCLOG("  Layer position: (%.2f, %.2f)", this->getPosition().x, this->getPosition().y);
}

void VillageLayer::setupEventHandling() {
  setupTouchHandling(); // 拖动
  setupMouseHandling(); // 缩放
}
#pragma endregion

#pragma region 辅助方法
Sprite* VillageLayer::createMapSprite() {
  auto mapSprite = Sprite::create("Scene/LinedVillageScene.jpg");
  if (!mapSprite) {
    CCLOG("Error: Failed to load map image");
    return nullptr;
  }

  // 使用左下角锚点，位置设为(0,0)
  mapSprite->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
  mapSprite->setPosition(Vec2::ZERO);
  return mapSprite;
}

Vec2 VillageLayer::calculateCenterPosition() {
  return Director::getInstance()->getVisibleSize() / 2.0f;
}
#pragma endregion

#pragma region 触摸事件处理（拖动）
void VillageLayer::setupTouchHandling() {
  auto listener = EventListenerTouchOneByOne::create();
  listener->setSwallowTouches(true);

  listener->onTouchBegan = CC_CALLBACK_2(VillageLayer::onTouchBegan, this);
  listener->onTouchMoved = CC_CALLBACK_2(VillageLayer::onTouchMoved, this);
  listener->onTouchEnded = CC_CALLBACK_2(VillageLayer::onTouchEnded, this);

  _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

bool VillageLayer::onTouchBegan(Touch* touch, Event* event) {
  storeTouchStartState(touch);
  return true;
}

void VillageLayer::onTouchMoved(Touch* touch, Event* event) {
  handleMapDragging(touch);
}

void VillageLayer::onTouchEnded(Touch* touch, Event* event) {
  // 拖动结束
}

void VillageLayer::storeTouchStartState(Touch* touch) {
  _touchStartPos = touch->getLocation();
  _layerStartPos = this->getPosition();
  CCLOG("Touch began at (%.2f, %.2f), Layer start pos (%.2f, %.2f)",
        _touchStartPos.x, _touchStartPos.y,
        _layerStartPos.x, _layerStartPos.y);
}

void VillageLayer::handleMapDragging(Touch* touch) {
  Vec2 currentTouchPos = touch->getLocation();
  Vec2 delta = currentTouchPos - _touchStartPos;
  Vec2 newPos = _layerStartPos + delta;

  newPos = clampMapPosition(newPos);
  this->setPosition(newPos);
}

Vec2 VillageLayer::clampMapPosition(const Vec2& position) {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  auto mapSize = this->getContentSize() * this->getScale();

  float minX, maxX, minY, maxY;

  if (mapSize.width <= visibleSize.width) {
    // 地图宽度小于屏幕，居中显示
    float centerX = (visibleSize.width - mapSize.width) / 2;
    minX = maxX = centerX;
  } else {
    // 地图宽度大于屏幕，允许拖拽
    minX = visibleSize.width - mapSize.width;  // 右边界
    maxX = 0;  // 左边界
  }

  if (mapSize.height <= visibleSize.height) {
    // 地图高度小于屏幕，居中显示
    float centerY = (visibleSize.height - mapSize.height) / 2;
    minY = maxY = centerY;
  } else {
    // 地图高度大于屏幕，允许拖拽
    minY = visibleSize.height - mapSize.height;  // 上边界
    maxY = 0;  // 下边界
  }

  return Vec2(
    clampf(position.x, minX, maxX),
    clampf(position.y, minY, maxY)
  );
}
#pragma endregion

#pragma region 鼠标事件处理（缩放）
void VillageLayer::setupMouseHandling() {
  auto mouseListener = EventListenerMouse::create();
  mouseListener->onMouseScroll = CC_CALLBACK_1(VillageLayer::onMouseScroll, this);
  _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
}

void VillageLayer::onMouseScroll(Event* event) {
  EventMouse* mouseEvent = static_cast<EventMouse*>(event);
  float scrollY = mouseEvent->getScrollY();

  float newScale = calculateNewScale(scrollY);
  if (newScale == _currentScale) return;

  Vec2 mousePos = getAdjustedMousePosition(mouseEvent);
  applyZoomAroundPoint(mousePos, newScale);
}

float VillageLayer::calculateNewScale(float scrollDelta) {
  // 使用指数缩放，让缩放感觉更自然
  float scaleFactor = 1.0f + (scrollDelta * ZOOM_SPEED);
  float newScale = _currentScale * scaleFactor;

  // 限制缩放范围
  newScale = clampf(newScale, _minScale, MAX_SCALE);

  // 如果变化很小，就不更新（避免微小抖动）
  if (abs(newScale - _currentScale) < 0.01f) {
    return _currentScale;
  }

  return newScale;
}

Vec2 VillageLayer::getAdjustedMousePosition(EventMouse* mouseEvent) {
  Vec2 mousePosInView = mouseEvent->getLocationInView();
  auto winSize = Director::getInstance()->getWinSize();

  // 从左上角原点转换为左下角原点
  mousePosInView.y = winSize.height - mousePosInView.y;
  return mousePosInView;
}

void VillageLayer::applyZoomAroundPoint(const Vec2& zoomPoint, float newScale) {
  // 保存当前状态
  Vec2 oldPosition = this->getPosition();
  float oldScale = _currentScale;

  // 计算缩放中心点在Layer中的相对位置
  Vec2 localPoint = zoomPoint - oldPosition;

  // 应用新的缩放
  this->setScale(newScale);
  _currentScale = newScale;

  // 计算缩放后需要调整的位置
  // 使缩放中心点保持在屏幕上的相同位置
  float scaleRatio = newScale / oldScale;
  Vec2 newLocalPoint = localPoint * scaleRatio;
  Vec2 offset = localPoint - newLocalPoint;
  Vec2 newPos = oldPosition + offset;

  // 应用边界限制
  newPos = clampMapPosition(newPos);
  this->setPosition(newPos);

  CCLOG("Zoom: %.3f -> %.3f around (%.0f, %.0f)",
        oldScale, newScale, zoomPoint.x, zoomPoint.y);
}
#pragma endregion