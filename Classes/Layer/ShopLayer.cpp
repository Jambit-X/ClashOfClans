// 【必须加在第一行】强制使用 UTF-8 编码，解决中文乱码
#pragma execution_character_set("utf-8")

#include "Manager/VillageDataManager.h"
#include "Layer/VillageLayer.h"
#include "ShopLayer.h"
#include "Manager/BuildingManager.h" 
#include "Layer/HUDLayer.h" 

USING_NS_CC;
using namespace ui;

// 统一管理字体路径
const std::string FONT_PATH = "fonts/simhei.ttf";

Scene* ShopLayer::createScene() {
    auto scene = Scene::create();
    auto layer = ShopLayer::create();
    scene->addChild(layer);
    return scene;
}

bool ShopLayer::init() {
    if (!Layer::init()) {
        return false;
    }
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 1. 黑色半透明遮罩
    auto shieldLayer = LayerColor::create(Color4B(0, 0, 0, 200));
    this->addChild(shieldLayer);

    // 吞噬触摸事件
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch* t, Event* e) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 2. 关闭按钮 (放在右上角)
    auto closeBtn = Button::create();
    closeBtn->setTitleText("X");
    closeBtn->setTitleFontName(FONT_PATH);
    closeBtn->setTitleFontSize(40);
    closeBtn->setTitleColor(Color3B::RED);
    closeBtn->setPosition(Vec2(visibleSize.width - 50, visibleSize.height - 40));
    closeBtn->addClickEventListener(CC_CALLBACK_1(ShopLayer::onCloseClicked, this));
    this->addChild(closeBtn, 10);

    // 3. 初始化各模块
    initScrollView();
    initTabs();
    initBottomBar();

    // 默认进入“军队”标签
    switchTab(0);

    return true;
}

void ShopLayer::initTabs() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    std::vector<std::string> titles = { "军队", "资源", "防御", "陷阱" };
    int tabCount = titles.size();

    // --- 计算整体布局，使其居中 ---
    float totalWidth = (tabCount * TAB_WIDTH) + ((tabCount - 1) * TAB_SPACING);

    // 计算起始 X 坐标 (屏幕宽 - 总宽) / 2
    float startX = (visibleSize.width - totalWidth) / 2;

    // 计算 Y 坐标：整体下移，不要贴顶
    // visibleSize.height - 130 (TAB_TOP_OFFSET)
    float startY = visibleSize.height - TAB_TOP_OFFSET;

    for (int i = 0; i < tabCount; ++i) {
        auto btn = Button::create();

        // 【关键】禁止自动适配内容大小，强制使用 setContentSize 的尺寸
        // 这样文字就会在 180x50 的框里自动居中
        btn->ignoreContentAdaptWithSize(false);
        btn->setContentSize(Size(TAB_WIDTH, TAB_HEIGHT));

        // 添加纯色背景层
        auto bgLayer = LayerColor::create(COLOR_TAB_NORMAL, TAB_WIDTH, TAB_HEIGHT);
        bgLayer->setPosition(Vec2(0, 0));
        bgLayer->setTag(999);
        bgLayer->setTouchEnabled(false); // 这一层不响应触摸，让 Button 响应
        btn->addChild(bgLayer, -1);      // 放在文字下面

        // 设置文字
        btn->setTitleText(titles[i]);
        btn->setTitleFontName(FONT_PATH);
        btn->setTitleFontSize(24);
        btn->setTitleColor(Color3B::WHITE);

        // 设置位置 (锚点设为左下角)
        btn->setAnchorPoint(Vec2(0, 0));
        float xPos = startX + i * (TAB_WIDTH + TAB_SPACING);
        btn->setPosition(Vec2(xPos, startY));

        btn->addClickEventListener([=](Ref* sender) {
            this->onTabClicked(sender, i);
            });

        this->addChild(btn);
        _tabButtons.push_back(btn);
    }
}

void ShopLayer::initScrollView() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    _scrollView = ScrollView::create();
    _scrollView->setDirection(ScrollView::Direction::HORIZONTAL);
    _scrollView->setBounceEnabled(true);

    // --- 调整列表区域，紧贴标签下方 ---
    // 标签 Y = height - 130，高度 50。标签底部 = height - 130。
    // 我们让列表的顶部在 height - 140，留 10 像素缝隙。
    float topY = visibleSize.height - (TAB_TOP_OFFSET + 10);
    float bottomY = 80; // 底部留给资源条
    float height = topY - bottomY;

    _scrollView->setContentSize(Size(visibleSize.width, height));
    _scrollView->setAnchorPoint(Vec2(0, 0));
    _scrollView->setPosition(Vec2(0, bottomY));

    this->addChild(_scrollView);
}

