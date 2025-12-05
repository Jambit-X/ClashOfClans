#include "VillageLayer.h"
#include "../Controller/InputController.h"
#include "../Util/GridMapUtils.h"
#include "../proj.win32/Constants.h"
#include <iostream>
USING_NS_CC;

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

  // 2. 初始化基本属性
  initializeBasicProperties();

  // 3. 创建输入控制器（控制器会负责初始化缩放和位置）
  _inputController = new InputController(this);
  _inputController->setupInputListeners();

  CCLOG("VillageLayer initialized with InputController");

  return true;
}

void VillageLayer::cleanup() {
  if (_inputController) {
    _inputController->cleanup();
    delete _inputController;
    _inputController = nullptr;
  }
  
  Layer::cleanup();
}

#pragma region 初始化方法
void VillageLayer::initializeBasicProperties() {
  auto mapSize = _mapSprite->getContentSize();
  this->setContentSize(mapSize);

  // 使用左下角锚点
  this->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);

  CCLOG("VillageLayer basic properties initialized:");
  CCLOG("  Map size: %.0fx%.0f", mapSize.width, mapSize.height);
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
#pragma endregion