#pragma execution_character_set("utf-8")
#include "LaboratoryLayer.h"
#include "Manager/VillageDataManager.h"
#include "Model/TroopUpgradeConfig.h"
#include "Model/BuildingConfig.h"

USING_NS_CC;
using namespace ui;

// 字体设置
const std::string LAB_FONT = "fonts/simhei.ttf";

// 界面尺寸配置（根据背景图 1216x911 的比例）
const float BG_WIDTH = 800.0f;
const float BG_HEIGHT = 600.0f;

// 兵种卡片尺寸（大幅度放大）
const float CARD_WIDTH = 180.0f;
const float CARD_HEIGHT = 235.0f;

Scene* LaboratoryLayer::createScene() {
    auto scene = Scene::create();
    auto layer = LaboratoryLayer::create();
    scene->addChild(layer);
    return scene;
}

bool LaboratoryLayer::init() {
    if (!Layer::init()) {
        return false;
    }

    // 1. 全屏半透明遮罩
    auto shieldLayer = LayerColor::create(Color4B(0, 0, 0, 180));
    this->addChild(shieldLayer);

    // 设置触摸吞噬
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch* t, Event* e) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 2. 初始化 UI
    initBackground();
    initTroopCards();

    // 3. 启动更新（用于倒计时）
    this->scheduleUpdate();

    return true;
}

void LaboratoryLayer::initBackground() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 主面板容器
    _bgNode = Node::create();
    _bgNode->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    this->addChild(_bgNode);

    // 背景图片
    auto bgSprite = Sprite::create("UI/laboratory/backgroud.png");
    if (bgSprite) {
        // 按比例缩放
        float scaleX = BG_WIDTH / bgSprite->getContentSize().width;
        float scaleY = BG_HEIGHT / bgSprite->getContentSize().height;
        float scale = std::min(scaleX, scaleY);
        bgSprite->setScale(scale);
        bgSprite->setPosition(0, 0);
        _bgNode->addChild(bgSprite, 0);

        // 根据背景图比例计算位置
        float bgDisplayWidth = bgSprite->getContentSize().width * scale;
        float bgDisplayHeight = bgSprite->getContentSize().height * scale;

        // 关闭按钮（覆盖在背景图右上角X按钮位置）
        auto closeBtn = Button::create();
        closeBtn->ignoreContentAdaptWithSize(false);
        closeBtn->setContentSize(Size(50, 50));
        closeBtn->setPosition(Vec2(bgDisplayWidth / 2 - 35, bgDisplayHeight / 2 - 30));
        closeBtn->addClickEventListener([=](Ref*) {
            this->onCloseClicked();
        });
        _bgNode->addChild(closeBtn, 10);

        // 立即完成区域（女角色右侧空白处）
        // 先添加"立即完成升级："文字标签
        auto finishLabel = Label::createWithTTF("立即完成升级：", LAB_FONT, 18);
        if (!finishLabel) finishLabel = Label::createWithSystemFont("立即完成升级：", "Arial", 18);
        finishLabel->setColor(Color3B::WHITE);
        finishLabel->setPosition(Vec2(bgDisplayWidth / 4, bgDisplayHeight / 4 + 10));
        finishLabel->setName("finishLabel");
        finishLabel->setVisible(false);  // 默认隐藏
        _bgNode->addChild(finishLabel, 10);
        
        // 立即完成按钮（在文字下方）
        _instantFinishBtn = Button::create();
        _instantFinishBtn->ignoreContentAdaptWithSize(false);
        _instantFinishBtn->setContentSize(Size(120, 40));
        // 位置放在文字下方
        _instantFinishBtn->setPosition(Vec2(bgDisplayWidth / 4, bgDisplayHeight / 4 - 25));
        
        // 按钮背景（绿色风格，参考游戏UI）
        auto btnBg = LayerColor::create(Color4B(80, 180, 80, 255), 120, 40);
        btnBg->setTouchEnabled(false);
        _instantFinishBtn->addChild(btnBg, -1);
        
        // 钻石图标
        auto gemIcon = Sprite::create("ImageElements/gem_icon.png");
        if (gemIcon) {
            gemIcon->setScale(0.28f);
            gemIcon->setPosition(90, 20);
            _instantFinishBtn->addChild(gemIcon, 1);
        }
        
        // 费用标签
        auto costLabel = Label::createWithTTF("10", LAB_FONT, 20);
        if (!costLabel) costLabel = Label::createWithSystemFont("10", "Arial", 20);
        costLabel->setPosition(45, 20);
        costLabel->setName("costLabel");
        costLabel->enableBold();
        _instantFinishBtn->addChild(costLabel, 1);
        
        _instantFinishBtn->addClickEventListener([=](Ref*) {
            this->onInstantFinishClicked();
        });
        _instantFinishBtn->setVisible(false);  // 默认隐藏，有研究时才显示
        _bgNode->addChild(_instantFinishBtn, 10);
    } else {
        // 如果背景图加载失败，用纯色代替
        auto panelBg = LayerColor::create(Color4B(180, 170, 160, 255), BG_WIDTH, BG_HEIGHT);
        panelBg->ignoreAnchorPointForPosition(false);
        panelBg->setAnchorPoint(Vec2(0.5f, 0.5f));
        panelBg->setPosition(0, 0);
        _bgNode->addChild(panelBg, 0);

        // 关闭按钮
        auto closeBtn = Button::create();
        closeBtn->ignoreContentAdaptWithSize(false);
        closeBtn->setContentSize(Size(50, 50));
        auto btnBg = LayerColor::create(Color4B(200, 50, 50, 255), 50, 50);
        btnBg->setTouchEnabled(false);
        closeBtn->addChild(btnBg, -1);
        closeBtn->setTitleText("X");
        closeBtn->setTitleFontSize(30);
        closeBtn->setPosition(Vec2(BG_WIDTH / 2 - 35, BG_HEIGHT / 2 - 35));
        closeBtn->addClickEventListener([=](Ref*) {
            this->onCloseClicked();
        });
        _bgNode->addChild(closeBtn, 10);
    }
}

