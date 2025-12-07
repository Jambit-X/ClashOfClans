#include "VillageScene.h"
#include "Layer/HUDLayer.h" 
#include "Manager/VillageDataManager.h"

USING_NS_CC;

Scene* VillageScene::createScene() {
  return VillageScene::create();
}

bool VillageScene::init() {
  if (!Scene::init()) {
    return false;
  }

  // 直接获取单例，无需创建
  auto dataManager = VillageDataManager::getInstance();

  // 1. 添加游戏层（村庄地图）
  auto villageLayer = VillageLayer::create();
  villageLayer->setTag(1);  // 关键：设置 Tag = 1
  this->addChild(villageLayer, 0);

  // 2. 添加 HUD 层（UI 界面，包含商店按钮）
  auto hudLayer = HUDLayer::create();
  hudLayer->setTag(100);  // 可选：设置 HUD 的 Tag
  this->addChild(hudLayer, 10);

  CCLOG("VillageScene initialized with VillageLayer(tag=1) and HUDLayer(tag=100)");
  return true;
}