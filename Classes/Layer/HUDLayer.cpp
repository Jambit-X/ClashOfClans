#pragma execution_character_set("utf-8")
#include "HUDLayer.h"
#include "Layer/ShopLayer.h"
#include "Manager/VillageDataManager.h"
#include "Manager/BuildingUpgradeManager.h" 
#include "Model/BuildingConfig.h"
#include "UI/ResourceCollectionUI.h"  // 新增

USING_NS_CC;
using namespace ui;

const std::string FONT_PATH = "fonts/simhei.ttf";

bool HUDLayer::init() {
  if (!Layer::init()) {
    return false;
  }

  auto dataManager = VillageDataManager::getInstance();
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 1. 当前资源标签
  _goldLabel = Label::createWithTTF("Gold: 0", "fonts/Marker Felt.ttf", 24);
  _goldLabel->setPosition(Vec2(origin.x + 100, origin.y + visibleSize.height - 30));
  this->addChild(_goldLabel);

  _elixirLabel = Label::createWithTTF("Elixir: 0", "fonts/Marker Felt.ttf", 24);
  _elixirLabel->setPosition(Vec2(origin.x + 300, origin.y + visibleSize.height - 30));
  this->addChild(_elixirLabel);

  // 2. 初始化资源显示
  updateResourceDisplay(dataManager->getGold(), dataManager->getElixir());

  // 3. 设置资源回调
  dataManager->setResourceCallback([this](int gold, int elixir) {
    updateResourceDisplay(gold, elixir);
  });

  // 4. 新增：使用独立的资源收集UI组件
  auto resourceUI = ResourceCollectionUI::create();
  this->addChild(resourceUI, 10);

  this->scheduleUpdate();

  // 监听资源变化事件
  auto resource_update_listener = EventListenerCustom::create("EVENT_RESOURCE_CHANGED",
                                                              [this](EventCustom* event) {
    auto dataManager = VillageDataManager::getInstance();
    int gold = dataManager->getGold();
    int elixir = dataManager->getElixir();
    this->updateResourceDisplay(gold, elixir);
    CCLOG("HUD 接收到资源变化通知，已更新：Gold=%d, Elixir=%d", gold, elixir);
  });
  _eventDispatcher->addEventListenerWithSceneGraphPriority(resource_update_listener, this);

  // 商店入口按钮
  auto shopBtn = ui::Button::create("UI/Shop/Shop-button.png");
  shopBtn->setAnchorPoint(Vec2(1, 0));
  shopBtn->setPosition(Vec2(origin.x + visibleSize.width - 20, origin.y + 20));
  shopBtn->addClickEventListener([=](Ref* sender) {
    CCLOG("Member C: 点击了商店图标，准备打开商店...");
    auto shopLayer = ShopLayer::create();
    this->getScene()->addChild(shopLayer, 100);
  });
  this->addChild(shopBtn);

  // 初始化底部建筑操作菜单 
  initActionMenu();

  // 进攻按钮
  auto battleBtn = ui::Button::create("UI/battle/battle-icon/battle-icon.png");
  if (battleBtn) {
    battleBtn->setAnchorPoint(Vec2(0, 0));
    battleBtn->setPosition(Vec2(origin.x + 20, origin.y + 20));
    battleBtn->setScale(0.8f);
    battleBtn->addClickEventListener([=](Ref* sender) {
      CCLOG("点击了进攻按钮！TODO: 进入战斗场景");
    });
    this->addChild(battleBtn);
  } else {
    CCLOG("错误：无法加载进攻按钮图片，请检查路径");
  }

  return true;
}

// ========== 恢复缺失的函数 ==========

void HUDLayer::update(float dt) {
  Layer::update(dt);
  BuildingUpgradeManager::getInstance()->update(dt);
}

void HUDLayer::updateResourceDisplay(int gold, int elixir) {
  if (_goldLabel) {
    _goldLabel->setString(StringUtils::format("Gold: %d", gold));
  }
  if (_elixirLabel) {
    _elixirLabel->setString(StringUtils::format("Elixir: %d", elixir));
  }
}

