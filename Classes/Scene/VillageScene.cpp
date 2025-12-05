#include "VillageScene.h"
#include "Layer/HUDLayer.h" 

USING_NS_CC;

Scene* VillageScene::createScene() {
  return VillageScene::create();
}

bool VillageScene::init() {
  if (!Scene::init()) {
    return false;
  }

  // 1. 添加游戏层（村庄地图）
  _gameLayer = VillageLayer::create();
  this->addChild(_gameLayer, 1);

  // 2. 添加 HUD 层（UI 界面，包含商店按钮）
  auto hudLayer = HUDLayer::create();
  this->addChild(hudLayer, 10);  // zOrder 设高一点

  return true;
}