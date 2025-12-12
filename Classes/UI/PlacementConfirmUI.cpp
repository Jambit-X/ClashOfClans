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
  initButtons();

  // 默认隐藏
  this->setVisible(false);

  return true;
}

void PlacementConfirmUI::initButtons() {
  auto visibleSize = Director::getInstance()->getVisibleSize();

  // 确认按钮（勾）
  _confirmBtn = Button::create("ImageElements/right.png");
  if (_confirmBtn) {
    _confirmBtn->setScale(0.8f);
    _confirmBtn->setPosition(Vec2(visibleSize.width / 2 + 80, 150));
    _confirmBtn->addClickEventListener([this](Ref*) {
      onConfirmClicked();
    });
    this->addChild(_confirmBtn);
  } else {
    CCLOG("PlacementConfirmUI: ERROR - Failed to load right.png");
  }

  // 取消按钮（叉）
  _cancelBtn = Button::create("ImageElements/wrong.png");
  if (_cancelBtn) {
    _cancelBtn->setScale(0.8f);
    _cancelBtn->setPosition(Vec2(visibleSize.width / 2 - 80, 150));
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
  this->setVisible(true);

  // 弹入动画
  this->setScale(0.1f);
  this->runAction(EaseBackOut::create(ScaleTo::create(0.2f, 1.0f)));
}

void PlacementConfirmUI::hide() {
  // 缩小动画
  this->runAction(Sequence::create(
    ScaleTo::create(0.1f, 0.1f),
    Hide::create(),
    nullptr
  ));
}

void PlacementConfirmUI::updateButtonState(bool canPlace) {
  _canPlace = canPlace;

  if (_confirmBtn) {
    // 如果不能放置，确认按钮变灰并禁用
    _confirmBtn->setEnabled(canPlace);
    _confirmBtn->setOpacity(canPlace ? 255 : 128);
  }
}

void PlacementConfirmUI::onConfirmClicked() {
  if (!_canPlace) {
    // 显示错误提示
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto label = Label::createWithTTF("此位置无法放置建筑！", "fonts/simhei.ttf", 28);
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