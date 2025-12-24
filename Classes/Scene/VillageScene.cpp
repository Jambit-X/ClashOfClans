#include "VillageScene.h"
#include "Layer/HUDLayer.h"
#include "Layer/BattleTroopLayer.h"
#include "Layer/ReplayListLayer.h"
#include "Manager/VillageDataManager.h"
#include "Manager/AudioManager.h"
#include "cocos2d.h"

USING_NS_CC;
using namespace cocos2d::ui;

Scene* VillageScene::createScene() {
  return VillageScene::create();
}

bool VillageScene::init() {
  if (!Scene::init()) {
    return false;
  }

  auto dataManager = VillageDataManager::getInstance();
  
  // ✅ 预加载背景音乐（避免播放时还在加载）
  auto audioManager = AudioManager::getInstance();
  audioManager->preloadAudio("Audios/village_background_music.m4a");

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

  // 获取可见区域大小
  auto visibleSize = Director::getInstance()->getVisibleSize();

  // ✅ 新增：回放按钮
  auto replayBtn = Button::create("UI/replay/replay_enter.png");
  replayBtn->setPosition(Vec2(60, visibleSize.height - 200));
  replayBtn->setScale(0.8f);
  replayBtn->addClickEventListener([this](Ref*) {
      // 打开回放列表
      auto replayList = ReplayListLayer::create();
      this->addChild(replayList, 1000);
  });
  this->addChild(replayBtn, 10);
  
  return true;
}

void VillageScene::onEnter() {
  Scene::onEnter();
  
  _backgroundMusicID = -1;
  
  auto audioManager = AudioManager::getInstance();
  _backgroundMusicID = audioManager->playBackgroundMusic(
      "Audios/village_background_music.mp3",
      1.0f,
      true
  );
}

void VillageScene::onExit() {
  auto audioManager = AudioManager::getInstance();
  if (_backgroundMusicID != -1) {
    audioManager->stopAudio(_backgroundMusicID);
    _backgroundMusicID = -1;
  }
  
  Scene::onExit();
}
