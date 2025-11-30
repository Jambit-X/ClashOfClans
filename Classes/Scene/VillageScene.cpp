#include "VillageScene.h"

USING_NS_CC;

Scene* VillageScene::createScene() {
  // 使用 Scene::create() 而不是 GameScene::create()，因为 GameScene 继承自 Scene
  return VillageScene::create();
}

bool VillageScene::init() {
  if (!Scene::init()) {
    return false;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 1. 创建 GameLayer (负责地图和建筑)
  _villageLayer = VillageLayer::create();
  this->addChild(_villageLayer, 1); // Z-Order 1

  // 2. 创建 HUDLayer (负责 UI)
  _hudLayer = HUDLayer::create();
  this->addChild(_hudLayer, 2); // Z-Order 2 (在地图上方)

  return true;
}