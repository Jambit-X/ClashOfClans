// TrainingLayer.cpp
// 训练层实现，处理部队训练和军队编辑功能

#pragma execution_character_set("utf-8")
#include "TrainingLayer.h"
#include "Model/TroopConfig.h"
#include "Manager/VillageDataManager.h"
#include "Manager/BuildingManager.h"
#include "Model/BuildingConfig.h" 

USING_NS_CC;
using namespace ui;

const std::string TRAIN_FONT = "fonts/simhei.ttf";

// 界面尺寸配置
const float PANEL_WIDTH = 800.0f;
const float PANEL_HEIGHT = 500.0f;
const float SECTION_WIDTH = 740.0f;
const float SECTION_HEIGHT = 160.0f;

// 兵种卡片尺寸
const float CARD_WIDTH = 130.0f;
const float CARD_HEIGHT = 160.0f;

Scene* TrainingLayer::createScene() {
    auto scene = Scene::create();
    auto layer = TrainingLayer::create();
    scene->addChild(layer);
    return scene;
}

bool TrainingLayer::init() {
    if (!Layer::init()) {
        return false;
    }

    // 初始化数据
    _capacityLabel = nullptr;

    // 全屏半透明遮罩
    auto shieldLayer = LayerColor::create(Color4B(0, 0, 0, 180));
    this->addChild(shieldLayer);

    // 设置触摸吞噬，处理定时器清理
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch* t, Event* e) { return true; };
    listener->onTouchEnded = [this](Touch* t, Event* e) {
        if (!_activeLongPressKey.empty()) {
            this->unschedule(_activeLongPressKey);
            _activeLongPressKey.clear();
        }
    };
    listener->onTouchCancelled = [this](Touch* t, Event* e) {
        if (!_activeLongPressKey.empty()) {
            this->unschedule(_activeLongPressKey);
            _activeLongPressKey.clear();
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 初始化核心UI
    initBackground();

    // 初始化底部选择面板
    initTroopSelectionPanel();
    
    updateCapacityLabel();
    updateArmyView();
    return true;
}

void TrainingLayer::initBackground() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 主面板容器
    _bgNode = Node::create();
    _bgNode->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    this->addChild(_bgNode);

    // 弹窗底板
    auto panelBg = LayerColor::create(Color4B(60, 40, 30, 255), PANEL_WIDTH, PANEL_HEIGHT);
    panelBg->ignoreAnchorPointForPosition(false);
    panelBg->setAnchorPoint(Vec2(0.5f, 0.5f));
    panelBg->setPosition(0, 0);
    _bgNode->addChild(panelBg);

    // 顶部标签"我的军队"
    auto titleBg = LayerColor::create(Color4B(50, 160, 50, 255), 200, 50);
    titleBg->ignoreAnchorPointForPosition(false);
    titleBg->setAnchorPoint(Vec2(0.5f, 0.0f));
    titleBg->setPosition(0, PANEL_HEIGHT / 2);
    _bgNode->addChild(titleBg);

    auto titleLabel = Label::createWithTTF("我的军队", TRAIN_FONT, 28);
    if (!titleLabel) titleLabel = Label::createWithSystemFont("我的军队", "Arial", 28);
    titleLabel->setPosition(100, 25);
    titleBg->addChild(titleLabel);

    // 关闭按钮
    auto closeBtn = Button::create();
    closeBtn->ignoreContentAdaptWithSize(false);
    closeBtn->setContentSize(Size(50, 50));
    auto btnBg = LayerColor::create(Color4B(200, 50, 50, 255), 50, 50);
    btnBg->setTouchEnabled(false);
    closeBtn->addChild(btnBg, -1);

    closeBtn->setTitleText("X");
    closeBtn->setTitleFontSize(30);
    closeBtn->setTitleFontName("Arial");
    closeBtn->setPosition(Vec2(PANEL_WIDTH / 2 - 35, PANEL_HEIGHT / 2 - 35));
    closeBtn->addClickEventListener([=](Ref*) {
        this->onCloseClicked();
    });
    _bgNode->addChild(closeBtn);

    // 创建军队模块（上方）
    createSection("部队", "0/40", "点击以编辑军队",
        Color4B(90, 60, 40, 255),
        50.0f,
        [=]() { this->onTroopsSectionClicked(); });
    
    // 初始化顶部军队面板
    initTopArmyPanel();

    // 创建法术模块（下方）
    createSection("法术", "0/2", "点击以编辑法术",
        Color4B(70, 50, 70, 255),
        -130.0f,
        [=]() { this->onSpellsSectionClicked(); });

    updateCapacityLabel();
    updateArmyView();
}

