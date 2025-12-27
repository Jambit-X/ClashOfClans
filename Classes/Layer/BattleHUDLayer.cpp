// BattleHUDLayer.cpp
// 战斗HUD层实现，显示战斗界面的UI元素（倒计时、资源、兵种栏、按钮等）

#pragma execution_character_set("utf-8")
#include "BattleHUDLayer.h"
#include "Manager/VillageDataManager.h"
#include "Model/TroopConfig.h"

USING_NS_CC;
using namespace ui;

const std::string FONT_PATH = "fonts/simhei.ttf";

bool BattleHUDLayer::init() {
    if (!Layer::init()) return false;

    initTopInfo();
    initBottomButtons();
    initTroopBar();

    return true;
}

void BattleHUDLayer::initTopInfo() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 倒计时背景
    auto timerBg = LayerColor::create(Color4B(0, 0, 0, 100), 120, 40);
    timerBg->setPosition(visibleSize.width / 2 - 60, visibleSize.height - 50);
    this->addChild(timerBg);

    _timerLabel = Label::createWithTTF("30s", FONT_PATH, 24);
    _timerLabel->setPosition(60, 20);
    timerBg->addChild(_timerLabel);

    // 资源掠夺信息 - 图标+数值格式
    // 金币图标
    _goldIcon = Sprite::create("ImageElements/coin_icon.png");
    if (_goldIcon) {
        _goldIcon->setScale(0.4f);
        _goldIcon->setAnchorPoint(Vec2(1, 0.5f));
        _goldIcon->setPosition(Vec2(origin.x + 50, origin.y + visibleSize.height - 30));
        this->addChild(_goldIcon);
    }
    
    // 金币数值标签
    _goldLabel = Label::createWithTTF("0/0", "fonts/Marker Felt.ttf", 22);
    _goldLabel->setAnchorPoint(Vec2(0, 0.5f));
    _goldLabel->setPosition(Vec2(origin.x + 55, origin.y + visibleSize.height - 30));
    _goldLabel->setColor(Color3B(255, 215, 0));
    _goldLabel->enableOutline(Color4B::BLACK, 2);
    this->addChild(_goldLabel);

    // 圣水图标
    _elixirIcon = Sprite::create("ImageElements/elixir_icon.png");
    if (_elixirIcon) {
        _elixirIcon->setScale(0.4f);
        _elixirIcon->setAnchorPoint(Vec2(1, 0.5f));
        _elixirIcon->setPosition(Vec2(origin.x + 200, origin.y + visibleSize.height - 30));
        this->addChild(_elixirIcon);
    }
    
    // 圣水数值标签
    _elixirLabel = Label::createWithTTF("0/0", "fonts/Marker Felt.ttf", 22);
    _elixirLabel->setAnchorPoint(Vec2(0, 0.5f));
    _elixirLabel->setPosition(Vec2(origin.x + 205, origin.y + visibleSize.height - 30));
    _elixirLabel->setColor(Color3B(255, 0, 255));
    _elixirLabel->enableOutline(Color4B::BLACK, 2);
    this->addChild(_elixirLabel);
}

void BattleHUDLayer::initBottomButtons() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // [寻找对手]按钮 - 右下角
    _btnNext = Button::create("UI/battle/battle-prepare/next-icon.png");
    _btnNext->setScale(0.6f);
    _btnNext->setAnchorPoint(Vec2(1, 0));
    _btnNext->setPosition(Vec2(visibleSize.width - 20, 20));
    _btnNext->setZoomScale(-0.05f);

    _btnNext->addClickEventListener([this](Ref*) {
        if (auto scene = getBattleScene()) {
            scene->onNextOpponentClicked();
        }
        });
    this->addChild(_btnNext);

    // [回营]按钮（绿色）- 侦查阶段使用，左下角
    _btnReturn = Button::create("UI/battle/battle-prepare/back.png");
    _btnReturn->setScale(0.5f);
    _btnReturn->setAnchorPoint(Vec2(0, 0));
    _btnReturn->setPosition(Vec2(20, 20));

    _btnReturn->addClickEventListener([this](Ref*) {
        if (auto scene = getBattleScene()) scene->onReturnHomeClicked();
        });
    this->addChild(_btnReturn);

    // [结束战斗]按钮（红色）- 战斗阶段使用，左下角，初始隐藏
    _btnEnd = Button::create("UI/battle/battle-prepare/finishbattle.png");
    _btnEnd->setScale(0.3f);
    _btnEnd->setAnchorPoint(Vec2(0, 0));
    _btnEnd->setPosition(Vec2(20, 20));
    _btnEnd->setVisible(false);

    _btnEnd->addClickEventListener([this](Ref*) {
        if (auto scene = getBattleScene()) scene->onEndBattleClicked();
        });
    this->addChild(_btnEnd);
}

