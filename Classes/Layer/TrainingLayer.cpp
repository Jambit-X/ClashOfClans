#pragma execution_character_set("utf-8")
#include "TrainingLayer.h"
#include "Model/TroopConfig.h"
#include "Manager/VillageDataManager.h"
#include "Manager/BuildingManager.h"
#include "Model/BuildingConfig.h" 

USING_NS_CC;
using namespace ui;

// 字体设置
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
    _capacityLabel = nullptr; // 防止野指针

    // 1. 全屏半透明遮罩
    auto shieldLayer = LayerColor::create(Color4B(0, 0, 0, 180));
    this->addChild(shieldLayer);

    // 设置触摸吞噬
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch* t, Event* e) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 2. 初始化核心 UI
    initBackground();

    // 3. 【新增】初始化底部选择面板
    initTroopSelectionPanel();
    
    updateCapacityLabel();
    updateArmyView();
    return true;
}

void TrainingLayer::initBackground() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // --- 主面板容器 ---
    _bgNode = Node::create();
    _bgNode->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    this->addChild(_bgNode);

    // 1. 弹窗底板 (深褐色)
    auto panelBg = LayerColor::create(Color4B(60, 40, 30, 255), PANEL_WIDTH, PANEL_HEIGHT);
    panelBg->ignoreAnchorPointForPosition(false);
    panelBg->setAnchorPoint(Vec2(0.5f, 0.5f));
    panelBg->setPosition(0, 0);
    _bgNode->addChild(panelBg);

    // 2. 顶部标签 "我的军队" (绿色背景条)
    auto titleBg = LayerColor::create(Color4B(50, 160, 50, 255), 200, 50);
    titleBg->ignoreAnchorPointForPosition(false);
    titleBg->setAnchorPoint(Vec2(0.5f, 0.0f));
    titleBg->setPosition(0, PANEL_HEIGHT / 2);
    _bgNode->addChild(titleBg);

    auto titleLabel = Label::createWithTTF("我的军队", TRAIN_FONT, 28);
    if (!titleLabel) titleLabel = Label::createWithSystemFont("我的军队", "Arial", 28);
    titleLabel->setPosition(100, 25);
    titleBg->addChild(titleLabel);

    // 3. 关闭按钮
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

    // 4. 创建【军队】模块 (上方)
    createSection("部队", "0/40", "点击以编辑军队",
        Color4B(90, 60, 40, 255),
        50.0f,
        [=]() { this->onTroopsSectionClicked(); });
    
    // 【新增】在创建完背景后，初始化顶部军队面板
    initTopArmyPanel();

    // 5. 创建【法术】模块 (下方)
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

    // 【关键】如果是“部队”栏，保存Label引用以便后续更新
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
// ==========================================
// 【新增】顶部军队面板
// ==========================================

void TrainingLayer::initTopArmyPanel() {
    // 假设面板总宽 800，深褐色区域大概宽 740，高 120 左右
    // 位置大约在中心偏上

    _armyScrollView = ScrollView::create();
    _armyScrollView->setDirection(ScrollView::Direction::HORIZONTAL);
    _armyScrollView->setContentSize(Size(740, 110)); // 高度设为110，留点边距
    _armyScrollView->setAnchorPoint(Vec2(0.5f, 0.5f));

    // 放在 _bgNode 上。根据你之前的布局，Panel高500，"部队"栏大概在 Y=50 左右的位置
    // 这里需要你根据实际视觉微调 Y 坐标，让它正好在那个褐色框里
    _armyScrollView->setPosition(Vec2(0, 50)); // 假设 0 是中心 X

    _armyScrollView->setScrollBarEnabled(false);
    _armyScrollView->setBounceEnabled(true);
    _armyScrollView->setTouchEnabled(true);

    _armyScrollView->addTouchEventListener([=](Ref* sender, Widget::TouchEventType type) {
        // 当手指抬起时 (TOUCH_ENDED)
        if (type == Widget::TouchEventType::ENDED) {
            // 核心逻辑：区分“点击”和“拖动”
            // 如果用户是在拖动兵种列表，我们不应该开关菜单
            // 只有当手指移动距离很短时，才视为“点击”

            Vec2 startPos = _armyScrollView->getTouchBeganPosition();
            Vec2 endPos = _armyScrollView->getTouchEndPosition();

            if (startPos.distance(endPos) < 20.0f) { // 阈值设为20像素
                this->onTroopsSectionClicked();
            }
        }
        });
    _bgNode->addChild(_armyScrollView, 10); // Z=10 确保显示在背景之上
}