void TrainingLayer::createSection(const std::string& title,
    const std::string& capacityStr,
    const std::string& subText,
    const cocos2d::Color4B& color,
    float posY,
    const std::function<void()>& callback)
{
    auto sectionBtn = Button::create();
    sectionBtn->ignoreContentAdaptWithSize(false);
    sectionBtn->setContentSize(Size(SECTION_WIDTH, SECTION_HEIGHT));
    sectionBtn->setAnchorPoint(Vec2(0.5f, 0.5f));
    sectionBtn->setPosition(Vec2(0, posY));

    auto bgLayer = LayerColor::create(color, SECTION_WIDTH, SECTION_HEIGHT);
    bgLayer->setTouchEnabled(false);
    sectionBtn->addChild(bgLayer, -1);

    sectionBtn->addClickEventListener([=](Ref*) {
        if (callback) callback();
    });

    _bgNode->addChild(sectionBtn);

    // 标题
    auto titleLabel = Label::createWithTTF(title, TRAIN_FONT, 20);
    if (!titleLabel) titleLabel = Label::createWithSystemFont(title, "Arial", 20);
    titleLabel->setAnchorPoint(Vec2(0, 1));
    titleLabel->setPosition(Vec2(10, SECTION_HEIGHT - 10));
    titleLabel->setColor(Color3B::YELLOW);
    bgLayer->addChild(titleLabel);

    // 容量显示
    auto capLabel = Label::createWithTTF(capacityStr, TRAIN_FONT, 20);
    if (!capLabel) capLabel = Label::createWithSystemFont(capacityStr, "Arial", 20);
    capLabel->setAnchorPoint(Vec2(0, 1));
    capLabel->setPosition(Vec2(80, SECTION_HEIGHT - 10));
    bgLayer->addChild(capLabel);

    // 如果是"部队"栏，保存Label引用
    if (title == "部队") {
        _capacityLabel = capLabel;
    }

    // 中间的大提示文字
    auto hintLabel = Label::createWithTTF(subText, TRAIN_FONT, 24);
    if (!hintLabel) hintLabel = Label::createWithSystemFont(subText, "Arial", 24);
    hintLabel->setPosition(Vec2(SECTION_WIDTH / 2, SECTION_HEIGHT / 2));
    hintLabel->setColor(Color3B(200, 200, 200));
    bgLayer->addChild(hintLabel);
}

void TrainingLayer::initTopArmyPanel() {
    _armyScrollView = ScrollView::create();
    _armyScrollView->setDirection(ScrollView::Direction::HORIZONTAL);
    _armyScrollView->setContentSize(Size(740, 110));
    _armyScrollView->setAnchorPoint(Vec2(0.5f, 0.5f));
    _armyScrollView->setPosition(Vec2(0, 50));
    _armyScrollView->setScrollBarEnabled(false);
    _armyScrollView->setBounceEnabled(true);
    _armyScrollView->setTouchEnabled(true);

    _armyScrollView->addTouchEventListener([=](Ref* sender, Widget::TouchEventType type) {
        if (type == Widget::TouchEventType::ENDED) {
            // 区分点击和拖动
            Vec2 startPos = _armyScrollView->getTouchBeganPosition();
            Vec2 endPos = _armyScrollView->getTouchEndPosition();

            if (startPos.distance(endPos) < 20.0f) {
                this->onTroopsSectionClicked();
            }
        }
    });
    _bgNode->addChild(_armyScrollView, 10);
}

