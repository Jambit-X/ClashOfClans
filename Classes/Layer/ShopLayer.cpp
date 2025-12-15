// 【必须加在第一行】强制使用 UTF-8 编码，解决中文乱码
#pragma execution_character_set("utf-8")

#include "Manager/VillageDataManager.h"
#include "Layer/VillageLayer.h"
#include "ShopLayer.h"
#include "Manager/BuildingManager.h" 
#include "Layer/HUDLayer.h" 
#include "Model/BuildingRequirements.h" 
#include "Model/BuildingConfig.h"

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

    // 监听大本营升级事件
    auto townHallListener = EventListenerCustom::create("EVENT_TOWNHALL_UPGRADED",
                                                        [this](EventCustom* event) {
      CCLOG("ShopLayer: Town Hall upgraded, refreshing shop");
      // 刷新当前标签的商店列表
      if (!_tabButtons.empty()) {
        // 找到当前选中的标签
        for (int i = 0; i < _tabButtons.size(); ++i) {
          auto btn = _tabButtons[i];
          auto bgLayer = dynamic_cast<LayerColor*>(btn->getChildByTag(999));
          if (bgLayer && bgLayer->getColor() == Color3B(COLOR_TAB_SELECT.r, COLOR_TAB_SELECT.g, COLOR_TAB_SELECT.b)) {
            switchTab(i);  // 重新加载当前标签
            break;
          }
        }
      }
    });
    _eventDispatcher->addEventListenerWithSceneGraphPriority(townHallListener, this);
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

  // ========== 获取建筑信息 ==========
  auto dataManager = VillageDataManager::getInstance();
  auto requirements = BuildingRequirements::getInstance();

  int currentTHLevel = dataManager->getTownHallLevel();
  int currentCount = 0;
  for (const auto& b : dataManager->getAllBuildings()) {
    if (b.type == data.id && b.state != BuildingInstance::State::PLACING) {
      currentCount++;
    }
  }

  bool isLocked = !requirements->canPurchase(data.id, currentTHLevel, currentCount);

  // 获取最大数量和解锁大本营等级
  int maxCount = requirements->getMaxCount(data.id, currentTHLevel);
  int minTHLevel = requirements->getMinTHLevel(data.id);
  // ==========================================

  // 1. 卡片背景
  auto bg = LayerColor::create(Color4B(40, 40, 40, 200));
  bg->setContentSize(Size(cardWidth, cardHeight));

  // 垂直居中于 ScrollView
  float y = (_scrollView->getContentSize().height - cardHeight) / 2;
  float x = margin + index * (cardWidth + margin);
  bg->setPosition(Vec2(x, y));

  _scrollView->addChild(bg);

  // ========== 如果锁定，整体变暗 ==========
  if (isLocked) {
    bg->setColor(Color3B(80, 80, 80));
  }
  // ========================================

  // ========== 修复：角标显示逻辑 ==========
  // 判断锁定原因：是因为大本营等级不足，还是数量已满
  bool isDueToTownHall = (currentTHLevel < minTHLevel);  // 大本营等级不足
  bool isDueToCount = (currentCount >= maxCount);         // 数量已达上限

  if (isDueToTownHall && !isDueToCount) {
    // 情况1：大本营等级不足（且数量未满）- 显示解锁大本营等级
    auto unlockBg = LayerColor::create(Color4B(200, 50, 50, 230), 85, 26);
    unlockBg->setAnchorPoint(Vec2(0.5f, 0));
    unlockBg->setPosition(Vec2(cardWidth / 2, cardHeight));
    bg->addChild(unlockBg, 5);

    std::string unlockText = "大本" + std::to_string(minTHLevel) + "级";
    auto unlockLabel = Label::createWithTTF(unlockText, FONT_PATH, 14);
    unlockLabel->setPosition(Vec2(42.5f, 13));
    unlockLabel->setColor(Color3B::WHITE);
    unlockLabel->enableOutline(Color4B::BLACK, 1);
    unlockBg->addChild(unlockLabel);
  } else {
    // 情况2：大本营已解锁（无论数量是否已满）- 显示数量信息
    // 根据是否达到上限选择背景颜色
    Color4B bgColor = isDueToCount
      ? Color4B(220, 120, 0, 230)   // 橙色警告（数量已满）
      : Color4B(50, 180, 80, 230);  // 绿色正常（可继续购买）

    auto countBg = LayerColor::create(bgColor, 70, 26);
    countBg->setAnchorPoint(Vec2(0.5f, 0));
    countBg->setPosition(Vec2(cardWidth / 2, cardHeight));
    bg->addChild(countBg, 5);

    std::string countText = std::to_string(currentCount) + "/" + std::to_string(maxCount);
    auto countLabel = Label::createWithTTF(countText, FONT_PATH, 16);
    countLabel->setPosition(Vec2(35, 13));
    countLabel->setColor(Color3B::WHITE);
    countLabel->enableOutline(Color4B::BLACK, 1);
    countBg->addChild(countLabel);
  }
  // ===========================================================

  // 2. 建筑图片
  auto sprite = Sprite::create(data.imagePath);
  if (sprite) {
    float maxImgSize = 130;
    float scale = maxImgSize / std::max(sprite->getContentSize().width, sprite->getContentSize().height);
    if (scale > 1.0f) scale = 1.0f;

    sprite->setScale(scale);
    sprite->setPosition(Vec2(cardWidth / 2, cardHeight / 2 + 15));

    if (isLocked) {
      sprite->setColor(Color3B(100, 100, 100));
      sprite->setOpacity(180);
    }

    bg->addChild(sprite);
  } else {
    auto err = LayerColor::create(Color4B::RED, 100, 100);
    err->setPosition(Vec2(40, 80));
    bg->addChild(err);
  }

  // 3. 信息按钮
  auto infoBtn = Label::createWithTTF("i", FONT_PATH, 16);
  infoBtn->setPosition(Vec2(cardWidth - 15, cardHeight - 15));
  infoBtn->setColor(Color3B::GRAY);
  bg->addChild(infoBtn);

  // 4. 名称
  auto nameLabel = Label::createWithTTF(data.name, FONT_PATH, 17);
  nameLabel->setAnchorPoint(Vec2(0, 1));
  nameLabel->setPosition(Vec2(8, cardHeight - 8));
  bg->addChild(nameLabel);

  // 5. 建造时间
  auto timeLabel = Label::createWithTTF("时间: " + data.time, FONT_PATH, 13);
  timeLabel->setAnchorPoint(Vec2(0, 0));
  timeLabel->setPosition(Vec2(8, 55));
  timeLabel->setColor(Color3B(200, 200, 200));
  bg->addChild(timeLabel);

  // ========== 如果锁定，显示锁图标和解锁条件 ==========
  if (isLocked) {
    // 锁图标
    auto lockLabel = Label::createWithSystemFont("🔒", "Arial", 40);
    lockLabel->setPosition(Vec2(cardWidth / 2, cardHeight / 2 + 30));
    lockLabel->setColor(Color3B(255, 200, 0));
    bg->addChild(lockLabel, 10);

    // ========== 修复：根据锁定原因显示不同的提示 ==========
    std::string reason;
    if (isDueToTownHall) {
      // 大本营等级不足
      reason = "需要" + std::to_string(minTHLevel) + "级大本营\n才能解锁";
    } else if (isDueToCount) {
      // 数量已达上限
      reason = "数量已达上限\n最多" + std::to_string(maxCount) + "个";
    }

    auto lockReasonLabel = Label::createWithTTF(reason, FONT_PATH, 13);
    lockReasonLabel->setPosition(Vec2(cardWidth / 2, cardHeight / 2 - 10));
    lockReasonLabel->setColor(Color3B::RED);
    lockReasonLabel->setDimensions(cardWidth - 20, 45);
    lockReasonLabel->setAlignment(cocos2d::TextHAlignment::CENTER);
    lockReasonLabel->setLineHeight(18);
    lockReasonLabel->enableOutline(Color4B::BLACK, 1);
    bg->addChild(lockReasonLabel, 10);
  }
  // ===========================================================

  // 6. 价格
  std::string priceStr = std::to_string(data.cost) + " " + data.costType;
  auto priceLabel = Label::createWithTTF(priceStr, FONT_PATH, 20);
  priceLabel->setPosition(Vec2(cardWidth / 2, 28));

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

  if (isLocked) {
    touchBtn->setEnabled(false);
  } else {
    touchBtn->addClickEventListener([=](Ref*) {
      CCLOG("购买了 %s", data.name.c_str());
      this->onPurchaseBuilding(data);
    });
  }

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
        list.push_back({ 201, "建筑工人",     path + "Builders_Hut1.png",       50, "宝石", "0秒" });
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
  CCLOG("ShopLayer: Purchase button clicked for %s", data.name.c_str());

  auto dataManager = VillageDataManager::getInstance();
  auto requirements = BuildingRequirements::getInstance();

  // ========== 检查0：工人是否空闲（特殊建筑例外）==========
  // 城墙（303）和建筑工人小屋（201）不需要工人
  if (data.id != 303 && data.id != 201) {
    if (!dataManager->hasIdleWorker()) {
      int idle = dataManager->getIdleWorkerCount();
      int total = dataManager->getTotalWorkers();

      std::string tip = "所有工人都在忙碌！\n";
      tip += "空闲工人: " + std::to_string(idle) + "/" + std::to_string(total);
      tip += "\n请购买建筑工人小屋（50宝石）";

      showTips(tip, Color3B::ORANGE);
      return;
    }
  }

  // 检查大本营等级限制
  int currentTHLevel = dataManager->getTownHallLevel();
  int requiredTHLevel = requirements->getMinTHLevel(data.id);

  if (currentTHLevel < requiredTHLevel) {
    std::string msg = "需要大本营 " + std::to_string(requiredTHLevel) + " 级!";
    showTips(msg, Color3B::RED);
    return;
  }

  // 检查数量限制
  int currentCount = 0;
  for (const auto& building : dataManager->getAllBuildings()) {
    if (building.type == data.id && building.state != BuildingInstance::State::PLACING) {
      currentCount++;
    }
  }

  int maxCount = requirements->getMaxCount(data.id, currentTHLevel);
  if (currentCount >= maxCount) {
    std::string msg = "已达到最大数量限制! (" + std::to_string(maxCount) + ")";
    showTips(msg, Color3B::RED);
    return;
  }

  // 检查资源
  auto config = BuildingConfig::getInstance()->getConfig(data.id);
  if (!config) return;

  int cost = config->initialCost;
  if (config->costType == "gold" && dataManager->getGold() < cost) {
    showTips("金币不足!", Color3B::RED);
    return;
  }
  if (config->costType == "elixir" && dataManager->getElixir() < cost) {
    showTips("圣水不足!", Color3B::RED);
    return;
  }
  // ========== 特殊处理：建筑工人小屋（完全模仿其他建筑）==========
  if (data.id == 201) {
    CCLOG("ShopLayer: Purchasing builder hut");

    // 扣除宝石（和其他建筑一样的逻辑）
    if (!dataManager->spendGem(cost)) {
      showTips("宝石不足!", Color3B::RED);
      return;
    }

    // 创建建筑（PLACING 状态，需要玩家放置）
    int buildingId = dataManager->addBuilding(
      data.id, 0, 0, 1, BuildingInstance::State::PLACING
    );

    if (buildingId < 0) {
      showTips("购买失败!", Color3B::RED);
      return;
    }

    CCLOG("ShopLayer: Builder hut created, ID=%d, entering placement mode", buildingId);

    // 通知 VillageLayer 启动放置流程（和其他建筑完全一样）
    auto scene = this->getScene();
    if (scene) {
      auto villageLayer = dynamic_cast<VillageLayer*>(scene->getChildByTag(1));
      if (villageLayer) {
        villageLayer->onBuildingPurchased(buildingId);
      }
    }

    // 关闭商店
    this->removeFromParent();
    return;
  }
  // ========== 核心修改：检测城墙购买，进入连续建造模式 ==========
  if (data.id == 303) {  // 城墙的 ID
    CCLOG("ShopLayer: Wall purchase detected, entering continuous build mode");

    // 获取 HUDLayer
    auto scene = this->getScene();
    if (scene) {
      auto hudLayer = dynamic_cast<HUDLayer*>(scene->getChildByTag(100));
      if (hudLayer) {
        // 进入连续建造模式（HUDLayer 会自动处理扣费和创建）
        hudLayer->enterContinuousBuildMode(303);

        // 关闭商店
        this->removeFromParent();
        return;
      }
    }
  }
  // =================================================================

  // ========== 其他建筑的正常购买流程（保持不变） ==========
  // 扣除资源
  if (config->costType == "gold") {
    dataManager->spendGold(cost);
  } else if (config->costType == "elixir") {
    dataManager->spendElixir(cost);
  }

  // 创建建筑
  int buildingId = dataManager->addBuilding(data.id, 0, 0, 1, BuildingInstance::State::PLACING);
  if (buildingId < 0) {
    showTips("购买失败!", Color3B::RED);
    return;
  }

  CCLOG("ShopLayer: Building purchased successfully, ID=%d", buildingId);

  // 通知 VillageLayer
  auto scene = this->getScene();
  if (scene) {
    auto villageLayer = dynamic_cast<VillageLayer*>(scene->getChildByTag(1));
    if (villageLayer) {
      villageLayer->onBuildingPurchased(buildingId);
    }
  }

  // 关闭商店
  this->removeFromParent();
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