void LaboratoryLayer::initTroopCards() {
    // 创建 ScrollView 放在背景图下方灰色区域
    _scrollView = ScrollView::create();
    _scrollView->setDirection(ScrollView::Direction::HORIZONTAL);
    _scrollView->setContentSize(Size(BG_WIDTH - 40, CARD_HEIGHT + 30));
    _scrollView->setAnchorPoint(Vec2(0.5f, 0.5f));
    // 放在背景下方区域（调整Y坐标以填满女角色下方空间）
    _scrollView->setPosition(Vec2(0, -BG_HEIGHT / 2 + 150));
    _scrollView->setScrollBarEnabled(false);
    _scrollView->setBounceEnabled(true);
    _bgNode->addChild(_scrollView, 5);

    refreshCards();
    updateInstantFinishButton();
}

void LaboratoryLayer::refreshCards() {
    _scrollView->removeAllChildren();

    auto dataManager = VillageDataManager::getInstance();
    auto troopConfig = TroopConfig::getInstance();
    auto allTroops = troopConfig->getAllTroops();

    float startX = 10;
    float padding = 10;

    for (const auto& troop : allTroops) {
        int currentLevel = dataManager->getTroopLevel(troop.id);
        CardState state = getCardState(troop.id, currentLevel);

        auto card = createTroopCard(troop, state, currentLevel);
        card->setAnchorPoint(Vec2(0, 0.5f));
        card->setPosition(Vec2(startX, (CARD_HEIGHT + 20) / 2));
        _scrollView->addChild(card);

        startX += CARD_WIDTH + padding;
    }

    _scrollView->setInnerContainerSize(Size(startX, CARD_HEIGHT + 20));
    updateInstantFinishButton();
}