void HUDLayer::initActionMenu() {
  auto visibleSize = Director::getInstance()->getVisibleSize();

  _actionMenuNode = Node::create();
  _actionMenuNode->setPosition(Vec2(visibleSize.width / 2, 100));
  _actionMenuNode->setVisible(false);
  this->addChild(_actionMenuNode, 20);

  _buildingNameLabel = Label::createWithTTF("建筑名称", FONT_PATH, 24);
  _buildingNameLabel->enableOutline(Color4B::BLACK, 2);
  _buildingNameLabel->setPosition(Vec2(0, 110));
  _actionMenuNode->addChild(_buildingNameLabel);

  float btnSize = 140.0f;
  float spacing = 20.0f;
  std::string imgPath = "UI/training-camp/building-icon/";

  // 信息按钮
  _btnInfo = Button::create(imgPath + "info.png");
  _btnInfo->ignoreContentAdaptWithSize(false);
  _btnInfo->setContentSize(Size(btnSize, btnSize));
  _btnInfo->setPosition(Vec2(-(btnSize + spacing), 0));
  _btnInfo->addClickEventListener([=](Ref*) {
    CCLOG("点击了信息");
  });
  _actionMenuNode->addChild(_btnInfo);

  // 升级按钮
  _btnUpgrade = Button::create(imgPath + "upgrade.png");
  _btnUpgrade->ignoreContentAdaptWithSize(false);
  _btnUpgrade->setContentSize(Size(btnSize, btnSize));
  _btnUpgrade->setPosition(Vec2(0, 0));
  _btnUpgrade->addClickEventListener([=](Ref*) {
    CCLOG("点击升级");

    if (_currentSelectedBuildingId == -1) return;

    auto dataManager = VillageDataManager::getInstance();
    if (dataManager->startUpgradeBuilding(_currentSelectedBuildingId)) {
      CCLOG("升级开始成功!");
      hideBuildingActions();
    } else {
      CCLOG("升级失败:资源不足或已达最高等级");
    }
  });
  _actionMenuNode->addChild(_btnUpgrade);

  _upgradeCostLabel = Label::createWithTTF("270000 圣水", FONT_PATH, 20);
  _upgradeCostLabel->setColor(Color3B::MAGENTA);
  _upgradeCostLabel->enableOutline(Color4B::BLACK, 2);
  _upgradeCostLabel->setAnchorPoint(Vec2(0.5f, 1.0f));
  _upgradeCostLabel->setPosition(Vec2(btnSize / 2, btnSize - 10));
  _btnUpgrade->addChild(_upgradeCostLabel);

  // 训练按钮
  _btnTrain = Button::create(imgPath + "training.png");
  _btnTrain->ignoreContentAdaptWithSize(false);
  _btnTrain->setContentSize(Size(btnSize, btnSize));
  _btnTrain->setPosition(Vec2(btnSize + spacing, 0));
  _btnTrain->addClickEventListener([=](Ref*) {
    CCLOG("点击了训练部队");
  });
  _actionMenuNode->addChild(_btnTrain);
}

void HUDLayer::showBuildingActions(int buildingId) {
  if (_currentSelectedBuildingId == buildingId && _actionMenuNode->isVisible()) {
    return;
  }

  _currentSelectedBuildingId = buildingId;
  updateActionButtons(buildingId);

  _actionMenuNode->setVisible(true);
  _actionMenuNode->setScale(0.1f);
  _actionMenuNode->runAction(EaseBackOut::create(ScaleTo::create(0.2f, 1.0f)));
}

void HUDLayer::hideBuildingActions() {
  if (_actionMenuNode->isVisible()) {
    _actionMenuNode->setVisible(false);
    _currentSelectedBuildingId = -1;
  }
}

void HUDLayer::updateActionButtons(int buildingId) {
  auto dataManager = VillageDataManager::getInstance();
  auto buildingInstance = dataManager->getBuildingById(buildingId);
  if (!buildingInstance) return;

  auto configMgr = BuildingConfig::getInstance();
  auto config = configMgr->getConfig(buildingInstance->type);
  if (!config) return;

  std::string title = config->name + " (" + std::to_string(buildingInstance->level) + "级)";
  _buildingNameLabel->setString(title);

  if (configMgr->canUpgrade(buildingInstance->type, buildingInstance->level)) {
    int cost = configMgr->getUpgradeCost(buildingInstance->type, buildingInstance->level);
    _upgradeCostLabel->setString(std::to_string(cost) + " 圣水");
    _upgradeCostLabel->setVisible(true);
    _btnUpgrade->setTouchEnabled(true);
    _btnUpgrade->setOpacity(255);
  } else {
    _upgradeCostLabel->setString("已满级");
    _btnUpgrade->setTouchEnabled(false);
    _btnUpgrade->setOpacity(128);
  }

  bool canTrain = (buildingInstance->type == 101 || buildingInstance->type == 102);
  _btnTrain->setVisible(canTrain);

  float btnSize = 140.0f;
  float spacing = 20.0f;

  if (canTrain) {
    _btnInfo->setPosition(Vec2(-(btnSize + spacing), 0));
    _btnUpgrade->setPosition(Vec2(0, 0));
    _btnTrain->setPosition(Vec2(btnSize + spacing, 0));
  } else {
    _btnInfo->setPosition(Vec2(-(btnSize / 2 + spacing / 2), 0));
    _btnUpgrade->setPosition(Vec2(btnSize / 2 + spacing / 2, 0));
  }
}