void ShopLayer::showErrorDialog(const std::string& message) {
  auto shieldLayer = cocos2d::LayerColor::create(cocos2d::Color4B(0, 0, 0, 150));
  this->addChild(shieldLayer, 100);

  auto dialogBg = cocos2d::LayerColor::create(cocos2d::Color4B(180, 0, 0, 255), 400, 150);
  dialogBg->ignoreAnchorPointForPosition(false);
  dialogBg->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));

  auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
  dialogBg->setPosition(visibleSize.width / 2, visibleSize.height / 2);
  shieldLayer->addChild(dialogBg);

  auto label = cocos2d::Label::createWithTTF(message, "fonts/simhei.ttf", 24);
  if (!label) {
    label = cocos2d::Label::createWithSystemFont(message, "Arial", 24);
  }
  label->setPosition(200, 100);
  label->setColor(cocos2d::Color3B::WHITE);
  dialogBg->addChild(label);

  auto okBtn = cocos2d::ui::Button::create();
  okBtn->setTitleText("确定");
  okBtn->setTitleFontSize(20);
  okBtn->setTitleColor(cocos2d::Color3B::WHITE);
  okBtn->setPosition(cocos2d::Vec2(200, 40));
  okBtn->addClickEventListener([shieldLayer](cocos2d::Ref*) {
    shieldLayer->removeFromParent();
  });
  dialogBg->addChild(okBtn);

  auto listener = cocos2d::EventListenerTouchOneByOne::create();
  listener->setSwallowTouches(true);
  listener->onTouchBegan = [shieldLayer](cocos2d::Touch*, cocos2d::Event*) {
    shieldLayer->removeFromParent();
    return true;
  };
  this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, shieldLayer);
}

// 在 ShopLayer 类中添加 showTips 方法实现
void ShopLayer::showTips(const std::string& message, const cocos2d::Color3B& color) {
    // 简单弹窗提示实现，可根据需要美化
    auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
    auto tipsBg = cocos2d::LayerColor::create(cocos2d::Color4B(0, 0, 0, 180), 420, 60);
    tipsBg->setIgnoreAnchorPointForPosition(false);
    tipsBg->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));
    tipsBg->setPosition(visibleSize.width / 2, visibleSize.height / 2 + 100);
    this->addChild(tipsBg, 999);

    auto label = cocos2d::Label::createWithTTF(message, "fonts/simhei.ttf", 28);
    label->setPosition(210, 30);
    label->setColor(color);
    label->enableOutline(cocos2d::Color4B::BLACK, 2);
    tipsBg->addChild(label);

    tipsBg->runAction(
        cocos2d::Sequence::create(
            cocos2d::DelayTime::create(1.2f),
            cocos2d::FadeOut::create(0.3f),
            cocos2d::RemoveSelf::create(),
            nullptr
        )
    );
}