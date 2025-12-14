#pragma execution_character_set("utf-8")
#include "HUDLayer.h"
#include "Layer/ShopLayer.h"
#include "Manager/VillageDataManager.h"
#include "Manager/BuildingUpgradeManager.h"
#include "Manager/BuildingSpeedupManager.h"
#include "Model/BuildingConfig.h"
#include "Model/BuildingRequirements.h"
#include "Layer/TrainingLayer.h"
#include "UI/ResourceCollectionUI.h"
#include "Layer/VillageLayer.h"
#include "Scene/BattleScene.h" 

USING_NS_CC;
using namespace ui;

const std::string FONT_PATH = "fonts/simhei.ttf";

//  定义按钮布局配置
const HUDLayer::ButtonLayout HUDLayer::LAYOUT_TWO_BUTTONS = {
  Vec2(-80, 0),   // 信息按钮在左
  Vec2(80, 0),    // 升级按钮在右
  Vec2(0, 0)      // 训练按钮不使用
};

const HUDLayer::ButtonLayout HUDLayer::LAYOUT_THREE_BUTTONS = {
  Vec2(-160, 0),  // 信息按钮在左
  Vec2(0, 0),     // 升级按钮在中
  Vec2(160, 0)    // 训练按钮在右
};

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

  _gemLabel = Label::createWithTTF("Gem: 0", "fonts/Marker Felt.ttf", 24);
  _gemLabel->setPosition(Vec2(origin.x + 500, origin.y + visibleSize.height - 30));
  _gemLabel->setColor(Color3B::GREEN);
  this->addChild(_gemLabel);

  //  新增：预先创建提示Label（复用）
  _tipsLabel = Label::createWithTTF("", FONT_PATH, 30);
  _tipsLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
  _tipsLabel->enableOutline(Color4B::BLACK, 2);
  _tipsLabel->setVisible(false);  // 默认隐藏
  this->addChild(_tipsLabel, 1000);

  // 2. 初始化资源显示
  updateResourceDisplay(dataManager->getGold(), dataManager->getElixir());

  // 3. 设置资源回调
  dataManager->setResourceCallback([this](int gold, int elixir) {
    updateResourceDisplay(gold, elixir);
  });

  // 4. 使用独立的资源收集UI组件
  auto resourceUI = ResourceCollectionUI::create();
  this->addChild(resourceUI, 10);

  this->scheduleUpdate();

  // 商店入口按钮
  auto shopBtn = ui::Button::create("UI/Shop/Shop-button.png");
  shopBtn->setAnchorPoint(Vec2(1, 0));
  shopBtn->setPosition(Vec2(origin.x + visibleSize.width - 20, origin.y + 20));
  shopBtn->addClickEventListener([this](Ref* sender) {
    CCLOG("点击了商店图标，准备打开商店...");
    auto shopLayer = ShopLayer::create();
    this->getScene()->addChild(shopLayer, 100);
  });
  this->addChild(shopBtn);

  // 初始化底部建筑操作菜单
  initActionMenu();

  // 初始化放置UI
  _placementUI = PlacementConfirmUI::create();
  this->addChild(_placementUI, 1000);

  // 设置放置UI的回调
  _placementUI->setConfirmCallback([this]() {
    if (_placementController && _placementController->confirmPlacement()) {
      CCLOG("HUDLayer: Building placement confirmed");
      if (_placementTouchListener) {
        Director::getInstance()->getEventDispatcher()->removeEventListener(_placementTouchListener);
        _placementTouchListener = nullptr;
      }
    }
  });

  _placementUI->setCancelCallback([this]() {
    if (_placementController) {
      _placementController->cancelPlacement();
      CCLOG("HUDLayer: Building placement cancelled");
      if (_placementTouchListener) {
        Director::getInstance()->getEventDispatcher()->removeEventListener(_placementTouchListener);
        _placementTouchListener = nullptr;
      }
    }
  });

  // 进攻按钮
  auto battleBtn = ui::Button::create("UI/battle/battle-icon/battle-icon.png");
  if (battleBtn) {
    battleBtn->setAnchorPoint(Vec2(0, 0));
    battleBtn->setPosition(Vec2(origin.x + 20, origin.y + 20));
    battleBtn->setScale(0.8f);
    battleBtn->addClickEventListener([this](Ref* sender) {
      CCLOG("点击了进攻按钮！");
      
      // 关键修复：使用延迟切换场景，避免在事件处理中直接切换导致监听器冲突
      this->getScene()->runAction(Sequence::create(
        DelayTime::create(0.1f),  // 延迟0.1秒，确保触摸事件完全结束
        CallFunc::create([]() {
          auto battleScene = BattleScene::createScene();
          Director::getInstance()->replaceScene(TransitionFade::create(0.5f, battleScene));
        }),
        nullptr
      ));
    });
    this->addChild(battleBtn);
  }

  // 初始化放置控制器
  _placementController = new BuildingPlacementController();
  _placementTouchListener = nullptr;

  _placementController->setPlacementCallback([this](bool success, int buildingId) {
    if (success) {
      CCLOG("HUDLayer: Building placed successfully, starting construction");
      auto scene = this->getScene();
      if (scene) {
        auto villageLayer = dynamic_cast<VillageLayer*>(scene->getChildByTag(1));
        if (villageLayer) {
          villageLayer->updateBuildingDisplay(buildingId);
        }
      }
    } else {
      CCLOG("HUDLayer: Building placement cancelled, removing sprite");
      auto scene = this->getScene();
      if (scene) {
        auto villageLayer = dynamic_cast<VillageLayer*>(scene->getChildByTag(1));
        if (villageLayer) {
          villageLayer->removeBuildingSprite(buildingId);
        }
      }
    }
    hidePlacementUI();
  });

  return true;
}