LaboratoryLayer::CardState LaboratoryLayer::getCardState(int troopId, int currentLevel) const {
    auto dataManager = VillageDataManager::getInstance();
    auto upgradeConfig = TroopUpgradeConfig::getInstance();
    auto troopConfig = TroopConfig::getInstance();

    // 1. 检查是否有正在进行的研究
    int researchingId = dataManager->getResearchingTroopId();
    if (researchingId != -1) {
        if (researchingId == troopId) {
            return CardState::RESEARCHING;
        } else {
            return CardState::RESEARCH_BUSY;
        }
    }

    // 2. 检查兵种是否已解锁（训练营等级）
    TroopInfo info = troopConfig->getTroopById(troopId);
    int barracksLevel = getMaxBarracksLevel();
    if (barracksLevel < info.unlockBarracksLvl) {
        return CardState::TROOP_LOCKED;
    }

    // 3. 检查是否已满级
    int maxLevel = upgradeConfig->getMaxLevel(troopId);
    if (currentLevel >= maxLevel) {
        return CardState::MAX_LEVEL;
    }

    // 4. 检查实验室等级是否足够
    int labLevel = dataManager->getLaboratoryLevel();
    if (!upgradeConfig->canUpgradeWithLabLevel(troopId, currentLevel, labLevel)) {
        return CardState::LAB_LOCKED;
    }

    return CardState::NORMAL;
}

Widget* LaboratoryLayer::createTroopCard(const TroopInfo& info, CardState state, int currentLevel) {
    auto widget = Widget::create();
    widget->setContentSize(Size(CARD_WIDTH, CARD_HEIGHT));
    widget->setTouchEnabled(true);

    auto dataManager = VillageDataManager::getInstance();
    auto upgradeConfig = TroopUpgradeConfig::getInstance();

    // 1. 卡片背景
    Color4B bgColor = (state == CardState::NORMAL) ? Color4B(80, 80, 80, 255) : Color4B(60, 60, 60, 255);
    auto bg = LayerColor::create(bgColor, CARD_WIDTH, CARD_HEIGHT);
    widget->addChild(bg);

    // 2. 兵种头像（放大后的尺寸）
    auto sprite = Sprite::create(info.iconPath);
    if (sprite) {
        float scale = (CARD_WIDTH - 30) / sprite->getContentSize().width;
        sprite->setScale(scale);
        sprite->setPosition(CARD_WIDTH / 2, CARD_HEIGHT / 2 + 20);

        // 非正常状态变灰
        if (state != CardState::NORMAL && state != CardState::RESEARCHING) {
            sprite->setColor(Color3B(128, 128, 128));
        }
        widget->addChild(sprite);
    }

    // 3. 左上角等级/状态显示
    std::string levelText;
    if (state == CardState::MAX_LEVEL) {
        levelText = "MAX";
    } else {
        levelText = StringUtils::format("Lv.%d", currentLevel);
    }

    auto levelBg = LayerColor::create(Color4B::BLACK, 60, 28);
    levelBg->setPosition(8, CARD_HEIGHT - 35);
    widget->addChild(levelBg);

    auto lvlLabel = Label::createWithTTF(levelText, LAB_FONT, 18);
    if (!lvlLabel) lvlLabel = Label::createWithSystemFont(levelText, "Arial", 18);
    lvlLabel->setPosition(30, 14);
    levelBg->addChild(lvlLabel);

    // 4. 右上角 info 按钮
    auto infoBtn = Button::create("UI/training-camp/troop-cards/info_btn.png");
    if (infoBtn) {
        infoBtn->setPosition(Vec2(CARD_WIDTH - 22, CARD_HEIGHT - 22));
        infoBtn->setScale(0.12f);
        infoBtn->addClickEventListener([=](Ref*) {
            this->showUpgradePopup(info.id);
        });
        widget->addChild(infoBtn);
    }

    // 5. 底部显示（根据状态不同显示不同内容）
    Label* bottomLabel = nullptr;
    switch (state) {
        case CardState::NORMAL: {
            // 显示升级费用 + 圣水图标（放大后的尺寸）
            int cost = upgradeConfig->getUpgradeCost(info.id, currentLevel);
            auto costBg = LayerColor::create(Color4B(0, 0, 0, 150), CARD_WIDTH - 16, 32);
            costBg->setPosition(8, 8);
            widget->addChild(costBg);

            auto costLabel = Label::createWithTTF(StringUtils::format("%d", cost), LAB_FONT, 18);
            if (!costLabel) costLabel = Label::createWithSystemFont(std::to_string(cost), "Arial", 18);
            costLabel->setAnchorPoint(Vec2(0, 0.5f));
            costLabel->setPosition(8, 16);
            costBg->addChild(costLabel);

            // 圣水图标
            auto elixirIcon = Sprite::create("ImageElements/elixir_icon.png");
            if (elixirIcon) {
                elixirIcon->setScale(0.22f);
                elixirIcon->setPosition(CARD_WIDTH - 40, 16);
                costBg->addChild(elixirIcon);
            }
            break;
        }
        case CardState::MAX_LEVEL:
            // 不显示费用
            break;
        case CardState::LAB_LOCKED: {
            int requiredLab = upgradeConfig->getRequiredLabLevel(info.id, currentLevel + 1);
            bottomLabel = Label::createWithTTF(StringUtils::format("需要%d级实验室", requiredLab), LAB_FONT, 16);
            break;
        }
        case CardState::TROOP_LOCKED: {
            bottomLabel = Label::createWithTTF(StringUtils::format("需要%d级训练营", info.unlockBarracksLvl), LAB_FONT, 16);
            break;
        }
        case CardState::RESEARCHING: {
            // 显示倒计时和研究中（放大后的尺寸）
            long long finishTime = dataManager->getResearchFinishTime();
            long long currentTime = time(nullptr);
            int remaining = (int)(finishTime - currentTime);
            if (remaining < 0) remaining = 0;

            auto timeBg = LayerColor::create(Color4B(0, 100, 0, 200), CARD_WIDTH - 16, 50);
            timeBg->setPosition(8, 8);
            widget->addChild(timeBg);

            auto timeLabel = Label::createWithTTF(formatTime(remaining), LAB_FONT, 20);
            if (!timeLabel) timeLabel = Label::createWithSystemFont(formatTime(remaining), "Arial", 20);
            timeLabel->setPosition((CARD_WIDTH - 16) / 2, 32);
            timeLabel->setName("timeLabel");
            timeBg->addChild(timeLabel);

            auto statusLabel = Label::createWithTTF("【研究中】", LAB_FONT, 16);
            if (!statusLabel) statusLabel = Label::createWithSystemFont("研究中", "Arial", 16);
            statusLabel->setPosition((CARD_WIDTH - 16) / 2, 12);
            timeBg->addChild(statusLabel);
            break;
        }
        case CardState::RESEARCH_BUSY:
            bottomLabel = Label::createWithTTF("研究中", LAB_FONT, 16);
            break;
    }

    if (bottomLabel) {
        bottomLabel->setColor(Color3B(200, 200, 200));
        bottomLabel->setPosition(CARD_WIDTH / 2, 25);
        widget->addChild(bottomLabel);
    }

    // 6. 点击事件
    widget->addClickEventListener([=](Ref*) {
        this->onTroopCardClicked(info.id);
    });

    return widget;
}

