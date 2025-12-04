#include "VillageLayer.h"

USING_NS_CC;

// 静态常量定义
const float VillageLayer::MIN_SCALE = 0.5f;
const float VillageLayer::MAX_SCALE = 2.0f;
const float VillageLayer::ZOOM_SPEED = 0.1f;

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

  // 2. 设置Layer的内容大小为地图大小
  this->setContentSize(_mapSprite->getContentSize());

  // 3. 初始化缩放和位置
  initializeBasicProperties();

  // 4. 设置事件处理
  setupEventHandling();

  return true;
}

#pragma region 初始化方法

void VillageLayer::initializeBasicProperties() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  auto mapSize = this->getContentSize();

  // 计算合适的初始缩放，让地图适配窗口
  float scaleX = visibleSize.width / mapSize.width;
  float scaleY = visibleSize.height / mapSize.height;
  
  // 使用较小的缩放值，确保地图完全可见
  float initialScale = std::min(scaleX, scaleY) * 0.9f; // 留10%边距
  
  // 限制在允许范围内
  _currentScale = clampf(initialScale, MIN_SCALE, MAX_SCALE);
  this->setScale(_currentScale);

  // 计算居中位置
  Vec2 initialPos = calculateCenterPosition();
  this->setPosition(initialPos);

  CCLOG("VillageLayer initialized:");
  CCLOG("  Map size: %.0f x %.0f", mapSize.width, mapSize.height);
  CCLOG("  Window size: %.0f x %.0f", visibleSize.width, visibleSize.height);
  CCLOG("  Initial scale: %.2f", _currentScale);
  CCLOG("  Initial position: (%.0f, %.0f)", initialPos.x, initialPos.y);
}

void VillageLayer::setupEventHandling() {
  setupTouchHandling();
  setupMouseHandling();
}

#pragma endregion

#pragma region 辅助方法

Sprite* VillageLayer::createMapSprite() {
  auto mapSprite = Sprite::create("Scene/LinedVillageScene.jpg");
  if (!mapSprite) {
    // 如果找不到图片则创建占位符
    mapSprite = Sprite::create();
    mapSprite->setTextureRect(Rect(0, 0, 3705, 2545));
    mapSprite->setColor(Color3B(50, 150, 50));
  }

  // 设置锚点为左下角，位置为原点
  mapSprite->setAnchorPoint(Vec2::ZERO);
  mapSprite->setPosition(Vec2::ZERO);

  return mapSprite;
}

Vec2 VillageLayer::calculateCenterPosition() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  
  // 获取缩放后的地图实际显示大小
  Size scaledMapSize = this->getContentSize() * this->getScale();

  // 计算居中位置（Layer的锚点默认是(0,0)）
  float x = (visibleSize.width - scaledMapSize.width) / 2.0f;
  float y = (visibleSize.height - scaledMapSize.height) / 2.0f;

  return Vec2(x, y);
}

#pragma endregion

#pragma region 触摸事件处理

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
  CCLOG("VillageLayer: Map drag started");
  return true;
}

void VillageLayer::onTouchMoved(Touch* touch, Event* event) {
  handleMapDragging(touch);
}

void VillageLayer::onTouchEnded(Touch* touch, Event* event) {
  CCLOG("VillageLayer: Map drag ended");
}

void VillageLayer::storeTouchStartState(Touch* touch) {
  _touchStartPos = touch->getLocation();
  _layerStartPos = this->getPosition();
}

void VillageLayer::handleMapDragging(Touch* touch) {
  Vec2 currentTouchPos = touch->getLocation();
  Vec2 delta = currentTouchPos - _touchStartPos;
  Vec2 newPos = _layerStartPos + delta;

  // 应用边界约束
  newPos = clampMapPosition(newPos);
  this->setPosition(newPos);
}

Vec2 VillageLayer::clampMapPosition(const Vec2& position) {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Size scaledMapSize = this->getContentSize() * this->getScale();

  float clampedX = position.x;
  float clampedY = position.y;

  // X轴边界处理
  if (scaledMapSize.width <= visibleSize.width) {
    // 地图比窗口小，居中显示
    clampedX = (visibleSize.width - scaledMapSize.width) / 2.0f;
  } else {
    // 地图比窗口大，限制拖动范围
    // 最小X：地图右边缘对齐窗口右边缘
    float minX = visibleSize.width - scaledMapSize.width;
    // 最大X：地图左边缘对齐窗口左边缘
    float maxX = 0.0f;
    clampedX = clampf(position.x, minX, maxX);
  }

  // Y轴边界处理
  if (scaledMapSize.height <= visibleSize.height) {
    // 地图比窗口小，居中显示
    clampedY = (visibleSize.height - scaledMapSize.height) / 2.0f;
  } else {
    // 地图比窗口大，限制拖动范围
    // 最小Y：地图顶边缘对齐窗口顶边缘
    float minY = visibleSize.height - scaledMapSize.height;
    // 最大Y：地图底边缘对齐窗口底边缘
    float maxY = 0.0f;
    clampedY = clampf(position.y, minY, maxY);
  }

  return Vec2(clampedX, clampedY);
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

  // 计算新的缩放值
  float newScale = calculateNewScale(scrollY);
  if (std::abs(newScale - _currentScale) < 0.01f) {
    return; // 缩放值没有实质变化
  }

  // 获取鼠标在屏幕上的位置
  Vec2 mousePos = getAdjustedMousePosition(mouseEvent);
  
  // 以鼠标位置为中心进行缩放
  applyZoomAroundPoint(mousePos, newScale);
}

float VillageLayer::calculateNewScale(float scrollDelta) {
  // 向上滚动（scrollDelta > 0）放大，向下滚动缩小
  float newScale = _currentScale + scrollDelta * ZOOM_SPEED;
  return clampf(newScale, MIN_SCALE, MAX_SCALE);
}

Vec2 VillageLayer::getAdjustedMousePosition(EventMouse* mouseEvent) {
  // 获取鼠标在窗口中的位置（左上角为原点）
  Vec2 mousePos = mouseEvent->getLocationInView();
  
  // 转换为Cocos2d坐标系（左下角为原点）
  auto visibleSize = Director::getInstance()->getVisibleSize();
  mousePos.y = visibleSize.height - mousePos.y;
  
  return mousePos;
}

void VillageLayer::applyZoomAroundPoint(const Vec2& screenPoint, float newScale) {
  // 1. 将屏幕坐标转换为Layer的本地坐标（缩放前）
  Vec2 pointInLayer = this->convertToNodeSpace(screenPoint);

  // 2. 应用新的缩放
  float oldScale = _currentScale;
  _currentScale = newScale;
  this->setScale(_currentScale);

  // 3. 计算缩放后，该点在世界坐标系中的新位置
  Vec2 pointInWorldAfterScale = this->convertToWorldSpace(pointInLayer);

  // 4. 计算需要的偏移量，使该点保持在原来的屏幕位置
  Vec2 offset = screenPoint - pointInWorldAfterScale;

  // 5. 应用偏移并限制边界
  Vec2 newPos = this->getPosition() + offset;
  newPos = clampMapPosition(newPos);
  this->setPosition(newPos);

  CCLOG("Zoom: %.2f -> %.2f at screen point (%.0f, %.0f)", 
        oldScale, newScale, screenPoint.x, screenPoint.y);
}

#pragma endregion