// ==========================================
// 创建上方的小卡片
// ==========================================

Widget* TrainingLayer::createArmyUnitCell(int troopId, int count) {
    auto config = TroopConfig::getInstance()->getTroopById(troopId);
    float cellSize = 90.0f; // 上方图标比下方小一点

    auto widget = Widget::create();
    widget->setContentSize(Size(cellSize, cellSize));
    widget->setTouchEnabled(true); // 允许点击整个图标

    // 1. 头像背景 (可选，或者直接用头像)
    auto bg = LayerColor::create(Color4B(0, 0, 0, 50), cellSize, cellSize);
    widget->addChild(bg);

    // 2. 兵种头像
    auto sprite = Sprite::create(config.iconPath);
    if (sprite) {
        float scale = (cellSize - 10) / sprite->getContentSize().width;
        sprite->setScale(scale);
        sprite->setPosition(cellSize / 2, cellSize / 2);
        widget->addChild(sprite);
    }

    // 3. 数量角标 (左上角 x7)
    // 蓝色小背景
    auto numBg = LayerColor::create(Color4B(0, 100, 200, 200), 40, 20);
    numBg->setPosition(0, cellSize - 20);
    widget->addChild(numBg);

    auto numLabel = Label::createWithTTF(StringUtils::format("x%d", count), TRAIN_FONT, 16);
    numLabel->setPosition(20, 10); // 居中于 numBg
    numBg->addChild(numLabel);

    // 4. 红色删除按钮 (右上角 -)
    auto removeBtn = Button::create();
    removeBtn->ignoreContentAdaptWithSize(false);
    removeBtn->setContentSize(Size(25, 25));

    // 红色背景模拟按钮图片
    auto btnColor = LayerColor::create(Color4B::RED, 25, 25);
    btnColor->setTouchEnabled(false);
    removeBtn->addChild(btnColor, -1);

    // 减号文字
    auto minusLabel = Label::createWithSystemFont("-", "Arial", 24, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);
    minusLabel->setPosition(12.5f, 12.5f);
    removeBtn->addChild(minusLabel);

    removeBtn->setPosition(Vec2(cellSize - 12.5f, cellSize - 12.5f));

    // 【交互】点击红色按钮移除
    std::string key = "remove_troop_" + std::to_string(troopId);

    setupLongPressAction(removeBtn, [=]() {
        this->removeTroop(troopId);
        }, key);

    widget->addChild(removeBtn);

    return widget;
}
// ==========================================
// 刷新堆叠显示
// ==========================================
void TrainingLayer::updateArmyView() {
    if (!_armyScrollView) return;

    _armyScrollView->removeAllChildren();

    auto dataManager = VillageDataManager::getInstance();
    auto allConfig = TroopConfig::getInstance()->getAllTroops(); // 获取所有配置以保持顺序

    float startX = 0;
    float padding = 10.0f;
    float cellSize = 90.0f;

    // 遍历所有可能的兵种ID
    for (const auto& info : allConfig) {
        int id = info.id;
        int count = dataManager->getTroopCount(id); // 向 Manager 查询数量

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

// ==========================================
// removeTroop（点击移除）
// ==========================================
void TrainingLayer::removeTroop(int troopId) {
    auto dataManager = VillageDataManager::getInstance();

    // 调用 Manager 移除1个单位
    if (dataManager->removeTroop(troopId, 1)) {
        // 如果移除成功，刷新界面
        updateCapacityLabel();
        updateArmyView();
    }
}

// ==========================================
// 【新增】设置长按连点功能的辅助函数
// ==========================================
void TrainingLayer::setupLongPressAction(Widget* target, std::function<void()> callback, const std::string& scheduleKey) {
    target->addTouchEventListener([=](Ref* sender, Widget::TouchEventType type) {
        if (type == Widget::TouchEventType::BEGAN) {
            // 1. 立即执行一次（保证点一下也能触发）
            callback();

            // 2. 开启定时器：延迟 0.5秒后，每 0.1秒执行一次
            // 参数说明：(回调, 间隔时间, 重复次数, 延迟时间, key)
            this->schedule([=](float) {
                callback();
                }, 0.1f, CC_REPEAT_FOREVER, 0.5f, scheduleKey);
        }
        else if (type == Widget::TouchEventType::ENDED || type == Widget::TouchEventType::CANCELED) {
            // 3. 手指抬起或移出，取消定时器
            this->unschedule(scheduleKey);
        }
        });
}
// ==========================================
// 【新增】底部兵种选择面板
// ==========================================

void TrainingLayer::initTroopSelectionPanel() {
    // 1. 创建 ScrollView
    _scrollView = ScrollView::create();
    _scrollView->setDirection(ScrollView::Direction::HORIZONTAL);
    _scrollView->setContentSize(Size(PANEL_WIDTH - 40, 180)); // 高度刚好放下卡片
    _scrollView->setAnchorPoint(Vec2(0.5f, 0.0f));
    // 放在 _bgNode 的下方 (-250 差不多是 _bgNode 底部再往下一点)
    // 根据 PANEL_HEIGHT=500, _bgNode 中心是(0,0)，bottom是 -250
    _scrollView->setPosition(Vec2(0, -230));
    _scrollView->setScrollBarEnabled(false);
    _scrollView->setBounceEnabled(true);
    _bgNode->addChild(_scrollView);

    // 2. 获取数据
    auto troopList = TroopConfig::getInstance()->getAllTroops();
    int barracksLevel = getMaxBarracksLevel();

    // 如果没有训练营，为了防止空界面，默认至少展示1级兵
    if (barracksLevel == 0) barracksLevel = 1;

    // 3. 动态添加卡片
    float startX = 0;
    float padding = 10;

    for (const auto& troop : troopList) {
        // 判断解锁条件
        // 规则：当前训练营等级 >= 兵种需要的解锁等级
        // 为了方便您测试，您可以暂时注释掉下面这行逻辑，改为 true
        bool isUnlocked = (barracksLevel >= troop.unlockBarracksLvl);
        // bool isUnlocked = true; // 【调试模式：全解锁】

        auto card = createTroopCard(troop, isUnlocked);
        // 卡片锚点0.5, 0.5
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

    // 1. 卡片背景 (灰色圆角矩形模拟)
    auto bg = LayerColor::create(Color4B(80, 80, 80, 255), CARD_WIDTH, CARD_HEIGHT);
    bg->setPosition(0, 0);
    widget->addChild(bg);

    // 2. 兵种头像
    auto sprite = Sprite::create(info.iconPath);
    if (sprite) {
        // 缩放适应卡片
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

    // 3. 左下角：等级 (Lv.1)
    auto levelBg = LayerColor::create(Color4B::BLACK, 40, 20);
    levelBg->setPosition(5, 5);
    widget->addChild(levelBg);

    auto lvlLabel = Label::createWithTTF(StringUtils::format("Lv.%d", info.level), TRAIN_FONT, 14);
    lvlLabel->setPosition(20, 10);
    levelBg->addChild(lvlLabel);

    // 4. 右下角：人口空间 (如 5) - 纯文字
    auto spaceBg = LayerColor::create(Color4B(0, 0, 0, 150), 30, 20);
    spaceBg->setPosition(CARD_WIDTH - 35, 5);
    widget->addChild(spaceBg);

    auto spaceLabel = Label::createWithTTF(StringUtils::format("%d", info.housingSpace), TRAIN_FONT, 14);
    spaceLabel->setColor(Color3B(200, 200, 255)); // 稍微带点蓝
    spaceLabel->setPosition(15, 10);
    spaceBg->addChild(spaceLabel);

    // 5. 右上角：信息按钮 i
    auto infoBtn = Button::create("UI/training-camp/troop-cards/info_btn.png");
    if (infoBtn) {
        infoBtn->setPosition(Vec2(CARD_WIDTH - 15, CARD_HEIGHT - 15));
        infoBtn->setScale(0.1f);
        infoBtn->addClickEventListener([=](Ref*) {
            this->onInfoClicked(info.id);
            });
        widget->addChild(infoBtn);
    }

    // 6. 点击卡片本身 -> 训练
    if (isUnlocked) {
        // 使用唯一的 key，防止不同兵种定时器冲突
        std::string key = "train_troop_" + std::to_string(info.id);

        setupLongPressAction(widget, [=]() {
            this->onTroopCardClicked(info.id);
            }, key);
    }
    else {
        // 未解锁点击提示
        widget->addClickEventListener([=](Ref*) {
            CCLOG("Troop locked! Need Barracks Lv.%d", info.unlockBarracksLvl);
            });
    }

    return widget;
}

// ==========================================
// 交互逻辑
// ==========================================

void TrainingLayer::onTroopCardClicked(int troopId) {
    auto dataManager = VillageDataManager::getInstance();
    TroopInfo info = TroopConfig::getInstance()->getTroopById(troopId);

    // 1. 从 Manager 获取当前占用和总容量
    int currentSpace = dataManager->getCurrentHousingSpace();
    int maxSpace = dataManager->calculateTotalHousingSpace();

    // 2. 检查人口
    if (currentSpace + info.housingSpace > maxSpace) {
        // 弹出提示
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

    // 3. 【无消耗】直接训练：调用 Manager 添加兵种
    dataManager->addTroop(troopId, 1);

    // 4. 刷新 UI
    updateCapacityLabel();
    updateArmyView();
}

void TrainingLayer::updateCapacityLabel() {
    auto dataManager = VillageDataManager::getInstance();

    // 实时计算
    int current = dataManager->getCurrentHousingSpace();
    int max = dataManager->calculateTotalHousingSpace();

    if (_capacityLabel) {
        _capacityLabel->setString(StringUtils::format("%d/%d", current, max));
    }
}

void TrainingLayer::onInfoClicked(int troopId) {
    TroopInfo info = TroopConfig::getInstance()->getTroopById(troopId);

    // 1. 全屏遮罩 (半透明黑色，阻挡点击)
    auto shieldLayer = LayerColor::create(Color4B(0, 0, 0, 180));
    this->addChild(shieldLayer, 200); // ZOrder 200 保证在最顶层

    // 点击遮罩外的空白处关闭
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [=](Touch* t, Event* e) {
        // 这里为了体验更好，改为点击右上角X关闭，防止误触，
        // 但为了方便测试，保留点击黑色背景关闭的功能
        shieldLayer->removeFromParent();
        return true;
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, shieldLayer);

    // 2. 弹窗主面板 (米白色背景，模仿 COC 风格)
    float popupW = 600;
    float popupH = 400;
    auto bg = LayerColor::create(Color4B(210, 200, 190, 255), popupW, popupH);
    bg->ignoreAnchorPointForPosition(false);
    bg->setAnchorPoint(Vec2(0.5f, 0.5f));

    // 居中显示
    auto visibleSize = Director::getInstance()->getVisibleSize();
    bg->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    shieldLayer->addChild(bg);

    // --- 3. 顶部标题栏 ---
    auto titleBg = LayerColor::create(Color4B(180, 160, 140, 255), popupW, 50);
    titleBg->setPosition(0, popupH - 50);
    bg->addChild(titleBg);

    // 标题文字 (比如 "1级 野蛮人")
    auto titleLabel = Label::createWithTTF(StringUtils::format("%d级 %s", info.level, info.name.c_str()), TRAIN_FONT, 24);
    if (!titleLabel) titleLabel = Label::createWithSystemFont(info.name, "Arial", 24);
    titleLabel->setPosition(popupW / 2, popupH - 25);
    titleLabel->setColor(Color3B::BLACK);
    titleLabel->enableBold();
    bg->addChild(titleLabel);

    // 关闭按钮 (X)
    auto closeBtn = Button::create();
    closeBtn->setTitleText("X");
    closeBtn->setTitleFontSize(30);
    closeBtn->setTitleColor(Color3B::RED);
    closeBtn->setPosition(Vec2(popupW - 30, popupH - 25));
    closeBtn->addClickEventListener([=](Ref*) {
        shieldLayer->removeFromParent();
        });
    bg->addChild(closeBtn);

    // --- 4. 左侧：兵种大图 ---
    auto sprite = Sprite::create(info.iconPath);
    if (sprite) {
        sprite->setScale(1.5f); // 放大一点
        // 放在左侧中心位置
        sprite->setPosition(popupW * 0.25f, popupH * 0.55f);
        bg->addChild(sprite);
    }

    // --- 5. 右侧：属性列表 ---
    // 我们用一个简单的排版逻辑：起始X在中间偏右，起始Y在上面，每行往下排
    float startX = popupW * 0.5f;
    float startY = popupH - 80;
    float lineHeight = 35;
    int lineCount = 0;

    // 辅助 lambda：快速创建一行属性 "标签: 数值"
    auto addStatRow = [&](const std::string& key, const std::string& value) {
        float y = startY - (lineCount * lineHeight);

        // 属性名 (灰色)
        auto keyLabel = Label::createWithTTF(key, TRAIN_FONT, 20);
        if (!keyLabel) keyLabel = Label::createWithSystemFont(key, "Arial", 20);
        keyLabel->setAnchorPoint(Vec2(0, 0.5f));
        keyLabel->setPosition(startX, y);
        keyLabel->setColor(Color3B(80, 80, 80)); // 深灰
        bg->addChild(keyLabel);

        // 属性值 (黑色)
        auto valLabel = Label::createWithTTF(value, TRAIN_FONT, 20);
        if (!valLabel) valLabel = Label::createWithSystemFont(value, "Arial", 20);
        valLabel->setAnchorPoint(Vec2(0, 0.5f));
        valLabel->setPosition(startX + 110, y); // 向右偏移一些
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

    // --- 6. 底部描述文本 ---
    // 分割线
    auto line = LayerColor::create(Color4B(150, 150, 150, 255), popupW - 40, 2);
    line->setPosition(20, 70);
    bg->addChild(line);

    // 描述文字
    auto descLabel = Label::createWithTTF(info.description, TRAIN_FONT, 16);
    if (!descLabel) descLabel = Label::createWithSystemFont(info.description, "Arial", 16);
    descLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
    descLabel->setPosition(popupW / 2, 35); // 底部区域中心
    descLabel->setDimensions(popupW - 40, 60); // 限制宽度自动换行
    descLabel->setAlignment(TextHAlignment::CENTER, TextVAlignment::CENTER);
    descLabel->setColor(Color3B(60, 60, 60));
    bg->addChild(descLabel);
}

int TrainingLayer::getMaxBarracksLevel() {
    auto dataManager = VillageDataManager::getInstance();
    if (!dataManager) return 1;

    // 遍历所有建筑找到最高等级的训练营 (Type ID 102)
    auto buildings = dataManager->getAllBuildings();
    int maxLvl = 0;

    for (const auto& b : buildings) {
        // 假设训练营ID是102，且状态必须是 BUILT (已建成)
        if (b.type == 102 && b.state == BuildingInstance::State::BUILT) {
            if (b.level > maxLvl) maxLvl = b.level;
        }
    }

    // 如果没有训练营，可能是新号，返回1级防止逻辑卡死，或者返回0表示都锁住
    // 这里为了测试方便返回1
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
        // 切换显示状态：如果是显示的就隐藏，隐藏的就显示
        bool isVisible = _scrollView->isVisible();

        if (!isVisible) {
            // 如果要显示，加个简单的弹入动画
            _scrollView->setVisible(true);
            _scrollView->setScale(0.1f);
            _scrollView->runAction(EaseBackOut::create(ScaleTo::create(0.2f, 1.0f)));
        }
        else {
            // 如果要隐藏
            _scrollView->setVisible(false);
        }
    }
}

void TrainingLayer::onSpellsSectionClicked() {
    CCLOG("点击了【法术】模块 - 暂未实现");
}