Widget* TrainingLayer::createArmyUnitCell(int troopId, int count) {
    auto config = TroopConfig::getInstance()->getTroopById(troopId);
    float cellSize = 90.0f;

    auto widget = Widget::create();
    widget->setContentSize(Size(cellSize, cellSize));
    widget->setTouchEnabled(true);

    // 头像背景
    auto bg = LayerColor::create(Color4B(0, 0, 0, 50), cellSize, cellSize);
    widget->addChild(bg);

    // 兵种头像
    auto sprite = Sprite::create(config.iconPath);
    if (sprite) {
        float scale = (cellSize - 10) / sprite->getContentSize().width;
        sprite->setScale(scale);
        sprite->setPosition(cellSize / 2, cellSize / 2);
        widget->addChild(sprite);
    }

    // 数量角标
    auto numBg = LayerColor::create(Color4B(0, 100, 200, 200), 40, 20);
    numBg->setPosition(0, cellSize - 20);
    widget->addChild(numBg);

    auto numLabel = Label::createWithTTF(StringUtils::format("x%d", count), TRAIN_FONT, 16);
    numLabel->setPosition(20, 10);
    numLabel->setTag(100);
    numBg->addChild(numLabel);
    numBg->setTag(101);

    // 红色删除按钮
    auto removeBtn = Button::create();
    removeBtn->ignoreContentAdaptWithSize(false);
    removeBtn->setContentSize(Size(25, 25));

    auto btnColor = LayerColor::create(Color4B::RED, 25, 25);
    btnColor->setTouchEnabled(false);
    removeBtn->addChild(btnColor, -1);

    auto minusLabel = Label::createWithSystemFont("-", "Arial", 24, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);
    minusLabel->setPosition(12.5f, 12.5f);
    removeBtn->addChild(minusLabel);

    removeBtn->setPosition(Vec2(cellSize - 12.5f, cellSize - 12.5f));

    std::string key = "remove_troop_" + std::to_string(troopId);

    setupLongPressAction(removeBtn, [=]() {
        this->removeTroop(troopId);
    }, key);

    widget->addChild(removeBtn);

    widget->setTag(troopId);

    return widget;
}

void TrainingLayer::updateArmyView() {
    if (!_armyScrollView) return;

    // 停止所有remove_troop_的定时器
    auto allConfig = TroopConfig::getInstance()->getAllTroops();
    for (const auto& info : allConfig) {
        std::string key = "remove_troop_" + std::to_string(info.id);
        if (key != _activeLongPressKey) {
            this->unschedule(key);
        }
    }

    _armyScrollView->removeAllChildren();

    auto dataManager = VillageDataManager::getInstance();
    auto allConfigForIteration = TroopConfig::getInstance()->getAllTroops();

    float startX = 0;
    float padding = 10.0f;
    float cellSize = 90.0f;

    for (const auto& info : allConfigForIteration) {
        int id = info.id;
        int count = dataManager->getTroopCount(id);

        if (count > 0) {
            auto cell = createArmyUnitCell(id, count);
            cell->setAnchorPoint(Vec2(0, 0));
            cell->setPosition(Vec2(startX, 10));
            _armyScrollView->addChild(cell);

            startX += cellSize + padding;
        }
    }

    _armyScrollView->setInnerContainerSize(Size(startX, 110));
}

void TrainingLayer::removeTroop(int troopId) {
    auto dataManager = VillageDataManager::getInstance();

    if (dataManager->removeTroop(troopId, 1)) {
        updateCapacityLabel();
        
        int newCount = dataManager->getTroopCount(troopId);
        
        // 如果正在长按且数量还大于0，只更新数量标签
        if (!_activeLongPressKey.empty() && newCount > 0) {
            auto cell = _armyScrollView->getChildByTag(troopId);
            if (cell) {
                auto numBg = cell->getChildByTag(101);
                if (numBg) {
                    auto numLabel = dynamic_cast<Label*>(numBg->getChildByTag(100));
                    if (numLabel) {
                        numLabel->setString(StringUtils::format("x%d", newCount));
                        return;
                    }
                }
            }
        }
        
        updateArmyView();
    }
}