void ShopLayer::initBottomBar() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 底部绿色条
    auto bar = LayerColor::create(Color4B(80, 180, 50, 255));
    bar->setContentSize(Size(visibleSize.width, 80));
    bar->setPosition(Vec2(0, 0));
    this->addChild(bar, 5);

    // 从单例获取真实资源数据
    auto dataManager = VillageDataManager::getInstance();
    int gold = dataManager->getGold();
    int elixir = dataManager->getElixir();

    // 资源文字
    std::string resourceText = "金币: " + std::to_string(gold) +
      "   圣水: " + std::to_string(elixir) +
      "   宝石: 100"; // 宝石暂时硬编码

    auto label = Label::createWithTTF(resourceText, FONT_PATH, 28);
    label->setPosition(Vec2(visibleSize.width / 2, 40));
    label->enableOutline(Color4B::BLACK, 2);
    bar->addChild(label);
}

void ShopLayer::switchTab(int categoryIndex) {
    // 选中时稍微凸起的高度
    float selectedHeight = TAB_HEIGHT + 10.0f;

    // 1. 刷新标签外观
    for (int i = 0; i < _tabButtons.size(); ++i) {
        auto btn = _tabButtons[i];
        bool isSelected = (i == categoryIndex);

        float targetHeight = isSelected ? selectedHeight : TAB_HEIGHT;

        // 【关键】更新按钮尺寸，保证文字始终居中
        btn->setContentSize(Size(TAB_WIDTH, targetHeight));

        // 更新背景颜色块
        auto bgLayer = dynamic_cast<LayerColor*>(btn->getChildByTag(999));
        if (bgLayer) {
            Color4B targetColor = isSelected ? COLOR_TAB_SELECT : COLOR_TAB_NORMAL;
            bgLayer->setColor(Color3B(targetColor.r, targetColor.g, targetColor.b));
            bgLayer->setContentSize(Size(TAB_WIDTH, targetHeight));
        }

        // 更新文字颜色 (选中黑字，未选中白字)
        btn->setTitleColor(isSelected ? Color3B::BLACK : Color3B::WHITE);
    }

    // 2. 获取数据并刷新列表
    auto items = getDummyData(categoryIndex);

    _scrollView->removeAllChildren();

    float cardWidth = 180;
    float cardHeight = 260;
    float margin = 20;

    // 计算内容总宽度
    float innerWidth = items.size() * (cardWidth + margin) + margin;
    if (innerWidth < _scrollView->getContentSize().width)
        innerWidth = _scrollView->getContentSize().width;

    _scrollView->setInnerContainerSize(Size(innerWidth, _scrollView->getContentSize().height));

    for (int i = 0; i < items.size(); ++i) {
        addShopItem(items[i], i);
    }
}