void HUDLayer::cleanup() {
  CCLOG("HUDLayer::cleanup - Cleaning up resources");
  
  // 清理触摸监听器
  if (_placementTouchListener) {
    Director::getInstance()->getEventDispatcher()->removeEventListener(_placementTouchListener);
    _placementTouchListener = nullptr;
    CCLOG("HUDLayer::cleanup - Removed placement touch listener");
  }
  
  // 清理放置控制器
  if (_placementController) {
    delete _placementController;
    _placementController = nullptr;
    CCLOG("HUDLayer::cleanup - Deleted placement controller");
  }
  
  Layer::cleanup();
}

void HUDLayer::update(float dt) {
  Layer::update(dt);
  BuildingUpgradeManager::getInstance()->update(dt);
}

void HUDLayer::updateResourceDisplay(int gold, int elixir) {
  auto dataManager = VillageDataManager::getInstance();
  int gem = dataManager->getGem();

  if (_goldLabel) {
    _goldLabel->setString(StringUtils::format("Gold: %d", gold));
  }
  if (_elixirLabel) {
    _elixirLabel->setString(StringUtils::format("Elixir: %d", elixir));
  }
  if (_gemLabel) {
    _gemLabel->setString(StringUtils::format("Gem: %d", gem));
  }
}

//  新增：提示消息复用方法
void HUDLayer::showTips(const std::string& text, const Color3B& color) {
  if (!_tipsLabel) return;

  // 停止之前的动画，避免冲突
  _tipsLabel->stopAllActions();

  // 设置文字和颜色
  _tipsLabel->setString(text);
  _tipsLabel->setColor(color);
  _tipsLabel->setOpacity(255);
  _tipsLabel->setVisible(true);

  // 淡出动画
  auto fadeOut = Sequence::create(
    DelayTime::create(1.5f),
    FadeOut::create(0.5f),
    Hide::create(),  // 只隐藏，不删除
    nullptr
  );

  _tipsLabel->runAction(fadeOut);
}

