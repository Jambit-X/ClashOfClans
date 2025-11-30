#include "VillageScene.h"
#include "../Model/PlayerData.h" // 引入数据模型

USING_NS_CC;

Scene* VillageScene::createScene() {
  return VillageScene::create();
}

bool VillageScene::init() {
  if (!Scene::init()) {
    return false;
  }

  // 1. 初始化 Layer (保持你之前的代码)
  _gameLayer = VillageLayer::create();
  this->addChild(_gameLayer, 1);

  _hudLayer = HUDLayer::create();
  this->addChild(_hudLayer, 2);

  // 2. 初始化控制器
  _resourceController = new ResourceController();
  _upgradeController = new UpgradeController();

  // 3. 开启 Update 调度器 (关键步骤)
  this->scheduleUpdate();

  // ---------------------------------------------------------
  // [测试代码] 添加一个临时按钮来测试 "消耗金币升级"
  // 实际项目中这会在 UI 层的点击事件里
  auto testBtn = MenuItemLabel::create(
    Label::createWithTTF("Test Upgrade (-500 Gold)", "fonts/Marker Felt.ttf", 20),
    [=](Ref* sender) {
    // 点击回调：尝试消耗 500 金币
    bool result = _upgradeController->tryUpgrade(500, true);
    if (result) {
      // 如果成功，可以在这里播放音效等
      log("Button Clicked: Upgrade Started");
    }
  }
  );
  testBtn->setPosition(Vec2(0, -100)); // 放在屏幕中心偏下
  auto menu = Menu::create(testBtn, nullptr);
  this->addChild(menu, 10);

  return true;
}

// 游戏主循环
void VillageScene::update(float dt) {
  // 1. 让资源控制器计算产出
  if (_resourceController) {
    _resourceController->update(dt);
  }

  // 2. 从 Model 获取最新数据
  auto data = PlayerData::getInstance();

  // 3. 刷新 UI 显示
  if (_hudLayer) {
    _hudLayer->updateResourceDisplay(data->getGold(), data->getElixir());
  }
}