void ShopLayer::addShopItem(const ShopItemData& data, int index) {
    float cardWidth = 180;
    float cardHeight = 260;
    float margin = 20;

    // 1. 卡片背景
    auto bg = LayerColor::create(Color4B(40, 40, 40, 200));
    bg->setContentSize(Size(cardWidth, cardHeight));

    // 垂直居中于 ScrollView
    float y = (_scrollView->getContentSize().height - cardHeight) / 2;
    float x = margin + index * (cardWidth + margin);
    bg->setPosition(Vec2(x, y));

    _scrollView->addChild(bg);

    // 2. 建筑图片
    auto sprite = Sprite::create(data.imagePath);
    if (sprite) {
        float maxImgSize = 140;
        float scale = maxImgSize / std::max(sprite->getContentSize().width, sprite->getContentSize().height);
        if (scale > 1.0f) scale = 1.0f;

        sprite->setScale(scale);
        sprite->setPosition(Vec2(cardWidth / 2, cardHeight / 2 + 20));
        bg->addChild(sprite);
    }
    else {
        auto err = LayerColor::create(Color4B::RED, 100, 100);
        err->setPosition(Vec2(40, 80));
        bg->addChild(err);
    }

    // 3. 信息按钮
    auto infoBtn = Label::createWithTTF("i", FONT_PATH, 18);
    infoBtn->setPosition(Vec2(cardWidth - 20, cardHeight - 20));
    infoBtn->setColor(Color3B::GRAY);
    bg->addChild(infoBtn);

    // 4. 名称
    auto nameLabel = Label::createWithTTF(data.name, FONT_PATH, 18);
    nameLabel->setAnchorPoint(Vec2(0, 1));
    nameLabel->setPosition(Vec2(10, cardHeight - 10));
    bg->addChild(nameLabel);

    // 5. 建造时间
    auto timeLabel = Label::createWithTTF("建造时间： " + data.time, FONT_PATH, 14);
    timeLabel->setAnchorPoint(Vec2(0, 0));
    timeLabel->setPosition(Vec2(10, 60));
    bg->addChild(timeLabel);

    // 6. 价格
    std::string priceStr = std::to_string(data.cost) + " " + data.costType;
    auto priceLabel = Label::createWithTTF(priceStr, FONT_PATH, 20);
    priceLabel->setPosition(Vec2(cardWidth / 2, 30));

    if (data.costType == "金币") priceLabel->setColor(Color3B::YELLOW);
    else if (data.costType == "圣水") priceLabel->setColor(Color3B::MAGENTA);
    else priceLabel->setColor(Color3B::GREEN);

    priceLabel->enableOutline(Color4B::BLACK, 2);
    bg->addChild(priceLabel);

    // 7. 点击交互
    auto touchBtn = Button::create();
    touchBtn->setScale9Enabled(true);
    touchBtn->setContentSize(Size(cardWidth, cardHeight));
    touchBtn->setPosition(Vec2(cardWidth / 2, cardHeight / 2));
    touchBtn->setOpacity(0);
    touchBtn->addClickEventListener([=](Ref*) {
        CCLOG("购买了 %s", data.name.c_str());
        this->onPurchaseBuilding(data);
        });
    bg->addChild(touchBtn);
}

std::vector<ShopItemData> ShopLayer::getDummyData(int categoryIndex) {
    std::vector<ShopItemData> list;
    std::string root = "UI/Shop/"; 

    if (categoryIndex == 0) { // 军队1
        std::string path = root + "military_architecture/";
        list.push_back({ 101, "兵营",       path + "Army_Camp1.png",    250,  "圣水", "5分钟" });
        list.push_back({ 102, "训练营",     path + "Barracks1.png",     200,  "圣水", "1分钟" });
        list.push_back({ 103, "实验室",     path + "Laboratory1.png",   500,  "圣水", "30分钟" });
        list.push_back({ 104, "法术工厂",   path + "Spell_Factory1.png",1500, "圣水", "1小时" });
        list.push_back({ 105, "暗黑训练营", path + "Dark_Barracks1.png",2000, "圣水", "4小时" });
    }
    else if (categoryIndex == 1) { // 资源
        std::string path = root + "resource_architecture/";
        list.push_back({ 201, "建筑工人",     path + "Builders_Hut1.png",       500, "宝石", "0秒" });
        list.push_back({ 202, "金矿",         path + "Gold_Mine1.png",          150, "圣水", "1分钟" });
        list.push_back({ 203, "圣水收集器",   path + "Elixir_Collector1.png",   150, "金币", "1分钟" });
        list.push_back({ 204, "储金罐",       path + "Gold_Storage1.png",       300, "圣水", "15分钟" });
        list.push_back({ 205, "圣水瓶",       path + "Elixir_Storage1.png",     300, "金币", "15分钟" });
        list.push_back({ 206, "暗黑重油钻井", path + "Dark_Elixir_Drill1.png",  1000,"圣水", "1小时" });
    }
    else if (categoryIndex == 2) { // 防御
        std::string path = root + "defence_architecture/";
        list.push_back({ 301, "加农炮",     path + "Cannon_lvl1.png",     250, "金币", "1分钟" });
        list.push_back({ 302, "箭塔",       path + "Archer_Tower1.png",   1000,"金币", "15分钟" });
        list.push_back({ 303, "城墙",       path + "Wall1.png",           50,  "金币", "0秒" });
        list.push_back({ 304, "迫击炮",     path + "Mortar1.png",         5000,"金币", "3小时" });
        list.push_back({ 305, "防空火箭",   path + "Air_Defense1.png",    2000,"金币", "1小时" });
        list.push_back({ 306, "法师塔",     path + "Wizard_Tower1.png",   3000,"金币", "2小时" });
        list.push_back({ 307, "空气炮",     path + "Air_Sweeper1.png",    4000,"金币", "2小时" });
    }
    else { // 陷阱
        std::string path = root + "trap/";
        list.push_back({ 401, "炸弹",       path + "Bomb1.png",             400,  "金币", "0秒" });
        list.push_back({ 402, "隐形弹簧",   path + "Spring_Trap1.png",      2000, "金币", "0秒" });
        list.push_back({ 403, "空中炸弹",   path + "Air_Bomb1.png",         4000, "金币", "0秒" });
        list.push_back({ 404, "巨型炸弹",   path + "Giant_Bomb1.png",       12500,"金币", "0秒" });
        list.push_back({ 405, "搜空地雷",   path + "Seeking_Air_Mine1.png", 10000,"金币", "0秒" });
    }

    return list;
}