void HUDLayer::hideTips() {
  if (_tipsLabel) {
    _tipsLabel->stopAllActions();
    _tipsLabel->setVisible(false);
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
  std::string imgPath = "UI/training-camp/building-icon/";

  // 信息按钮
  _btnInfo = Button::create(imgPath + "info.png");
  _btnInfo->ignoreContentAdaptWithSize(false);
  _btnInfo->setContentSize(Size(btnSize, btnSize));
  _btnInfo->addClickEventListener([this](Ref*) {
    CCLOG("点击了信息");
  });
  _actionMenuNode->addChild(_btnInfo);

  // 升级按钮 - 修复版
  _btnUpgrade = Button::create(imgPath + "upgrade.png");
  _btnUpgrade->ignoreContentAdaptWithSize(false);
  _btnUpgrade->setContentSize(Size(btnSize, btnSize));
  _btnUpgrade->addClickEventListener([this](Ref*) {
    CCLOG("点击升级");

    if (_currentSelectedBuildingId == -1) return;

    auto dataManager = VillageDataManager::getInstance();
    auto building = dataManager->getBuildingById(_currentSelectedBuildingId);

    if (!building) return;

    // ========== 检查1：是否满级 ==========
    if (building->level >= 3) {
      showTips("建筑已达到最大等级！", Color3B::RED);
      return;
    }

    auto requirements = BuildingRequirements::getInstance();

    // ========== 检查2：大本营特殊处理 ==========
    if (building->type == 1) {  // 大本营
      if (!requirements->canUpgradeTownHall(building->level)) {
        showTips("大本营升级条件未满足\n请先建造所有防御建筑！", Color3B::RED);
        return;
      }
    }
    // ========== 检查3：其他建筑的大本营等级限制 ==========
    else {
      int currentTHLevel = dataManager->getTownHallLevel();

      if (!requirements->canUpgrade(building->type, building->level, currentTHLevel)) {
        int targetLevel = building->level + 1;
        int requiredTH = requirements->getRequiredTHLevel(building->type, targetLevel);

        std::string reason = "需要" + std::to_string(requiredTH) + "级大本营";
        showTips(reason, Color3B::ORANGE);  // 橙色警告
        return;
      }
    }

    // ========== 检查4：资源充足并开始升级 ==========
    if (dataManager->startUpgradeBuilding(_currentSelectedBuildingId)) {
      CCLOG("升级开始成功!");
      hideBuildingActions();
      showTips("升级开始！", Color3B::GREEN);
    } else {
      showTips("升级失败：资源不足", Color3B::RED);
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
  _btnTrain->addClickEventListener([this](Ref*) {
    CCLOG("点击了训练部队");
    auto trainLayer = TrainingLayer::create();
    this->getScene()->addChild(trainLayer, 150);
  });
  _actionMenuNode->addChild(_btnTrain);

  //  加速按钮
  _btnSpeedup = Button::create("UI/Village/Speedup_button.png");
  _btnSpeedup->ignoreContentAdaptWithSize(false);
  _btnSpeedup->setContentSize(Size(btnSize, btnSize));
  _btnSpeedup->setVisible(false);  // 默认隐藏
  _btnSpeedup->addClickEventListener([this](Ref*) {
    CCLOG("点击了加速按钮");
    if (_currentSelectedBuildingId == -1) return;
    onSpeedupClicked(_currentSelectedBuildingId);
  });
  _actionMenuNode->addChild(_btnSpeedup);
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

  // 更新建筑名称
  std::string title = config->name + " (" + std::to_string(buildingInstance->level) + "级)";
  _buildingNameLabel->setString(title);

  // ========== 根据状态切换按钮 ==========
  bool isConstructing = (buildingInstance->state == BuildingInstance::State::CONSTRUCTING);

  _btnUpgrade->setVisible(!isConstructing);
  _btnSpeedup->setVisible(isConstructing);

  if (isConstructing) {
    // 建造中：显示加速按钮
    bool hasGem = (dataManager->getGem() >= 1);
    _btnSpeedup->setTouchEnabled(hasGem);
    _btnSpeedup->setOpacity(hasGem ? 255 : 128);
  } else {
    // 已完成：根据建筑类型分别处理
    auto requirements = BuildingRequirements::getInstance();
    int currentTHLevel = dataManager->getTownHallLevel();

    // ========== 大本营特殊处理 ==========
    if (buildingInstance->type == 1) {
      if (buildingInstance->level >= 3) {
        _upgradeCostLabel->setString("已满级");
        _upgradeCostLabel->setColor(Color3B::RED);
        _btnUpgrade->setTouchEnabled(false);
        _btnUpgrade->setOpacity(128);
      } else if (!requirements->canUpgradeTownHall(buildingInstance->level)) {
        _upgradeCostLabel->setString("需建造所有防御");
        _upgradeCostLabel->setColor(Color3B::ORANGE);
        _btnUpgrade->setTouchEnabled(false);
        _btnUpgrade->setOpacity(128);
      } else {
        // 可以升级 - 翻译 costType
        int cost = configMgr->getUpgradeCost(buildingInstance->type, buildingInstance->level);

        // 翻译为中文
        std::string costTypeCN;
        if (config->costType == "gold") {
          costTypeCN = "金币";
        } else if (config->costType == "elixir") {
          costTypeCN = "圣水";
        } else if (config->costType == "gem") {
          costTypeCN = "宝石";
        }
        _upgradeCostLabel->setString(std::to_string(cost) + " " + costTypeCN);

        // 使用英文判断
        if (config->costType == "gold") {
          _upgradeCostLabel->setColor(Color3B::YELLOW);
        } else if (config->costType == "elixir") {
          _upgradeCostLabel->setColor(Color3B::MAGENTA);
        } else {
          _upgradeCostLabel->setColor(Color3B::GREEN);
        }

        _upgradeCostLabel->setVisible(true);
        _btnUpgrade->setTouchEnabled(true);
        _btnUpgrade->setOpacity(255);
      }
    }
    // ========== 其他建筑的通用处理 ==========
    else {
      if (buildingInstance->level >= 3) {
        _upgradeCostLabel->setString("已满级");
        _upgradeCostLabel->setColor(Color3B::RED);
        _btnUpgrade->setTouchEnabled(false);
        _btnUpgrade->setOpacity(128);
      } else if (!requirements->canUpgrade(buildingInstance->type, buildingInstance->level, currentTHLevel)) {
        int targetLevel = buildingInstance->level + 1;
        int requiredTH = requirements->getRequiredTHLevel(buildingInstance->type, targetLevel);

        _upgradeCostLabel->setString("需要" + std::to_string(requiredTH) + "级大本营");
        _upgradeCostLabel->setColor(Color3B::ORANGE);
        _btnUpgrade->setTouchEnabled(false);
        _btnUpgrade->setOpacity(128);
      } else if (configMgr->canUpgrade(buildingInstance->type, buildingInstance->level)) {
        // 可以升级 - 翻译 costType
        int cost = configMgr->getUpgradeCost(buildingInstance->type, buildingInstance->level);

        // 翻译为中文
        std::string costTypeCN;
        if (config->costType == "gold") {
          costTypeCN = "金币";
        } else if (config->costType == "elixir") {
          costTypeCN = "圣水";
        } else if (config->costType == "gem") {
          costTypeCN = "宝石";
        }
        _upgradeCostLabel->setString(std::to_string(cost) + " " + costTypeCN);

        // 使用英文判断
        if (config->costType == "gold") {
          _upgradeCostLabel->setColor(Color3B::YELLOW);
        } else if (config->costType == "elixir") {
          _upgradeCostLabel->setColor(Color3B::MAGENTA);
        } else {
          _upgradeCostLabel->setColor(Color3B::GREEN);
        }

        _upgradeCostLabel->setVisible(true);
        _btnUpgrade->setTouchEnabled(true);
        _btnUpgrade->setOpacity(255);
      } else {
        _upgradeCostLabel->setString("已满级");
        _upgradeCostLabel->setColor(Color3B::RED);
        _btnUpgrade->setTouchEnabled(false);
        _btnUpgrade->setOpacity(128);
      }
    }
  }

  // ========== 训练按钮显示逻辑 ==========
  bool canTrain = (buildingInstance->type == 101 || buildingInstance->type == 102);
  _btnTrain->setVisible(canTrain);

  const ButtonLayout& layout = canTrain ? LAYOUT_THREE_BUTTONS : LAYOUT_TWO_BUTTONS;

  _btnInfo->setPosition(layout.infoPos);
  _btnUpgrade->setPosition(layout.upgradePos);
  _btnSpeedup->setPosition(layout.upgradePos);
  _btnTrain->setPosition(layout.trainPos);
}

//  新增：加速按钮回调
void HUDLayer::onSpeedupClicked(int buildingId) {
  auto speedupManager = BuildingSpeedupManager::getInstance();

  if (speedupManager->speedupBuilding(buildingId)) {
    CCLOG("HUDLayer: Building %d speedup successful", buildingId);
    hideBuildingActions();

    //  使用复用的提示Label
    showTips("建造完成！（消耗1宝石）", Color3B::GREEN);
  } else {
    std::string reason = speedupManager->getSpeedupFailReason(buildingId);
    CCLOG("HUDLayer: Speedup failed: %s", reason.c_str());

    //  使用复用的提示Label
    showTips(reason, Color3B::RED);
  }
}

// =========建筑放置==========
void HUDLayer::showPlacementUI(int buildingId) {
  _placementUI->show();
  updatePlacementUIState(false);
}

void HUDLayer::hidePlacementUI() {
  _placementUI->hide();
}

void HUDLayer::updatePlacementUIState(bool canPlace) {
  _placementUI->updateButtonState(canPlace);
}

void HUDLayer::startBuildingPlacement(int buildingId) {
  CCLOG("HUDLayer: Starting placement for building ID=%d", buildingId);

  _placementController->startPlacement(buildingId);
  showPlacementUI(buildingId);

  if (_placementTouchListener) {
    Director::getInstance()->getEventDispatcher()->removeEventListener(_placementTouchListener);
  }

  _placementTouchListener = EventListenerTouchOneByOne::create();
  _placementTouchListener->setSwallowTouches(true);

  _placementTouchListener->onTouchBegan = [this, buildingId](Touch* touch, Event* event) {
    return true;
  };

  _placementTouchListener->onTouchMoved = [this, buildingId](Touch* touch, Event* event) {
    Vec2 touchPos = touch->getLocation();
    auto scene = this->getScene();
    if (!scene) return;

    auto villageLayer = dynamic_cast<VillageLayer*>(scene->getChildByTag(1));
    if (!villageLayer) return;

    Vec2 worldPos = villageLayer->convertToNodeSpace(touchPos);
    villageLayer->updateBuildingPreviewPosition(buildingId, worldPos);
  };

  _placementTouchListener->onTouchEnded = [this](Touch* touch, Event* event) {
    CCLOG("HUDLayer: Placement touch ended");
  };

  Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(
    _placementTouchListener, this);
}