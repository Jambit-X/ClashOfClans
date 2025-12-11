#pragma execution_character_set("utf-8")
#include "ResourceCollectionUI.h"
#include "Manager/Resource/ResourceProductionSystem.h"

USING_NS_CC;
using namespace ui;

const std::string FONT_PATH = "fonts/simhei.ttf";

ResourceCollectionUI* ResourceCollectionUI::create() {
  auto ret = new ResourceCollectionUI();
  if (ret && ret->init()) {
    ret->autorelease();
    return ret;
  }
  delete ret;
  return nullptr;
}

bool ResourceCollectionUI::init() {
  if (!Node::init()) {
    return false;
  }

  initButtons();

  // 设置回调
  auto productionSystem = ResourceProductionSystem::getInstance();
  productionSystem->setPendingResourceCallback([this](int gold, int elixir) {
    updateDisplay(gold, elixir);
  });

  // 初始化显示
  updateDisplay(productionSystem->getPendingGold(), productionSystem->getPendingElixir());

  return true;
}

void ResourceCollectionUI::initButtons() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 金币收集按钮
  _collectGoldBtn = Button::create();
  _collectGoldBtn->setTitleText("收集金币");
  _collectGoldBtn->setTitleFontName(FONT_PATH);
  _collectGoldBtn->setTitleFontSize(18);
  _collectGoldBtn->setTitleColor(Color3B::YELLOW);
  _collectGoldBtn->ignoreContentAdaptWithSize(false);
  _collectGoldBtn->setContentSize(Size(100, 50));
  _collectGoldBtn->setPosition(Vec2(130, visibleSize.height - 70));

  auto goldBg = LayerColor::create(Color4B(100, 80, 0, 200), 100, 50);
  _collectGoldBtn->addChild(goldBg, -1);

  _collectGoldBtn->addClickEventListener([this](Ref*) { onCollectGold(); });
  this->addChild(_collectGoldBtn);

  _pendingGoldLabel = Label::createWithTTF("+0/500", FONT_PATH, 20);
  _pendingGoldLabel->setColor(Color3B::YELLOW);
  _pendingGoldLabel->enableOutline(Color4B::BLACK, 2);
  _pendingGoldLabel->setPosition(Vec2(130, visibleSize.height - 125));
  this->addChild(_pendingGoldLabel);

  // 圣水收集按钮
  _collectElixirBtn = Button::create();
  _collectElixirBtn->setTitleText("收集圣水");
  _collectElixirBtn->setTitleFontName(FONT_PATH);
  _collectElixirBtn->setTitleFontSize(18);
  _collectElixirBtn->setTitleColor(Color3B::MAGENTA);
  _collectElixirBtn->ignoreContentAdaptWithSize(false);
  _collectElixirBtn->setContentSize(Size(100, 50));
  _collectElixirBtn->setPosition(Vec2(330, visibleSize.height - 70));

  auto elixirBg = LayerColor::create(Color4B(100, 0, 100, 200), 100, 50);
  _collectElixirBtn->addChild(elixirBg, -1);

  _collectElixirBtn->addClickEventListener([this](Ref*) { onCollectElixir(); });
  this->addChild(_collectElixirBtn);

  _pendingElixirLabel = Label::createWithTTF("+0/500", FONT_PATH, 20);
  _pendingElixirLabel->setColor(Color3B::MAGENTA);
  _pendingElixirLabel->enableOutline(Color4B::BLACK, 2);
  _pendingElixirLabel->setPosition(Vec2(330, visibleSize.height - 125));
  this->addChild(_pendingElixirLabel);
}

void ResourceCollectionUI::onCollectGold() {
  auto productionSystem = ResourceProductionSystem::getInstance();
  int pendingGold = productionSystem->getPendingGold();

  if (pendingGold > 0) {
    productionSystem->collectGold();

    // 动画效果
    _collectGoldBtn->runAction(Sequence::create(
      ScaleTo::create(0.1f, 1.2f),
      ScaleTo::create(0.1f, 1.0f),
      nullptr
    ));

    // 飘字效果
    auto label = Label::createWithTTF("+" + std::to_string(pendingGold), FONT_PATH, 30);
    label->setColor(Color3B::YELLOW);
    label->enableOutline(Color4B::BLACK, 2);
    label->setPosition(_collectGoldBtn->getPosition() + Vec2(0, 40));
    this->getParent()->addChild(label, 100);

    label->runAction(Sequence::create(
      DelayTime::create(3.0f),
      Spawn::create(
      MoveBy::create(2.0f, Vec2(0, 100)),
      FadeOut::create(2.0f),
      nullptr
    ),
      RemoveSelf::create(),
      nullptr
    ));
  }
}

