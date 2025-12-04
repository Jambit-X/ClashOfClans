#include "VillageScene.h"

USING_NS_CC;

Scene* VillageScene::createScene() {
  return VillageScene::create();
}

bool VillageScene::init() {
  if (!Scene::init()) {
    return false;
  }

  // 只创建游戏层，包含基础的拖动功能
  _gameLayer = VillageLayer::create();
  this->addChild(_gameLayer, 1);

  return true;
}