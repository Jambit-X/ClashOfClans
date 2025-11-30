#include "HUDLayer.h"

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

  return true;
}

void HUDLayer::updateResourceDisplay(int gold, int elixir) {
  // 更新标签显示
  _goldLabel->setString(StringUtils::format("Gold: %d", gold));
  _elixirLabel->setString(StringUtils::format("Elixir: %d", elixir));
}