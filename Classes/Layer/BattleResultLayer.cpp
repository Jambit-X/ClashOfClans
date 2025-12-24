#pragma execution_character_set("utf-8")
#include "BattleResultLayer.h"
#include "../Scene/BattleScene.h"
#include "../Model/TroopConfig.h"
#include "../Manager/AudioManager.h"
#include "../Controller/BattleProcessController.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace ui;

const std::string FONT_PATH = "fonts/simhei.ttf";

BattleResultLayer* BattleResultLayer::createWithData(
    const std::map<int, int>& usedTroops,
    const std::map<int, int>& troopLevels,
    int lootedGold,
    int lootedElixir) {
    
    auto layer = new (std::nothrow) BattleResultLayer();
    if (layer && layer->initWithData(usedTroops, troopLevels, lootedGold, lootedElixir)) {
        layer->autorelease();
        return layer;
    }
    CC_SAFE_DELETE(layer);
    return nullptr;
}

bool BattleResultLayer::init() {
    // 默认初始化（无消耗数据）
    return initWithData({}, {}, 0, 0);
}

bool BattleResultLayer::initWithData(
    const std::map<int, int>& usedTroops,
    const std::map<int, int>& troopLevels,
    int lootedGold,
    int lootedElixir) {
    
    // 半透明黑色背景
    if (!LayerColor::initWithColor(Color4B(0, 0, 0, 200))) return false;

    _usedTroops = usedTroops;
    _troopLevels = troopLevels;
    _lootedGold = lootedGold;
    _lootedElixir = lootedElixir;

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // ✅ 新增：播放结算音乐（根据星数）
    playResultMusic();

    // 标题 - 黄色大字体
    auto titleLabel = Label::createWithTTF("战斗结束", FONT_PATH, 52);
    titleLabel->setPosition(visibleSize.width / 2, visibleSize.height / 2 + 180);
    titleLabel->setColor(Color3B::YELLOW);
    titleLabel->enableOutline(Color4B::BLACK, 3);
    this->addChild(titleLabel);

    // 【改造】战利品信息 - 使用图标+数值格式
    float centerX = visibleSize.width / 2;
    float lootY = visibleSize.height / 2 + 110;
    
    // "获得战利品："标签
    auto lootTitleLabel = Label::createWithTTF("获得战利品：", FONT_PATH, 28);
    lootTitleLabel->setAnchorPoint(Vec2(1, 0.5f));
    lootTitleLabel->setPosition(centerX - 60, lootY);
    lootTitleLabel->setColor(Color3B::WHITE);
    this->addChild(lootTitleLabel);
    
    // 金币图标
    auto goldIcon = Sprite::create("ImageElements/coin_icon.png");
    if (goldIcon) {
        goldIcon->setScale(0.4f);
        goldIcon->setPosition(centerX - 30, lootY);
        this->addChild(goldIcon);
    }
    
    // 金币数值
    auto goldLabel = Label::createWithTTF(StringUtils::format("%d", _lootedGold), "fonts/Marker Felt.ttf", 28);
    goldLabel->setAnchorPoint(Vec2(0, 0.5f));
    goldLabel->setPosition(centerX - 5, lootY);
    goldLabel->setColor(Color3B(255, 215, 0));  // 金色
    goldLabel->enableOutline(Color4B::BLACK, 2);
    this->addChild(goldLabel);
    
    // 圣水图标
    auto elixirIcon = Sprite::create("ImageElements/elixir_icon.png");
    if (elixirIcon) {
        elixirIcon->setScale(0.4f);
        elixirIcon->setPosition(centerX + 120, lootY);
        this->addChild(elixirIcon);
    }
    
    // 圣水数值
    auto elixirLabel = Label::createWithTTF(StringUtils::format("%d", _lootedElixir), "fonts/Marker Felt.ttf", 28);
    elixirLabel->setAnchorPoint(Vec2(0, 0.5f));
    elixirLabel->setPosition(centerX + 145, lootY);
    elixirLabel->setColor(Color3B(255, 0, 255));  // 紫红色
    elixirLabel->enableOutline(Color4B::BLACK, 2);
    this->addChild(elixirLabel);

    // 【新增】创建消耗兵种卡片
    createTroopCards();

    // [回营] 按钮
    auto btnReturn = Button::create("UI/battle/battle-prepare/back.png");
    btnReturn->setScale(0.4f);
    btnReturn->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 160));
    btnReturn->addClickEventListener([this](Ref*) {
        auto scene = dynamic_cast<BattleScene*>(this->getScene());
        if (scene) scene->onReturnHomeClicked();
    });
    this->addChild(btnReturn);

    // 吞噬触摸
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch*, Event*) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}

