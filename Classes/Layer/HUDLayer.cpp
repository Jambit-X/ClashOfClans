#include "HUDLayer.h"
#include "Layer/ShopLayer.h"

USING_NS_CC;

bool HUDLayer::init() {
  if (!Layer::init()) {
    return false;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 1. 创建金币标签 (左上角)
  _goldLabel = Label::createWithTTF("Gold: 0", "fonts/Marker Felt.ttf", 24); // 假设fonts/Marker Felt.ttf存在
  _goldLabel->setPosition(Vec2(origin.x + 100, origin.y + visibleSize.height - 30));
  this->addChild(_goldLabel);

  // 2. 创建圣水标签 (金币标签旁边)
  _elixirLabel = Label::createWithTTF("Elixir: 0", "fonts/Marker Felt.ttf", 24);
  _elixirLabel->setPosition(Vec2(origin.x + 300, origin.y + visibleSize.height - 30));
  this->addChild(_elixirLabel);

  // 初始更新显示
  updateResourceDisplay(500, 500);
 
  // --- 商店入口按钮开始 ---

// 1. 创建按钮

  auto shopBtn = ui::Button::create("UI/Shop/Shop-button.png");

  // 2. 设置位置 (左下角)
  shopBtn->setAnchorPoint(Vec2(1, 0)); // 锚点设为左下角
  shopBtn->setPosition(Vec2(origin.x + visibleSize.width - 20, origin.y + 20));  // 距离左边20，距离下边20

  // 3. 点击回调 
  shopBtn->addClickEventListener([=](Ref* sender) {
      // 调试日志
      CCLOG("Member C: 点击了商店图标，准备打开商店...");

      // 创建我们在 ShopLayer.cpp 里写的那个界面
      auto shopLayer = ShopLayer::create();

      // 将商店层添加到当前的 Scene (场景) 中
      // zOrder 设为 100，确保它盖在所有东西上面
      this->getScene()->addChild(shopLayer, 100);
      });

  // 4. 加到 HUD 层上
  this->addChild(shopBtn);

  return true;
}

void HUDLayer::updateResourceDisplay(int gold, int elixir) {
  // 更新标签显示
  _goldLabel->setString(StringUtils::format("Gold: %d", gold));
  _elixirLabel->setString(StringUtils::format("Elixir: %d", elixir));
}