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

  // ========== 核心修复1:设置 Node 的位置和锚点(和升级按钮完全一样) ==========
  auto visibleSize = Director::getInstance()->getVisibleSize();

  // 设置整个 Node 的位置(和 _actionMenuNode 一样)
  this->setPosition(Vec2(visibleSize.width / 2, 100.0f));

  // 设置锚点为中心点(这样缩放时会从中心向外扩展)
  this->setAnchorPoint(Vec2(0.5f, 0.5f));
  // =========================================================================

  initButtons();

  // 默认隐藏
  this->setVisible(false);

  return true;
}

void PlacementConfirmUI::initButtons() {
  // ========== 核心修复2:按钮位置改为相对于 Node 中心的偏移 ==========
  // 原来:按钮位置是绝对屏幕坐标
  // 现在:按钮位置是相对于 Node (0,0) 的偏移
  float buttonSpacing = 80.0f;  // 按钮间距

  // 确认按钮(绿勾) - 在右侧
  _confirmBtn = Button::create("ImageElements/right.png");
  if (_confirmBtn) {
    _confirmBtn->setScale(0.8f);
    _confirmBtn->setPosition(Vec2(buttonSpacing, 0));  // 相对于 Node 中心
    _confirmBtn->addClickEventListener([this](Ref*) {
      onConfirmClicked();
    });
    this->addChild(_confirmBtn);
  } else {
    CCLOG("PlacementConfirmUI: ERROR - Failed to load right.png");
  }

  // 取消按钮(红叉) - 在左侧
  _cancelBtn = Button::create("ImageElements/wrong.png");
  if (_cancelBtn) {
    _cancelBtn->setScale(0.8f);
    _cancelBtn->setPosition(Vec2(-buttonSpacing, 0));  // 相对于 Node 中心
    _cancelBtn->addClickEventListener([this](Ref*) {
      onCancelClicked();
    });
    this->addChild(_cancelBtn);
  } else {
    CCLOG("PlacementConfirmUI: ERROR - Failed to load wrong.png");
  }
  // =========================================================================
}

void PlacementConfirmUI::setConfirmCallback(ConfirmCallback callback) {
  _confirmCallback = callback;
}

void PlacementConfirmUI::setCancelCallback(CancelCallback callback) {
  _cancelCallback = callback;
}

void PlacementConfirmUI::show() {
  // ========== 核心修复：停止之前的动画，避免 hide() 覆盖 show() ==========
  this->stopAllActions();  // 停止之前的 hide() 动画

  this->setVisible(true);
  this->setScale(0.1f);
  this->runAction(EaseBackOut::create(ScaleTo::create(0.2f, 1.0f)));
}

void PlacementConfirmUI::hide() {
  // 缩小消失
  this->runAction(Sequence::create(
    ScaleTo::create(0.1f, 0.1f),
    Hide::create(),
    nullptr
  ));
}

void PlacementConfirmUI::updateButtonState(bool canPlace) {
  _canPlace = canPlace;

  if (_confirmBtn) {
    // 根据能否放置，确认按钮置灰或启用
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