// ✅ 新增：播放结算音乐
void BattleResultLayer::playResultMusic() {
    auto audioManager = AudioManager::getInstance();
    
    // 获取当前战斗的星数
    int stars = BattleProcessController::getInstance()->getCurrentStars();
    
    CCLOG("========================================");
    CCLOG("BattleResultLayer::playResultMusic - START");
    CCLOG("  Current stars: %d", stars);
    
    // 根据星数选择音乐
    std::string musicFile;
    if (stars == 0) {
        musicFile = "Audios/battle_lost.mp3";  // 0星：失败音乐
        CCLOG("  Result: DEFEAT");
        CCLOG("  Music: %s", musicFile.c_str());
    } else {
        musicFile = "Audios/battle_win.mp3";   // 1-3星：胜利音乐
        CCLOG("BattleResultLayer: Victory with %d stars! Playing win music", stars);
    }
    
    // 播放音乐（不循环）
    CCLOG("  Calling AudioManager::playBackgroundMusic...");
    _resultMusicID = audioManager->playBackgroundMusic(musicFile, 0.8f, false);
    
    CCLOG("  Music ID returned: %d", _resultMusicID);
    
    if (_resultMusicID == -1) {
        CCLOG("  ERROR: Failed to play result music!");
    } else {
        CCLOG("  SUCCESS: Result music started");
    }
    
    CCLOG("BattleResultLayer::playResultMusic - END");
    CCLOG("========================================");
}

// ✅ 新增：退出时停止音乐
void BattleResultLayer::onExit() {
    CCLOG("BattleResultLayer: Exiting, stopping result music");
    
    if (_resultMusicID != -1) {
        auto audioManager = AudioManager::getInstance();
        audioManager->stopAudio(_resultMusicID);
        _resultMusicID = -1;
        CCLOG("BattleResultLayer: Result music stopped");
    }
    
    LayerColor::onExit();
}

void BattleResultLayer::createTroopCards() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    
    // 检查是否有消耗
    bool hasUsedTroops = false;
    for (const auto& pair : _usedTroops) {
        if (pair.second > 0) {
            hasUsedTroops = true;
            break;
        }
    }
    
    if (!hasUsedTroops) {
        // 没有消耗兵种时显示提示
        auto noUsageLabel = Label::createWithTTF("本次战斗未消耗兵种", FONT_PATH, 24);
        noUsageLabel->setPosition(visibleSize.width / 2, visibleSize.height / 2);
        noUsageLabel->setColor(Color3B::GRAY);
        this->addChild(noUsageLabel);
        return;
    }
    
    // "消耗：" 标签 - 更大更醒目
    auto consumeLabel = Label::createWithTTF("消耗：", FONT_PATH, 36);
    consumeLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
    consumeLabel->setPosition(visibleSize.width / 2, visibleSize.height / 2 + 40);
    consumeLabel->setColor(Color3B(255, 150, 50)); // 橙色
    consumeLabel->enableOutline(Color4B::BLACK, 3);
    this->addChild(consumeLabel);
    
    // 创建卡片容器 - 在消耗标签下方，距离拉开
    auto cardContainer = Node::create();
    cardContainer->setPosition(visibleSize.width / 2, visibleSize.height / 2 - 50);
    this->addChild(cardContainer);
    
    // 计算卡片数量和间距
    int cardCount = 0;
    for (const auto& pair : _usedTroops) {
        if (pair.second > 0) cardCount++;
    }
    
    const float CARD_SPACING = 90.0f;
    float startX = -(cardCount - 1) * CARD_SPACING / 2.0f;
    
    // 创建每个兵种卡片
    for (const auto& pair : _usedTroops) {
        int troopId = pair.first;
        int usedCount = pair.second;
        
        if (usedCount <= 0) continue;
        
        auto troopInfo = TroopConfig::getInstance()->getTroopById(troopId);
        
        // 卡片背景（使用图标）
        auto iconSprite = Sprite::create(troopInfo.iconPath);
        if (iconSprite) {
            iconSprite->setScale(0.55f);
            iconSprite->setPosition(startX, 0);
            cardContainer->addChild(iconSprite);
            
            // 数量标签 - 卡片上方，距离拉开，字体更大
            auto countLabel = Label::createWithTTF(
                StringUtils::format("x%d", usedCount), FONT_PATH, 32);
            countLabel->setPosition(
                iconSprite->getContentSize().width / 2,
                iconSprite->getContentSize().height + 25);
            countLabel->setColor(Color3B::WHITE);
            countLabel->enableOutline(Color4B::BLACK, 3);
            iconSprite->addChild(countLabel);
            
            // 等级标签 - 卡片左下角
            int level = 1;
            auto levelIt = _troopLevels.find(troopId);
            if (levelIt != _troopLevels.end()) {
                level = levelIt->second;
            }
            
            auto levelLabel = Label::createWithTTF(
                StringUtils::format("Lv.%d", level), FONT_PATH, 24);
            levelLabel->setAnchorPoint(Vec2(0, 0));
            levelLabel->setPosition(5, 5);
            levelLabel->setColor(Color3B(255, 215, 0)); // 金色
            levelLabel->enableOutline(Color4B::BLACK, 3);
            iconSprite->addChild(levelLabel);
            
            startX += CARD_SPACING;
        }
    }
    
    CCLOG("BattleResultLayer: Created %d troop cards", cardCount);
}
