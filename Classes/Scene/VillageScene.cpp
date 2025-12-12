#include "VillageScene.h"
#include "Layer/HUDLayer.h"
#include "Layer/BattleTroopLayer.h"
#include "Manager/VillageDataManager.h"

USING_NS_CC;

Scene* VillageScene::createScene() {
  return VillageScene::create();
}

bool VillageScene::init() {
  if (!Scene::init()) {
    return false;
  }

  auto dataManager = VillageDataManager::getInstance();

  // 1. 村庄层（地图、建筑）
  auto villageLayer = VillageLayer::create();
  villageLayer->setTag(1);
  this->addChild(villageLayer, 0);

  // 2. 军队层（战斗单位）
  // ? 关键修复：作为 VillageLayer 的子节点，而不是 Scene 的子节点
  // 这样军队会跟随地图的缩放和移动
  auto troopLayer = BattleTroopLayer::create();
  troopLayer->setTag(2);
  villageLayer->addChild(troopLayer, 10);  // ? 添加到 villageLayer，zOrder=10 在地图之上
  
  CCLOG("========================================");
  CCLOG("BattleTroopLayer added as CHILD of VillageLayer");
  CCLOG("  This ensures troops scale with map (0.345x)");
  CCLOG("========================================");

  // 3. HUD 层（UI 界面，商店按钮）
  auto hudLayer = HUDLayer::create();
  hudLayer->setTag(100);
  this->addChild(hudLayer, 10);  // HUD 保持在 Scene 级别，不受地图缩放影响

  CCLOG("VillageScene initialized:");
  CCLOG("  - VillageLayer (tag=1, zOrder=0)");
  CCLOG("      └─ BattleTroopLayer (tag=2, zOrder=10) ← CHILD of VillageLayer");
  CCLOG("  - HUDLayer (tag=100, zOrder=10)");
  
  return true;
}