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

  // ? 初始化有效标志
  _isValid = std::make_shared<bool>(true);

  initButtons();

  // ? 修复：不捕获 this，而是捕获 shared_ptr 并通过它来访问成员
  auto productionSystem = ResourceProductionSystem::getInstance();
  
  // 捕获成员变量的 shared_ptr 引用，而不是 this
  std::weak_ptr<bool> weakValid = _isValid;
  cocos2d::Label* goldLabel = _pendingGoldLabel;  // 捕获原始指针
  cocos2d::Label* elixirLabel = _pendingElixirLabel;
  cocos2d::ui::Button* goldBtn = _collectGoldBtn;
  cocos2d::ui::Button* elixirBtn = _collectElixirBtn;
  
  productionSystem->setPendingResourceCallback([weakValid, goldLabel, elixirLabel, goldBtn, elixirBtn](int gold, int elixir) {
    // ? 检查对象是否仍然有效
    auto valid = weakValid.lock();
  if (!valid || !*valid) {
    CCLOG("ResourceCollectionUI: Callback ignored - object already destroyed");
    return;
    }
  
    // ? 在访问 Cocos2d 对象前检查它们是否还存在于渲染树中
    // 如果对象已经被 removeFromParent()，getReferenceCount() 可能为 0
    if (!goldLabel || !elixirLabel || !goldBtn || !elixirBtn) {
      CCLOG("ResourceCollectionUI: Callback ignored - UI elements null");
      return;
    }
    
    // ? 再次检查引用计数，确保对象仍然有效
    if (goldLabel->getReferenceCount() == 0 || elixirLabel->getReferenceCount() == 0) {
      CCLOG("ResourceCollectionUI: Callback ignored - UI elements released");
      return;
    }
    
    // 安全地更新显示（内联 updateDisplay 的逻辑以避免调用 this）
    auto prodSystem = ResourceProductionSystem::getInstance();
    if (!prodSystem) return;

    int goldCapacity = prodSystem->getGoldStorageCapacity();
    int elixirCapacity = prodSystem->getElixirStorageCapacity();

 // 更新金币显示
    if (goldLabel && goldLabel->getReferenceCount() > 0) {
  goldLabel->setString(cocos2d::StringUtils::format("+%d/%d", gold, goldCapacity));

      float goldRatio = (float)gold / goldCapacity;
      if (goldRatio >= 1.0f) {
        goldLabel->setColor(cocos2d::Color3B::RED);
      } else if (goldRatio >= 0.8f) {
        goldLabel->setColor(cocos2d::Color3B::ORANGE);
      } else {
        goldLabel->setColor(cocos2d::Color3B::YELLOW);
      }

      if (gold > 0 && goldRatio < 1.0f) {
        goldLabel->stopAllActions();
        goldLabel->runAction(cocos2d::RepeatForever::create(
      cocos2d::Sequence::create(
            cocos2d::FadeOut::create(0.8f),
      cocos2d::FadeIn::create(0.8f),
            nullptr
          )
        ));
        if (goldBtn) goldBtn->setEnabled(true);
      } else {
        goldLabel->stopAllActions();
        goldLabel->setOpacity(255);
        if (goldBtn) goldBtn->setEnabled(gold > 0);
      }
  }

    // 更新圣水显示
    if (elixirLabel && elixirLabel->getReferenceCount() > 0) {
      elixirLabel->setString(cocos2d::StringUtils::format("+%d/%d", elixir, elixirCapacity));

      float elixirRatio = (float)elixir / elixirCapacity;
      if (elixirRatio >= 1.0f) {
        elixirLabel->setColor(cocos2d::Color3B::RED);
      } else if (elixirRatio >= 0.8f) {
        elixirLabel->setColor(cocos2d::Color3B::ORANGE);
    } else {
      elixirLabel->setColor(cocos2d::Color3B::MAGENTA);
      }

      if (elixir > 0 && elixirRatio < 1.0f) {
        elixirLabel->stopAllActions();
        elixirLabel->runAction(cocos2d::RepeatForever::create(
          cocos2d::Sequence::create(
    cocos2d::FadeOut::create(0.8f),
     cocos2d::FadeIn::create(0.8f),
      nullptr
          )
        ));
   if (elixirBtn) elixirBtn->setEnabled(true);
      } else {
        elixirLabel->stopAllActions();
        elixirLabel->setOpacity(255);
    if (elixirBtn) elixirBtn->setEnabled(elixir > 0);
      }
    }
  });

  // 初始化显示
  updateDisplay(productionSystem->getPendingGold(), productionSystem->getPendingElixir());

  return true;
}