void TrainingLayer::setupLongPressAction(Widget* target, std::function<void()> callback, const std::string& scheduleKey) {
    target->addTouchEventListener([=](Ref* sender, Widget::TouchEventType type) {
        if (type == Widget::TouchEventType::BEGAN) {
            _activeLongPressKey = scheduleKey;
            
            // 立即执行一次
            callback();

            // 开启定时器
            this->schedule([=](float) {
                callback();
            }, 0.1f, CC_REPEAT_FOREVER, 0.5f, scheduleKey);
        }
        else if (type == Widget::TouchEventType::ENDED || type == Widget::TouchEventType::CANCELED) {
            _activeLongPressKey.clear();
            this->unschedule(scheduleKey);
        }
    });
}

void TrainingLayer::initTroopSelectionPanel() {
    // 创建ScrollView
    _scrollView = ScrollView::create();
    _scrollView->setDirection(ScrollView::Direction::HORIZONTAL);
    _scrollView->setContentSize(Size(PANEL_WIDTH - 40, 180));
    _scrollView->setAnchorPoint(Vec2(0.5f, 0.0f));
    _scrollView->setPosition(Vec2(0, -230));
    _scrollView->setScrollBarEnabled(false);
    _scrollView->setBounceEnabled(true);
    _bgNode->addChild(_scrollView);

    // 获取数据
    auto troopList = TroopConfig::getInstance()->getAllTroops();
    int barracksLevel = getMaxBarracksLevel();

    if (barracksLevel == 0) barracksLevel = 1;

    // 动态添加卡片
    float startX = 0;
    float padding = 10;

    for (const auto& troop : troopList) {
        bool isUnlocked = (barracksLevel >= troop.unlockBarracksLvl);

        auto card = createTroopCard(troop, isUnlocked);
        card->setPosition(Vec2(startX + CARD_WIDTH / 2, CARD_HEIGHT / 2));
        _scrollView->addChild(card);

        startX += CARD_WIDTH + padding;
    }

    _scrollView->setInnerContainerSize(Size(startX, 180));
    _scrollView->setVisible(false);
}

Widget* TrainingLayer::createTroopCard(const TroopInfo& info, bool isUnlocked) {
    auto widget = Widget::create();
    widget->setContentSize(Size(CARD_WIDTH, CARD_HEIGHT));
    widget->setTouchEnabled(true);

    // 卡片背景
    auto bg = LayerColor::create(Color4B(80, 80, 80, 255), CARD_WIDTH, CARD_HEIGHT);
    bg->setPosition(0, 0);
    widget->addChild(bg);

    // 兵种头像
    auto sprite = Sprite::create(info.iconPath);
    if (sprite) {
        float scale = (CARD_WIDTH - 10) / sprite->getContentSize().width;
        sprite->setScale(scale);
        sprite->setPosition(CARD_WIDTH / 2, CARD_HEIGHT / 2 + 10);

        // 如果未解锁，变灰
        if (!isUnlocked) {
            auto program = GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_GRAYSCALE);
            sprite->setGLProgramState(program);
        }
        widget->addChild(sprite);
    }

    // 左下角：等级
    auto levelBg = LayerColor::create(Color4B::BLACK, 40, 20);
    levelBg->setPosition(5, 5);
    widget->addChild(levelBg);

    int actualLevel = VillageDataManager::getInstance()->getTroopLevel(info.id);
    auto lvlLabel = Label::createWithTTF(StringUtils::format("Lv.%d", actualLevel), TRAIN_FONT, 14);
    lvlLabel->setPosition(20, 10);
    levelBg->addChild(lvlLabel);

    // 右下角：人口空间
    auto spaceBg = LayerColor::create(Color4B(0, 0, 0, 150), 30, 20);
    spaceBg->setPosition(CARD_WIDTH - 35, 5);
    widget->addChild(spaceBg);

    auto spaceLabel = Label::createWithTTF(StringUtils::format("%d", info.housingSpace), TRAIN_FONT, 14);
    spaceLabel->setColor(Color3B(200, 200, 255));
    spaceLabel->setPosition(15, 10);
    spaceBg->addChild(spaceLabel);

    // 右上角：信息按钮
    auto infoBtn = Button::create("UI/training-camp/troop-cards/info_btn.png");
    if (infoBtn) {
        infoBtn->setPosition(Vec2(CARD_WIDTH - 15, CARD_HEIGHT - 15));
        infoBtn->setScale(0.1f);
        infoBtn->addClickEventListener([=](Ref*) {
            this->onInfoClicked(info.id);
        });
        widget->addChild(infoBtn);
    }

    // 点击卡片训练
    if (isUnlocked) {
        std::string key = "train_troop_" + std::to_string(info.id);

        setupLongPressAction(widget, [=]() {
            this->onTroopCardClicked(info.id);
        }, key);
    }
    else {
        widget->addClickEventListener([=](Ref*) {
            CCLOG("Troop locked! Need Barracks Lv.%d", info.unlockBarracksLvl);
        });
    }

    return widget;
}