void LaboratoryLayer::onTroopCardClicked(int troopId) {
    auto dataManager = VillageDataManager::getInstance();
    int currentLevel = dataManager->getTroopLevel(troopId);
    CardState state = getCardState(troopId, currentLevel);

    switch (state) {
        case CardState::NORMAL:
            showUpgradePopup(troopId);
            break;
        case CardState::MAX_LEVEL:
            showToast("该兵种已经满级");
            break;
        case CardState::LAB_LOCKED:
            showToast("需要升级实验室");
            break;
        case CardState::TROOP_LOCKED:
            showToast("需要升级训练营解锁此兵种");
            break;
        case CardState::RESEARCHING:
            showToast("正在研究中...");
            break;
        case CardState::RESEARCH_BUSY:
            showToast("请等待当前研究完成");
            break;
    }
}

void LaboratoryLayer::showUpgradePopup(int troopId) {
    auto dataManager = VillageDataManager::getInstance();
    auto upgradeConfig = TroopUpgradeConfig::getInstance();
    auto troopConfig = TroopConfig::getInstance();

    TroopInfo info = troopConfig->getTroopById(troopId);
    int currentLevel = dataManager->getTroopLevel(troopId);
    int maxLevel = upgradeConfig->getMaxLevel(troopId);

    // 1. 全屏遮罩
    auto shieldLayer = LayerColor::create(Color4B(0, 0, 0, 180));
    this->addChild(shieldLayer, 200);

    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [=](Touch* t, Event* e) {
        shieldLayer->removeFromParent();
        return true;
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, shieldLayer);

    // 2. 弹窗主面板
    float popupW = 600;
    float popupH = 400;
    auto bg = LayerColor::create(Color4B(210, 200, 190, 255), popupW, popupH);
    bg->ignoreAnchorPointForPosition(false);
    bg->setAnchorPoint(Vec2(0.5f, 0.5f));

    auto visibleSize = Director::getInstance()->getVisibleSize();
    bg->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    shieldLayer->addChild(bg);

    // 3. 顶部标题栏
    auto titleBg = LayerColor::create(Color4B(180, 160, 140, 255), popupW, 50);
    titleBg->setPosition(0, popupH - 50);
    bg->addChild(titleBg);

    // 标题文字：显示升级方向（如果未满级）
    std::string titleStr;
    if (currentLevel < maxLevel) {
        titleStr = StringUtils::format("%d→%d级 %s", currentLevel, currentLevel + 1, info.name.c_str());
    } else {
        titleStr = StringUtils::format("%d级 %s (MAX)", currentLevel, info.name.c_str());
    }

    auto titleLabel = Label::createWithTTF(titleStr, LAB_FONT, 24);
    if (!titleLabel) titleLabel = Label::createWithSystemFont(titleStr, "Arial", 24);
    titleLabel->setPosition(popupW / 2, popupH - 25);
    titleLabel->setColor(Color3B::BLACK);
    titleLabel->enableBold();
    bg->addChild(titleLabel);

    // 关闭按钮
    auto closeBtn = Button::create();
    closeBtn->setTitleText("X");
    closeBtn->setTitleFontSize(30);
    closeBtn->setTitleColor(Color3B::RED);
    closeBtn->setPosition(Vec2(popupW - 30, popupH - 25));
    closeBtn->addClickEventListener([=](Ref*) {
        shieldLayer->removeFromParent();
    });
    bg->addChild(closeBtn);

    // 4. 左侧：兵种大图
    auto sprite = Sprite::create(info.iconPath);
    if (sprite) {
        sprite->setScale(1.5f);
        sprite->setPosition(popupW * 0.25f, popupH * 0.55f);
        bg->addChild(sprite);
    }

    // 5. 右侧：属性列表（显示原属性 + 增量）
    float startX = popupW * 0.5f;
    float startY = popupH - 80;
    float lineHeight = 35;
    int lineCount = 0;

    const TroopLevelData* currentData = upgradeConfig->getLevelData(troopId, currentLevel);
    const TroopLevelData* nextData = upgradeConfig->getLevelData(troopId, currentLevel + 1);

    auto addStatRow = [&](const std::string& key, const std::string& value) {
        float y = startY - (lineCount * lineHeight);

        auto keyLabel = Label::createWithTTF(key, LAB_FONT, 20);
        if (!keyLabel) keyLabel = Label::createWithSystemFont(key, "Arial", 20);
        keyLabel->setAnchorPoint(Vec2(0, 0.5f));
        keyLabel->setPosition(startX, y);
        keyLabel->setColor(Color3B(80, 80, 80));
        bg->addChild(keyLabel);

        auto valLabel = Label::createWithTTF(value, LAB_FONT, 20);
        if (!valLabel) valLabel = Label::createWithSystemFont(value, "Arial", 20);
        valLabel->setAnchorPoint(Vec2(0, 0.5f));
        valLabel->setPosition(startX + 110, y);
        valLabel->setColor(Color3B::BLACK);
        bg->addChild(valLabel);

        lineCount++;
    };

    // 属性显示：如果未满级，显示"原属性 + 增量"格式
    if (currentData && nextData && currentLevel < maxLevel) {
        int hpIncrease = nextData->hitpoints - currentData->hitpoints;
        int dpsIncrease = nextData->damagePerSecond - currentData->damagePerSecond;

        addStatRow("生命值:", StringUtils::format("%d + %d", currentData->hitpoints, hpIncrease));
        addStatRow("每秒伤害:", StringUtils::format("%d + %d", currentData->damagePerSecond, dpsIncrease));
    } else if (currentData) {
        addStatRow("生命值:", std::to_string(currentData->hitpoints));
        addStatRow("每秒伤害:", std::to_string(currentData->damagePerSecond));
    }

    addStatRow("所需空间:", std::to_string(info.housingSpace));
    addStatRow("移动速度:", std::to_string(info.moveSpeed));
    addStatRow("伤害类型:", info.damageType);
    addStatRow("攻击目标:", info.target);

    // 6. 底部：升级按钮（替换原来的描述）
    if (currentLevel < maxLevel && getCardState(troopId, currentLevel) == CardState::NORMAL) {
        int cost = upgradeConfig->getUpgradeCost(troopId, currentLevel);

        // 分割线
        auto line = LayerColor::create(Color4B(150, 150, 150, 255), popupW - 40, 2);
        line->setPosition(20, 70);
        bg->addChild(line);

        // 升级按钮
        auto upgradeBtn = Button::create();
        upgradeBtn->ignoreContentAdaptWithSize(false);
        upgradeBtn->setContentSize(Size(200, 50));

        auto btnBg = LayerColor::create(Color4B(50, 150, 50, 255), 200, 50);
        btnBg->setTouchEnabled(false);
        upgradeBtn->addChild(btnBg, -1);

        // 按钮内容：圣水图标 + 费用 + 确认
        auto elixirIcon = Sprite::create("ImageElements/elixir_icon.png");
        if (elixirIcon) {
            elixirIcon->setScale(0.25f);
            elixirIcon->setPosition(30, 25);
            upgradeBtn->addChild(elixirIcon);
        }

        auto costLabel = Label::createWithTTF(StringUtils::format("%d 确认", cost), LAB_FONT, 20);
        if (!costLabel) costLabel = Label::createWithSystemFont(StringUtils::format("%d Confirm", cost), "Arial", 20);
        costLabel->setPosition(120, 25);
        upgradeBtn->addChild(costLabel);

        upgradeBtn->setPosition(Vec2(popupW / 2, 45));
        upgradeBtn->addClickEventListener([=](Ref*) {
            shieldLayer->removeFromParent();
            this->onUpgradeConfirmed(troopId);
        });
        bg->addChild(upgradeBtn);
    } else {
        // 已满级或不可升级，显示描述文字
        auto line = LayerColor::create(Color4B(150, 150, 150, 255), popupW - 40, 2);
        line->setPosition(20, 70);
        bg->addChild(line);

        auto descLabel = Label::createWithTTF(info.description, LAB_FONT, 16);
        if (!descLabel) descLabel = Label::createWithSystemFont(info.description, "Arial", 16);
        descLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
        descLabel->setPosition(popupW / 2, 35);
        descLabel->setDimensions(popupW - 40, 60);
        descLabel->setAlignment(TextHAlignment::CENTER, TextVAlignment::CENTER);
        descLabel->setColor(Color3B(60, 60, 60));
        bg->addChild(descLabel);
    }
}