void ResourceCollectionUI::onExit() {
  CCLOG("ResourceCollectionUI::onExit - Cleaning up");
  
  // ? 标记对象已无效 - 这会阻止回调继续执行
  if (_isValid) {
    *_isValid = false;
    CCLOG("ResourceCollectionUI::onExit - Marked as invalid");
  }
  
  // ? 清除所有动画，避免回调在动画中触发
  if (_pendingGoldLabel) {
    _pendingGoldLabel->stopAllActions();
  }
  if (_pendingElixirLabel) {
    _pendingElixirLabel->stopAllActions();
  }
  
  // ? 清除回调，防止对象销毁后仍被调用
  // 注意：这里可能有竞态条件，如果新的 ResourceCollectionUI 还没设置回调
  // 所以我们先检查当前回调是否是"我们的"
  auto productionSystem = ResourceProductionSystem::getInstance();
  if (productionSystem) {
    // 为了安全起见，直接设为 nullptr
    // 新的 ResourceCollectionUI 会在 init() 中重新设置回调
    productionSystem->setPendingResourceCallback(nullptr);
    CCLOG("ResourceCollectionUI::onExit - Cleared callback");
  }
  
  Node::onExit();
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

    // ? 修复：飘字效果 - 检查父节点有效性
    auto parent = this->getParent();
    if (!parent) {
      CCLOG("ResourceCollectionUI::onCollectGold - Parent is null, skip floating text");
      return;
    }
    
    auto label = Label::createWithTTF("+" + std::to_string(pendingGold), FONT_PATH, 30);
    label->setColor(Color3B::YELLOW);
    label->enableOutline(Color4B::BLACK, 2);
    label->setPosition(_collectGoldBtn->getPosition() + Vec2(0, 40));
    parent->addChild(label, 100);

    // ? 使用 retain/release 保护动画执行
    label->retain();
    label->runAction(Sequence::create(
      DelayTime::create(3.0f),
      Spawn::create(
        MoveBy::create(2.0f, Vec2(0, 100)),
        FadeOut::create(2.0f),
        nullptr
      ),
      CallFunc::create([label]() {
     label->removeFromParent();
        label->release();  // ← 释放引用
      }),
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

    // ? 修复：飘字效果 - 检查父节点有效性
    auto parent = this->getParent();
    if (!parent) {
      CCLOG("ResourceCollectionUI::onCollectElixir - Parent is null, skip floating text");
      return;
    }
    
    auto label = Label::createWithTTF("+" + std::to_string(pendingElixir), FONT_PATH, 30);
  label->setColor(Color3B::MAGENTA);
    label->enableOutline(Color4B::BLACK, 2);
    label->setPosition(_collectElixirBtn->getPosition() + Vec2(0, 40));
    parent->addChild(label, 100);

    // ? 使用 retain/release 保护动画执行
    label->retain();
    label->runAction(Sequence::create(
      DelayTime::create(3.0f),
      Spawn::create(
        MoveBy::create(2.0f, Vec2(0, 100)),
     FadeOut::create(2.0f),
        nullptr
      ),
      CallFunc::create([label]() {
        label->removeFromParent();
        label->release();  // ← 释放引用
      }),
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