void BattleHUDLayer::initTroopBar() {
    _troopBarNode = Node::create();
    _troopBarNode->setPosition(0, 0);
    this->addChild(_troopBarNode);

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto dataManager = VillageDataManager::getInstance();
    auto troops = dataManager->getAllTroops();
    
    // 计算总宽度以便居中
    const float CARD_SPACING = 80.0f;
    int validTroopCount = 0;
    for (auto& pair : troops) {
     if (pair.second > 0) validTroopCount++;
    }
    
    float totalWidth = validTroopCount * CARD_SPACING;
    float startX = (visibleSize.width - totalWidth) / 2.0f + CARD_SPACING / 2.0f;

    for (auto& pair : troops) {
        int troopId = pair.first;
      int count = pair.second;
        if (count <= 0) continue;

        auto info = TroopConfig::getInstance()->getTroopById(troopId);

        // 使用Button替代普通精灵
        auto btn = ui::Button::create(info.iconPath);
        if (btn) {
            btn->setScale(0.5f);
  btn->setPosition(Vec2(startX, 60));
            // 添加点击事件
     btn->addClickEventListener([this, troopId](Ref*) {
         this->onTroopSelected(troopId);
                });

          _troopBarNode->addChild(btn);
    _troopButtons[troopId] = btn;

      // 数量标签 - 放在卡片上方
   auto numLabel = Label::createWithTTF(StringUtils::format("x%d", count), "fonts/simhei.ttf", 28);
       numLabel->setPosition(Vec2(btn->getContentSize().width / 2, btn->getContentSize().height + 15));
   numLabel->setColor(Color3B::WHITE);
            numLabel->enableOutline(Color4B::BLACK, 2);
          btn->addChild(numLabel);
            _troopCountLabels[troopId] = numLabel;

            // 等级标签 - 放在卡片左下角
            int level = dataManager->getTroopLevel(troopId);
            auto levelLabel = Label::createWithTTF(StringUtils::format("Lv.%d", level), "fonts/simhei.ttf", 24);
            levelLabel->setAnchorPoint(Vec2(0, 0));
            levelLabel->setPosition(Vec2(5, 5));
            levelLabel->setColor(Color3B::WHITE);
            levelLabel->enableOutline(Color4B::BLACK, 3);
            btn->addChild(levelLabel);

   startX += CARD_SPACING;
        }
    }
}

void BattleHUDLayer::onTroopSelected(int troopId) {
    // 如果点击已选中的，取消选中
    if (_selectedTroopId == troopId) {
        _selectedTroopId = -1;
        if (_troopButtons.count(troopId)) {
            _troopButtons[troopId]->runAction(ScaleTo::create(0.1f, 0.5f));
            _troopButtons[troopId]->setColor(Color3B::WHITE);
        }
        return;
    }

    // 还原之前的选中状态
    if (_selectedTroopId != -1 && _troopButtons.count(_selectedTroopId)) {
        _troopButtons[_selectedTroopId]->runAction(ScaleTo::create(0.1f, 0.5f));
        _troopButtons[_selectedTroopId]->setColor(Color3B::WHITE);
    }

    // 高亮当前选中的
    _selectedTroopId = troopId;
    if (_troopButtons.count(troopId)) {
        _troopButtons[troopId]->runAction(ScaleTo::create(0.1f, 0.6f));
        _troopButtons[troopId]->setColor(Color3B(200, 255, 200));
    }
}

void BattleHUDLayer::clearTroopSelection() {
    if (_selectedTroopId != -1 && _troopButtons.count(_selectedTroopId)) {
        _troopButtons[_selectedTroopId]->runAction(ScaleTo::create(0.1f, 0.5f));
        _troopButtons[_selectedTroopId]->setColor(Color3B::WHITE);
    }
    _selectedTroopId = -1;
    CCLOG("BattleHUDLayer: Troop selection cleared");
}