void LaboratoryLayer::onUpgradeConfirmed(int troopId) {
    auto dataManager = VillageDataManager::getInstance();
    auto upgradeConfig = TroopUpgradeConfig::getInstance();

    int currentLevel = dataManager->getTroopLevel(troopId);
    int cost = upgradeConfig->getUpgradeCost(troopId, currentLevel);

    // 检查资源
    if (dataManager->getElixir() < cost) {
        showToast("圣水不足！");
        return;
    }

    // 开始升级
    if (dataManager->startTroopUpgrade(troopId)) {
        showToast("开始研究！");
        refreshCards();
    } else {
        showToast("无法开始研究");
    }
}

void LaboratoryLayer::showToast(const std::string& message) {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    auto toast = Label::createWithTTF(message, LAB_FONT, 24);
    if (!toast) toast = Label::createWithSystemFont(message, "Arial", 24);
    toast->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    toast->setColor(Color3B::WHITE);

    // 背景
    auto toastBg = LayerColor::create(Color4B(0, 0, 0, 180),
        toast->getContentSize().width + 40,
        toast->getContentSize().height + 20);
    toastBg->ignoreAnchorPointForPosition(false);
    toastBg->setAnchorPoint(Vec2(0.5f, 0.5f));
    toastBg->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    this->addChild(toastBg, 300);

    toast->setPosition(toastBg->getContentSize().width / 2, toastBg->getContentSize().height / 2);
    toastBg->addChild(toast);

    // 动画消失
    toastBg->runAction(Sequence::create(
        DelayTime::create(1.5f),
        FadeOut::create(0.3f),
        RemoveSelf::create(),
        nullptr
    ));
}