void ResourceCollectionUI::onCollectElixir() {
  auto productionSystem = ResourceProductionSystem::getInstance();
  int pendingElixir = productionSystem->getPendingElixir();

  if (pendingElixir > 0) {
    productionSystem->collectElixir();

    _collectElixirBtn->runAction(Sequence::create(
      ScaleTo::create(0.1f, 1.2f),
      ScaleTo::create(0.1f, 1.0f),
      nullptr
    ));

    auto label = Label::createWithTTF("+" + std::to_string(pendingElixir), FONT_PATH, 30);
    label->setColor(Color3B::MAGENTA);
    label->enableOutline(Color4B::BLACK, 2);
    label->setPosition(_collectElixirBtn->getPosition() + Vec2(0, 40));
    this->getParent()->addChild(label, 100);

    label->runAction(Sequence::create(
      DelayTime::create(3.0f),
      Spawn::create(
      MoveBy::create(2.0f, Vec2(0, 100)),
      FadeOut::create(2.0f),
      nullptr
    ),
      RemoveSelf::create(),
      nullptr
    ));
  }
}

void ResourceCollectionUI::updateDisplay(int pendingGold, int pendingElixir) {
  auto productionSystem = ResourceProductionSystem::getInstance();

  int goldCapacity = productionSystem->getGoldStorageCapacity();
  int elixirCapacity = productionSystem->getElixirStorageCapacity();

  if (_pendingGoldLabel) {
    _pendingGoldLabel->setString(StringUtils::format("+%d/%d", pendingGold, goldCapacity));

    float goldRatio = (float)pendingGold / goldCapacity;
    if (goldRatio >= 1.0f) {
      _pendingGoldLabel->setColor(Color3B::RED);
    } else if (goldRatio >= 0.8f) {
      _pendingGoldLabel->setColor(Color3B::ORANGE);
    } else {
      _pendingGoldLabel->setColor(Color3B::YELLOW);
    }

    if (pendingGold > 0 && goldRatio < 1.0f) {
      _pendingGoldLabel->stopAllActions();
      _pendingGoldLabel->runAction(RepeatForever::create(
        Sequence::create(
        FadeOut::create(0.8f),
        FadeIn::create(0.8f),
        nullptr
      )
      ));
      _collectGoldBtn->setEnabled(true);
    } else {
      _pendingGoldLabel->stopAllActions();
      _pendingGoldLabel->setOpacity(255);
      _collectGoldBtn->setEnabled(pendingGold > 0);
    }
  }

  if (_pendingElixirLabel) {
    _pendingElixirLabel->setString(StringUtils::format("+%d/%d", pendingElixir, elixirCapacity));

    float elixirRatio = (float)pendingElixir / elixirCapacity;
    if (elixirRatio >= 1.0f) {
      _pendingElixirLabel->setColor(Color3B::RED);
    } else if (elixirRatio >= 0.8f) {
      _pendingElixirLabel->setColor(Color3B::ORANGE);
    } else {
      _pendingElixirLabel->setColor(Color3B::MAGENTA);
    }

    if (pendingElixir > 0 && elixirRatio < 1.0f) {
      _pendingElixirLabel->stopAllActions();
      _pendingElixirLabel->runAction(RepeatForever::create(
        Sequence::create(
        FadeOut::create(0.8f),
        FadeIn::create(0.8f),
        nullptr
      )
      ));
      _collectElixirBtn->setEnabled(true);
    } else {
      _pendingElixirLabel->stopAllActions();
      _pendingElixirLabel->setOpacity(255);
      _collectElixirBtn->setEnabled(pendingElixir > 0);
    }
  }
}