void TrainingLayer::onTroopCardClicked(int troopId) {
    auto dataManager = VillageDataManager::getInstance();
    TroopInfo info = TroopConfig::getInstance()->getTroopById(troopId);

    // 获取当前占用和总容量
    int currentSpace = dataManager->getCurrentHousingSpace();
    int maxSpace = dataManager->calculateTotalHousingSpace();

    // 检查人口
    if (currentSpace + info.housingSpace > maxSpace) {
        auto tip = Label::createWithTTF("军队队列空间不足！", TRAIN_FONT, 30);
        tip->setPosition(Director::getInstance()->getVisibleSize() / 2);
        tip->setColor(Color3B::RED);
        this->addChild(tip, 100);

        tip->runAction(Sequence::create(DelayTime::create(1.0f), FadeOut::create(0.5f), RemoveSelf::create(), nullptr));

        if (_capacityLabel) {
            _capacityLabel->runAction(Sequence::create(TintTo::create(0.1f, Color3B::RED), TintTo::create(0.1f, Color3B::WHITE), nullptr));
        }
        return;
    }

    // 直接训练
    dataManager->addTroop(troopId, 1);

    // 刷新UI
    updateCapacityLabel();
    updateArmyView();
}

void TrainingLayer::updateCapacityLabel() {
    auto dataManager = VillageDataManager::getInstance();

    int current = dataManager->getCurrentHousingSpace();
    int max = dataManager->calculateTotalHousingSpace();

    if (_capacityLabel) {
        _capacityLabel->setString(StringUtils::format("%d/%d", current, max));
    }
}