void BattleHUDLayer::updatePhase(BattleScene::BattleState state) {
    if (state == BattleScene::BattleState::PREPARE) {
        // 侦查阶段
        if (_btnNext) _btnNext->setVisible(true);
        if (_btnReturn) _btnReturn->setVisible(true);
        if (_btnEnd) _btnEnd->setVisible(false);
        if (_timerLabel) _timerLabel->setColor(Color3B::WHITE);
    }
    else if (state == BattleScene::BattleState::FIGHTING) {
        // 战斗阶段
        if (_btnNext) _btnNext->setVisible(false);
        if (_btnReturn) _btnReturn->setVisible(false);
        if (_btnEnd) _btnEnd->setVisible(true);
        if (_timerLabel) _timerLabel->setColor(Color3B::RED);
    }
    else if (state == BattleScene::BattleState::RESULT) {
        // 结算阶段
        if (_btnNext) _btnNext->setVisible(false);
        if (_btnReturn) _btnReturn->setVisible(false);
        if (_btnEnd) _btnEnd->setVisible(false);
        if (_troopBarNode) _troopBarNode->setVisible(false);
    }
}

void BattleHUDLayer::setButtonsEnabled(bool enabled) {
    if (_btnNext) _btnNext->setEnabled(enabled);
    if (_btnEnd) _btnEnd->setEnabled(enabled);
    if (_btnReturn) _btnReturn->setEnabled(enabled);
}

void BattleHUDLayer::updateTimer(int seconds) {
    if (_timerLabel) {
        _timerLabel->setString(StringUtils::format("%ds", seconds));
    }
}

BattleScene* BattleHUDLayer::getBattleScene() {
    return dynamic_cast<BattleScene*>(this->getScene());
}

void BattleHUDLayer::updateTroopCount(int troopId, int newCount) {
    // 更新数量标签
    if (_troopCountLabels.count(troopId)) {
        auto label = _troopCountLabels[troopId];
        label->setString(StringUtils::format("x%d", newCount));
        
        if (newCount <= 0) {
            label->setColor(Color3B::GRAY);
        }
    }
    
    // 更新按钮状态
    if (_troopButtons.count(troopId)) {
        auto btn = _troopButtons[troopId];
        
        if (newCount <= 0) {
            btn->setColor(Color3B(100, 100, 100));
            btn->setEnabled(false);
            
            if (_selectedTroopId == troopId) {
                _selectedTroopId = -1;
            }
            
            CCLOG("BattleHUDLayer: Troop %d depleted, button disabled", troopId);
        }
    }
}

void BattleHUDLayer::initLootDisplay(int totalGold, int totalElixir) {
    if (_goldLabel) {
        _goldLabel->setString(StringUtils::format("0/%d", totalGold));
    }
    if (_elixirLabel) {
        _elixirLabel->setString(StringUtils::format("0/%d", totalElixir));
    }
    CCLOG("BattleHUDLayer: Loot display initialized - Gold: 0/%d, Elixir: 0/%d", totalGold, totalElixir);
}

void BattleHUDLayer::updateLootDisplay(int lootedGold, int lootedElixir, 
                                        int totalGold, int totalElixir) {
    if (_goldLabel) {
        _goldLabel->setString(StringUtils::format("%d/%d", lootedGold, totalGold));
    }
    if (_elixirLabel) {
        _elixirLabel->setString(StringUtils::format("%d/%d", lootedElixir, totalElixir));
    }
}

void BattleHUDLayer::hideReplayControls() {
    // 隐藏兵种栏
    if (_troopBarNode) {
        _troopBarNode->setVisible(false);
        CCLOG("BattleHUDLayer: Troop bar hidden");
    }

    // 隐藏各个按钮
    if (_btnNext) {
        _btnNext->setVisible(false);
        CCLOG("BattleHUDLayer: Next button hidden");
    }

    if (_btnReturn) {
        _btnReturn->setVisible(false);
        CCLOG("BattleHUDLayer: Return button hidden");
    }

    if (_btnEnd) {
        _btnEnd->setVisible(false);
        CCLOG("BattleHUDLayer: End battle button hidden");
    }

    // 显示"回放中"标识
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto replayLabel = Label::createWithTTF("回放中...", "fonts/simhei.ttf", 32);
    replayLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 50));
    replayLabel->setColor(Color3B::YELLOW);
    replayLabel->enableOutline(Color4B::BLACK, 2);
    replayLabel->setTag(9999);
    this->addChild(replayLabel, 100);

    CCLOG("BattleHUDLayer: Replay controls hidden successfully");
}