// 购买调用函数
void ShopLayer::onPurchaseBuilding(const ShopItemData& data) {
  auto dataManager = VillageDataManager::getInstance();

  // ========== 兵营购买限制逻辑 ==========
  if (data.id == 101) { // 101 是兵营
      int thLevel = dataManager->getTownHallLevel();
      int currentCamps = dataManager->getArmyCampCount();

      // 规则：当前拥有的兵营数 必须 < 大本营等级
      // 例如：1级大本营，如果已有1个，则 1 < 1 为假，不可购买
      if (currentCamps >= thLevel) {
          CCLOG("ShopLayer: Army Camp limit reached (TH Lv.%d, Camps: %d)", thLevel, currentCamps);

          auto label = Label::createWithTTF("请升级大本营以解锁更多兵营！", "fonts/simhei.ttf", 24);
          label->setPosition(Director::getInstance()->getVisibleSize() / 2);
          label->setColor(Color3B::RED);
          label->enableOutline(Color4B::BLACK, 2);
          this->addChild(label, 100);

          label->runAction(Sequence::create(
              DelayTime::create(2.0f),
              FadeOut::create(0.5f),
              RemoveSelf::create(),
              nullptr
          ));
          return; // 阻止购买
      }
  }
  // ========================================
  bool success = false;

  if (data.costType == "金币") {
    success = dataManager->spendGold(data.cost);
  } else if (data.costType == "圣水") {
    success = dataManager->spendElixir(data.cost);
  } else if (data.costType == "宝石") {
    CCLOG("宝石购买尚未实现");
    return;
  }

  if (success) {
    CCLOG("购买成功: %s", data.name.c_str());

    // 添加建筑数据（状态为 PLACING）
    int buildingId = dataManager->addBuilding(
      data.id,
      1,
      22, 22,  // 默认放在地图中心
      BuildingInstance::State::PLACING,
      0
    );

    // 关闭商店
    this->onCloseClicked(nullptr);

    // 通知 VillageLayer 开始放置
    auto scene = this->getScene();
    if (scene) {
      auto villageLayer = dynamic_cast<VillageLayer*>(scene->getChildByTag(1));
      if (villageLayer) {
        villageLayer->onBuildingPurchased(buildingId);
      }

      // 显示放置UI
      auto hudLayer = dynamic_cast<HUDLayer*>(scene->getChildByTag(100));
      if (hudLayer) {
        hudLayer->showPlacementUI(buildingId);
      }
    }
  } else {
    CCLOG("资源不足，无法购买 %s", data.name.c_str());

    auto label = Label::createWithTTF("资源不足！", "fonts/simhei.ttf", 30);
    label->setPosition(Director::getInstance()->getVisibleSize() / 2);
    label->setColor(Color3B::RED);
    this->addChild(label, 100);

    label->runAction(Sequence::create(
      FadeOut::create(1.5f),
      RemoveSelf::create(),
      nullptr
    ));
  }
}

void ShopLayer::onCloseClicked(Ref* sender) {
    this->runAction(Sequence::create(
        ScaleTo::create(0.1f, 0.01f),
        RemoveSelf::create(),
        nullptr
    ));
}

void ShopLayer::onTabClicked(Ref* sender, int index) {
    switchTab(index);
}