void TrainingLayer::onInfoClicked(int troopId) {
    TroopInfo info = TroopConfig::getInstance()->getTroopById(troopId);

    // 全屏遮罩
    auto shieldLayer = LayerColor::create(Color4B(0, 0, 0, 180));
    this->addChild(shieldLayer, 200);

    // 点击遮罩关闭
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [=](Touch* t, Event* e) {
        shieldLayer->removeFromParent();
        return true;
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, shieldLayer);

    // 弹窗主面板
    float popupW = 600;
    float popupH = 400;
    auto bg = LayerColor::create(Color4B(210, 200, 190, 255), popupW, popupH);
    bg->ignoreAnchorPointForPosition(false);
    bg->setAnchorPoint(Vec2(0.5f, 0.5f));

    auto visibleSize = Director::getInstance()->getVisibleSize();
    bg->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    shieldLayer->addChild(bg);

    // 顶部标题栏
    auto titleBg = LayerColor::create(Color4B(180, 160, 140, 255), popupW, 50);
    titleBg->setPosition(0, popupH - 50);
    bg->addChild(titleBg);

    // 标题文字
    auto titleLabel = Label::createWithTTF(StringUtils::format("%d级 %s", info.level, info.name.c_str()), TRAIN_FONT, 24);
    if (!titleLabel) titleLabel = Label::createWithSystemFont(info.name, "Arial", 24);
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

    // 左侧：兵种大图
    auto sprite = Sprite::create(info.iconPath);
    if (sprite) {
        sprite->setScale(1.5f);
        sprite->setPosition(popupW * 0.25f, popupH * 0.55f);
        bg->addChild(sprite);
    }

    // 右侧：属性列表
    float startX = popupW * 0.5f;
    float startY = popupH - 80;
    float lineHeight = 35;
    int lineCount = 0;

    auto addStatRow = [&](const std::string& key, const std::string& value) {
        float y = startY - (lineCount * lineHeight);

        auto keyLabel = Label::createWithTTF(key, TRAIN_FONT, 20);
        if (!keyLabel) keyLabel = Label::createWithSystemFont(key, "Arial", 20);
        keyLabel->setAnchorPoint(Vec2(0, 0.5f));
        keyLabel->setPosition(startX, y);
        keyLabel->setColor(Color3B(80, 80, 80));
        bg->addChild(keyLabel);

        auto valLabel = Label::createWithTTF(value, TRAIN_FONT, 20);
        if (!valLabel) valLabel = Label::createWithSystemFont(value, "Arial", 20);
        valLabel->setAnchorPoint(Vec2(0, 0.5f));
        valLabel->setPosition(startX + 110, y);
        valLabel->setColor(Color3B::BLACK);
        bg->addChild(valLabel);

        lineCount++;
    };

    // 填充数据
    addStatRow("生命值:", std::to_string(info.hitpoints));
    addStatRow("每秒伤害:", std::to_string(info.damagePerSecond));
    addStatRow("所需空间:", std::to_string(info.housingSpace));
    addStatRow("移动速度:", std::to_string(info.moveSpeed));
    addStatRow("伤害类型:", info.damageType);
    addStatRow("攻击目标:", info.target);
    addStatRow("偏好目标:", info.favorite);

    // 底部描述文本
    auto line = LayerColor::create(Color4B(150, 150, 150, 255), popupW - 40, 2);
    line->setPosition(20, 70);
    bg->addChild(line);

    auto descLabel = Label::createWithTTF(info.description, TRAIN_FONT, 16);
    if (!descLabel) descLabel = Label::createWithSystemFont(info.description, "Arial", 16);
    descLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
    descLabel->setPosition(popupW / 2, 35);
    descLabel->setDimensions(popupW - 40, 60);
    descLabel->setAlignment(TextHAlignment::CENTER, TextVAlignment::CENTER);
    descLabel->setColor(Color3B(60, 60, 60));
    bg->addChild(descLabel);
}

int TrainingLayer::getMaxBarracksLevel() {
    auto dataManager = VillageDataManager::getInstance();
    if (!dataManager) return 1;

    auto buildings = dataManager->getAllBuildings();
    int maxLvl = 0;

    for (const auto& b : buildings) {
        if (b.type == 102 && b.state == BuildingInstance::State::BUILT) {
            if (b.level > maxLvl) maxLvl = b.level;
        }
    }

    return maxLvl > 0 ? maxLvl : 1;
}

void TrainingLayer::onCloseClicked() {
    _bgNode->runAction(Sequence::create(
        ScaleTo::create(0.1f, 0.0f),
        CallFunc::create([this]() {
            this->removeFromParent();
        }),
        nullptr
    ));
}

void TrainingLayer::onTroopsSectionClicked() {
    CCLOG("点击了【军队】模块");

    if (_scrollView) {
        bool isVisible = _scrollView->isVisible();

        if (!isVisible) {
            _scrollView->setVisible(true);
            _scrollView->setScale(0.1f);
            _scrollView->runAction(EaseBackOut::create(ScaleTo::create(0.2f, 1.0f)));
        }
        else {
            _scrollView->setVisible(false);
        }
    }
}

void TrainingLayer::onSpellsSectionClicked() {
    CCLOG("点击了【法术】模块 - 暂未实现");
}