void LaboratoryLayer::update(float dt) {
    auto dataManager = VillageDataManager::getInstance();

    // 记录之前的研究状态
    bool wasResearching = dataManager->isResearching();
    int previousResearchId = dataManager->getResearchingTroopId();

    // 检查研究是否完成
    dataManager->checkAndFinishResearch();

    // 如果研究刚刚完成，立即刷新卡片
    bool isNowResearching = dataManager->isResearching();
    if (wasResearching && !isNowResearching) {
        CCLOG("LaboratoryLayer: Research completed, refreshing cards");
        refreshCards();
        return;
    }

    // 如果有研究正在进行，每秒刷新卡片更新倒计时
    static float refreshTimer = 0;
    refreshTimer += dt;
    if (refreshTimer >= 1.0f) {
        refreshTimer = 0;
        if (isNowResearching) {
            refreshCards();
        }
    }
}

void LaboratoryLayer::onCloseClicked() {
    _bgNode->runAction(Sequence::create(
        ScaleTo::create(0.1f, 0.0f),
        CallFunc::create([this]() {
            this->removeFromParent();
        }),
        nullptr
    ));
}

int LaboratoryLayer::getMaxBarracksLevel() const {
    auto dataManager = VillageDataManager::getInstance();
    int maxLevel = 0;

    for (const auto& b : dataManager->getAllBuildings()) {
        if (b.type == 102 && b.state == BuildingInstance::State::BUILT) {
            if (b.level > maxLevel) maxLevel = b.level;
        }
    }

    return maxLevel > 0 ? maxLevel : 1;
}

