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

    // 倒计时背景
    auto timerBg = LayerColor::create(Color4B(0, 0, 0, 100), 120, 40);
    timerBg->setPosition(visibleSize.width / 2 - 60, visibleSize.height - 50);
    this->addChild(timerBg);

    _timerLabel = Label::createWithTTF("30s", FONT_PATH, 24);
    _timerLabel->setPosition(60, 20);
    timerBg->addChild(_timerLabel);

    // 资源掠夺信息
    auto lootLabel = Label::createWithTTF("可掠夺: 金币 1000  圣水 1000", FONT_PATH, 20);
    lootLabel->setAnchorPoint(Vec2(0, 1));
    lootLabel->setPosition(20, visibleSize.height - 20);
    lootLabel->enableOutline(Color4B::BLACK, 1);
    this->addChild(lootLabel);
}

void BattleHUDLayer::initBottomButtons() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // [寻找对手] 按钮 - 使用您提供的图片
    _btnNext = Button::create("UI/battle/battle-prepare/next-icon.png");
    _btnNext->setScale(0.6f);
    // 放在右下角
    _btnNext->setAnchorPoint(Vec2(1, 0));
    _btnNext->setPosition(Vec2(visibleSize.width - 20, 20));
    _btnNext->setZoomScale(-0.05f); // 点击时的缩放效果

    _btnNext->addClickEventListener([this](Ref*) {
        if (auto scene = getBattleScene()) {
            scene->onNextOpponentClicked();
        }
        });
    this->addChild(_btnNext);

    // 2. [回营] 按钮 (绿色 back.png) - 用于侦查阶段
    // 放在左下角
    _btnReturn = Button::create("UI/battle/battle-prepare/back.png");
    _btnReturn->setScale(0.5f); // 适当缩放
    _btnReturn->setAnchorPoint(Vec2(0, 0));
    _btnReturn->setPosition(Vec2(20, 20));

    _btnReturn->addClickEventListener([this](Ref*) {
        // 调用 Scene 的回营逻辑
        if (auto scene = getBattleScene()) scene->onReturnHomeClicked();
        });
    this->addChild(_btnReturn);

    // 3. [结束战斗] 按钮 (红色 finishbattle.png) - 用于战斗阶段
    // 同样放在左下角，初始隐藏
    _btnEnd = Button::create("UI/battle/battle-prepare/finishbattle.png");
    _btnEnd->setScale(0.3f);
    _btnEnd->setAnchorPoint(Vec2(0, 0));
    _btnEnd->setPosition(Vec2(20, 20)); // 位置与回营按钮重叠
    _btnEnd->setVisible(false); // 默认隐藏

    _btnEnd->addClickEventListener([this](Ref*) {
        // 调用 Scene 的结束战斗逻辑 (弹出结算)
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
    float startX = (visibleSize.width - totalWidth) / 2.0f + CARD_SPACING / 2.0f; // 居中起始位置

    for (auto& pair : troops) {
        int troopId = pair.first;
      int count = pair.second;
        if (count <= 0) continue;

        auto info = TroopConfig::getInstance()->getTroopById(troopId);

        // 改用 Button
        auto btn = ui::Button::create(info.iconPath);
        if (btn) {
            btn->setScale(0.5f);
  btn->setPosition(Vec2(startX, 60));
            // 添加点击事件
     btn->addClickEventListener([this, troopId](Ref*) {
         this->onTroopSelected(troopId);
                });

          _troopBarNode->addChild(btn);
    _troopButtons[troopId] = btn; // 存入 Map

      // 数量标签 - 放在卡片上方，使用白色大字体
   auto numLabel = Label::createWithTTF(StringUtils::format("x%d", count), "fonts/simhei.ttf", 28);
       numLabel->setPosition(Vec2(btn->getContentSize().width / 2, btn->getContentSize().height + 15));
   numLabel->setColor(Color3B::WHITE);
            numLabel->enableOutline(Color4B::BLACK, 2);
          btn->addChild(numLabel);

   startX += CARD_SPACING;
        }
    }
}

// 新增方法实现
void BattleHUDLayer::onTroopSelected(int troopId) {
    // 1. 如果点击已选中的，则取消选中
    if (_selectedTroopId == troopId) {
        _selectedTroopId = -1;
        if (_troopButtons.count(troopId)) {
            _troopButtons[troopId]->runAction(ScaleTo::create(0.1f, 0.5f)); // 恢复大小
            _troopButtons[troopId]->setColor(Color3B::WHITE); // 恢复颜色
        }
        return;
    }

    // 2. 还原之前的选中状态
    if (_selectedTroopId != -1 && _troopButtons.count(_selectedTroopId)) {
        _troopButtons[_selectedTroopId]->runAction(ScaleTo::create(0.1f, 0.5f));
        _troopButtons[_selectedTroopId]->setColor(Color3B::WHITE);
    }

    // 3. 高亮当前选中的
    _selectedTroopId = troopId;
    if (_troopButtons.count(troopId)) {
        _troopButtons[troopId]->runAction(ScaleTo::create(0.1f, 0.6f)); // 放大
        _troopButtons[troopId]->setColor(Color3B(200, 255, 200)); // 变绿高亮
    }
}

// 【新增】清除兵种选择状态
void BattleHUDLayer::clearTroopSelection() {
    if (_selectedTroopId != -1 && _troopButtons.count(_selectedTroopId)) {
        // 恢复之前选中的按钮状态
        _troopButtons[_selectedTroopId]->runAction(ScaleTo::create(0.1f, 0.5f));
        _troopButtons[_selectedTroopId]->setColor(Color3B::WHITE);
    }
    _selectedTroopId = -1;
    CCLOG("BattleHUDLayer: Troop selection cleared");
}

void BattleHUDLayer::updatePhase(BattleScene::BattleState state) {
    if (state == BattleScene::BattleState::PREPARE) {
        // --- 侦查阶段 ---
        // 显示 [下一个]
        if (_btnNext) _btnNext->setVisible(true);

        // 显示绿色 [回营]
        if (_btnReturn) _btnReturn->setVisible(true);
        // 隐藏红色 [结束战斗]
        if (_btnEnd) _btnEnd->setVisible(false);

        // 倒计时白色
        if (_timerLabel) _timerLabel->setColor(Color3B::WHITE);
    }
    else if (state == BattleScene::BattleState::FIGHTING) {
        // --- 战斗阶段 ---
        // 隐藏 [下一个] (不能换人了)
        if (_btnNext) _btnNext->setVisible(false);

        // 隐藏绿色 [回营]
        if (_btnReturn) _btnReturn->setVisible(false);
        // 显示红色 [结束战斗]
        if (_btnEnd) _btnEnd->setVisible(true);

        // 倒计时变红
        if (_timerLabel) _timerLabel->setColor(Color3B::RED);
    }
    else if (state == BattleScene::BattleState::RESULT) {
        // --- 结算阶段 ---
        // 全部隐藏，交给结算界面处理
        if (_btnNext) _btnNext->setVisible(false);
        if (_btnReturn) _btnReturn->setVisible(false);
        if (_btnEnd) _btnEnd->setVisible(false);

        if (_troopBarNode) _troopBarNode->setVisible(false);
    }
}

//  控制所有按钮的点击状态 (防止云层动画时误触)
void BattleHUDLayer::setButtonsEnabled(bool enabled) {
    if (_btnNext) _btnNext->setEnabled(enabled);
    if (_btnEnd) _btnEnd->setEnabled(enabled);

    // 【重要】别忘了把新加的 _btnReturn 也加上
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