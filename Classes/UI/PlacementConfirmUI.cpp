// PlacementConfirmUI.cpp
// 建筑放置确认UI实现，管理确认和取消按钮的交互

#pragma execution_character_set("utf-8")
#include "PlacementConfirmUI.h"

USING_NS_CC;
using namespace ui;

PlacementConfirmUI* PlacementConfirmUI::create() {
  auto ret = new PlacementConfirmUI();
  if (ret && ret->init()) {
    ret->autorelease();
    return ret;
  }
  delete ret;
  return nullptr;
}

bool PlacementConfirmUI::init() {
  if (!Node::init()) {
    return false;
  }

  _canPlace = true;

  // 设置Node位置和锚点，确保缩放动画从中心扩展
  auto visibleSize = Director::getInstance()->getVisibleSize();
  this->setPosition(Vec2(visibleSize.width / 2, 100.0f));
  this->setAnchorPoint(Vec2(0.5f, 0.5f));

  initButtons();

  this->setVisible(false);

  return true;
}

void PlacementConfirmUI::initButtons() {
  float buttonSpacing = 80.0f;

  // 创建确认按钮(绿勾)
  _confirmBtn = Button::create("ImageElements/right.png");
  if (_confirmBtn) {
    _confirmBtn->setScale(0.8f);
    _confirmBtn->setPosition(Vec2(buttonSpacing, 0));
    _confirmBtn->addClickEventListener([this](Ref*) {
      onConfirmClicked();
    });
    this->addChild(_confirmBtn);
  } else {
    CCLOG("PlacementConfirmUI: ERROR - Failed to load right.png");
  }

  // 创建取消按钮(红叉)
  _cancelBtn = Button::create("ImageElements/wrong.png");
  if (_cancelBtn) {
    _cancelBtn->setScale(0.8f);
    _cancelBtn->setPosition(Vec2(-buttonSpacing, 0));
    _cancelBtn->addClickEventListener([this](Ref*) {
      onCancelClicked();
    });
    this->addChild(_cancelBtn);
  } else {
    CCLOG("PlacementConfirmUI: ERROR - Failed to load wrong.png");
  }
}

void PlacementConfirmUI::setConfirmCallback(ConfirmCallback callback) {
  _confirmCallback = callback;
}

void PlacementConfirmUI::setCancelCallback(CancelCallback callback) {
  _cancelCallback = callback;
}

void PlacementConfirmUI::show() {
  // 停止之前的动画，避免hide()覆盖show()
  this->stopAllActions();

  this->setVisible(true);
  this->setScale(0.1f);
  this->runAction(EaseBackOut::create(ScaleTo::create(0.2f, 1.0f)));
}

void PlacementConfirmUI::hide() {
  this->runAction(Sequence::create(
    ScaleTo::create(0.1f, 0.1f),
    Hide::create(),
    nullptr
  ));
}

void PlacementConfirmUI::updateButtonState(bool canPlace) {
  _canPlace = canPlace;

  if (_confirmBtn) {
    // 不可放置时确认按钮置灰
    _confirmBtn->setEnabled(canPlace);
    _confirmBtn->setOpacity(canPlace ? 255 : 128);
  }
}

void PlacementConfirmUI::onConfirmClicked() {
  if (!_canPlace) {
    // 显示错误提示
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto label = Label::createWithTTF("该位置无法放置建筑！", "fonts/simhei.ttf", 28);
    label->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    label->setColor(Color3B::RED);
    this->getParent()->addChild(label, 1000);

    label->runAction(Sequence::create(
      DelayTime::create(1.5f),
      FadeOut::create(0.5f),
      RemoveSelf::create(),
      nullptr
    ));

    return;
  }

  if (_confirmCallback) {
    _confirmCallback();
  }
}

void PlacementConfirmUI::onCancelClicked() {
  if (_cancelCallback) {
    _cancelCallback();
  }
}