std::string LaboratoryLayer::formatTime(int seconds) const {
    if (seconds < 0) seconds = 0;

    int mins = seconds / 60;
    int secs = seconds % 60;

    return StringUtils::format("%02d:%02d", mins, secs);
}

int LaboratoryLayer::getInstantFinishCost() const {
    auto dataManager = VillageDataManager::getInstance();
    
    // 如果没有正在研究的兵种，返回0
    int troopId = dataManager->getResearchingTroopId();
    if (troopId == -1) {
        return 0;
    }
    
    // 获取当前兵种等级（正在升级中，所以目标等级是当前等级+1）
    int currentLevel = dataManager->getTroopLevel(troopId);
    int targetLevel = currentLevel + 1;
    
    // 根据目标等级确定钻石费用
    // 升级到2级：10钻石，升级到3级：20钻石
    if (targetLevel == 2) {
        return 10;
    } else if (targetLevel == 3) {
        return 20;
    }
    
    return 10;  // 默认值
}

void LaboratoryLayer::updateInstantFinishButton() {
    if (!_instantFinishBtn) return;
    
    auto dataManager = VillageDataManager::getInstance();
    bool isResearching = dataManager->isResearching();
    
    // 只有正在研究时才显示按钮和文字标签
    _instantFinishBtn->setVisible(isResearching);
    
    // 同时控制文字标签的显示
    auto finishLabel = _bgNode->getChildByName("finishLabel");
    if (finishLabel) {
        finishLabel->setVisible(isResearching);
    }
    
    if (isResearching) {
        // 更新费用显示
        int cost = getInstantFinishCost();
        auto costLabel = dynamic_cast<Label*>(_instantFinishBtn->getChildByName("costLabel"));
        if (costLabel) {
            costLabel->setString(std::to_string(cost));
        }
    }
}

void LaboratoryLayer::onInstantFinishClicked() {
    auto dataManager = VillageDataManager::getInstance();
    
    // 检查是否有正在研究的兵种
    int troopId = dataManager->getResearchingTroopId();
    if (troopId == -1) {
        showToast("没有正在进行的研究");
        return;
    }
    
    // 计算所需钻石
    int cost = getInstantFinishCost();
    
    // 检查钻石是否足够
    if (dataManager->getGems() < cost) {
        showToast("钻石不足！");
        return;
    }
    
    // 扣除钻石
    dataManager->addGems(-cost);
    
    // 立即完成升级 - 将完成时间设为当前时间
    dataManager->finishResearchImmediately();
    
    // 检查并完成研究
    dataManager->checkAndFinishResearch();
    
    showToast("研究已完成！");
    
    // 刷新界面
    refreshCards();
    updateInstantFinishButton();
}
