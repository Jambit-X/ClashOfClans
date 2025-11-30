#include "VillageLayer.h"

USING_NS_CC;

bool VillageLayer::init() {
  if (!Layer::init()) {
    return false;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();

  // 1. 设置地图背景 (简化版，使用一个大图模拟地图)
  // 实际项目中地图可能更大，这里使用一个比屏幕大的 Sprite 来模拟可拖拽区域
  auto mapSprite = Sprite::create("Scene/VillageScene.png"); // 假设 Resources 文件夹有这个图
  if (!mapSprite) {
    // 如果没有图，创建一个红色的 Layer 以示占位
    mapSprite = Sprite::create();
    mapSprite->setTextureRect(Rect(0, 0, 2000, 2000));
    mapSprite->setColor(Color3B(50, 50, 50)); // 深灰色
  }

  mapSprite->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
  mapSprite->setPosition(Vec2::ZERO);
  this->addChild(mapSprite);

  // 设置 GameLayer 的内容大小等于地图大小
  this->setContentSize(mapSprite->getContentSize());

  // 2. 启用触摸事件
  setupTouchHandling();

  // 3. 将地图中心移动到屏幕中心附近 (初始位置)
  this->setPosition(visibleSize.width / 2 - mapSprite->getContentSize().width / 2,
                    visibleSize.height / 2 - mapSprite->getContentSize().height / 2);

  return true;
}

void VillageLayer::setupTouchHandling() {
  // 创建单点触摸监听器
  auto listener = EventListenerTouchOneByOne::create();
  listener->setSwallowTouches(true); // 吞噬触摸，防止事件穿透

  listener->onTouchBegan = CC_CALLBACK_2(VillageLayer::onTouchBegan, this);
  listener->onTouchMoved = CC_CALLBACK_2(VillageLayer::onTouchMoved, this);
  listener->onTouchEnded = CC_CALLBACK_2(VillageLayer::onTouchEnded, this);

  _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

bool VillageLayer::onTouchBegan(Touch* touch, Event* event) {
  // 记录触摸开始时的位置
  _touchStartPos = touch->getLocation();
  // 记录 Layer 当前的位置
  _layerStartPos = this->getPosition();
  return true; // 总是返回 true 接收后续的 Moved/Ended 事件
}

void VillageLayer::onTouchMoved(Touch* touch, Event* event) {
  // 计算触摸点移动的距离
  Vec2 diff = touch->getLocation() - _touchStartPos;

  // 计算 Layer 应该移动到的新位置
  Vec2 newPos = _layerStartPos + diff;

  // 限制地图边界 (防止地图边缘移出屏幕)
  auto visibleSize = Director::getInstance()->getVisibleSize();
  auto mapSize = this->getContentSize();

  float minX = visibleSize.width - mapSize.width;
  float maxX = 0;
  float minY = visibleSize.height - mapSize.height;
  float maxY = 0;

  // 钳制新位置
  newPos.x = clampf(newPos.x, minX, maxX);
  newPos.y = clampf(newPos.y, minY, maxY);

  this->setPosition(newPos);
}

void VillageLayer::onTouchEnded(Touch* touch, Event* event) {
  // 触摸结束，如果只是轻点，可以在这里处理点击事件，但本阶段只关注平移
}