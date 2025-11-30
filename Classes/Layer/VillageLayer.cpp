#include "VillageLayer.h"
#include "../Model/Building.h"
#include "../View/BuildingSprite.h"
#include "../System/GridMap.h"

USING_NS_CC;

bool VillageLayer::init() {
  if (!Layer::init()) {
    return false;
  }
  // 初始化缩放比例
  _currentScale = 1.0f;
  this->setScale(_currentScale);

  // 启用触摸和平移事件 (保持不变)
  setupTouchHandling();

  // 新增：启用鼠标滚轮事件
  setupMouseHandling();

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

  mapSprite->setAnchorPoint(Vec2::ANCHOR_MIDDLE); // 中心锚点
  mapSprite->setPosition(this->getContentSize() / 2);
  this->addChild(mapSprite);

  // 设置 GameLayer 的内容大小等于地图大小
  this->setContentSize(mapSprite->getContentSize());
  this->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
  this->setPosition(visibleSize / 2);
  // 2. 启用触摸事件
  setupTouchHandling();

  // 3. 将地图中心移动到屏幕中心附近 (初始位置)
  this->setPosition(visibleSize.width / 2 - mapSprite->getContentSize().width / 2,
                    visibleSize.height / 2 - mapSprite->getContentSize().height / 2);

  // --- [测试代码] ---
    // 1. 创建一个数据模型：ID=1, 类型=大本营, 坐标=(5,5), 大小=3x3格
  Building* townHallData = new Building(1, Building::Type::TOWN_HALL, 5, 5, 3, 3);

  // 2. 创建视图并绑定数据
  BuildingSprite* townHallSprite = BuildingSprite::createWithBuilding(townHallData);

  // 3. 添加到地图层
  this->addChild(townHallSprite);
  // -----------------

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

// -------------------------------------------------------------
// 鼠标事件监听设置
void VillageLayer::setupMouseHandling() {
  auto mouseListener = EventListenerMouse::create();
  // 绑定滚轮事件回调
  mouseListener->onMouseScroll = CC_CALLBACK_1(VillageLayer::onMouseScroll, this);
  _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
}


// 鼠标滚轮缩放回调函数 (核心逻辑)
void VillageLayer::onMouseScroll(Event* event) {
  EventMouse* mouseEvent = static_cast<EventMouse*>(event);
  float scrollY = mouseEvent->getScrollY();

  float newScale = _currentScale + scrollY * ZOOM_SPEED;
  newScale = clampf(newScale, MIN_SCALE, MAX_SCALE);

  if (newScale == _currentScale) return;

  // 1. 鼠标在屏幕坐标
  Vec2 mousePosInView = mouseEvent->getLocationInView();
  // Cocos2d-x 的 getLocationInView 原点在左上角，需转为左下角
  auto winSize = Director::getInstance()->getWinSize();
  mousePosInView.y = winSize.height - mousePosInView.y;

  // 2. 鼠标在当前 layer 的局部坐标（以锚点为中心）
  Vec2 nodeSpaceBefore = this->convertToNodeSpaceAR(mousePosInView);

  // 3. 设置缩放
  this->setScale(newScale);

  // 4. 缩放后，鼠标指向的地图点应该保持在屏幕同一位置
  Vec2 nodeSpaceAfter = nodeSpaceBefore * (newScale / _currentScale);
  Vec2 worldDelta = nodeSpaceAfter - nodeSpaceBefore;
  Vec2 newPos = this->getPosition() - worldDelta;

  // 5. 限制缩放后的位置，防止地图超出边界
  auto visibleSize = Director::getInstance()->getVisibleSize();
  auto mapSize = this->getContentSize() * newScale;

  float minX = visibleSize.width - mapSize.width;
  float maxX = 0;
  float minY = visibleSize.height - mapSize.height;
  float maxY = 0;

  newPos.x = clampf(newPos.x, minX, maxX);
  newPos.y = clampf(newPos.y, minY, maxY);

  this->setPosition(newPos);

  _currentScale = newScale;
}