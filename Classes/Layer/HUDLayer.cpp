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
#include "DebugLayer.h"
#include "Layer/LaboratoryLayer.h"
#include "Layer/ThemeSwitchLayer.h" 

USING_NS_CC;
using namespace ui;

const std::string FONT_PATH = "fonts/simhei.ttf";
// ========== 定义自定义颜色 ==========
const Color3B COLOR_CYAN = Color3B(0, 255, 255);      // 青色
const Color3B COLOR_ORANGE = Color3B(255, 165, 0);    // 橙色

//  定义按钮布局配置
const HUDLayer::ButtonLayout HUDLayer::LAYOUT_ONE_BUTTON = {
  Vec2(0, 0),     // 信息按钮居中
  Vec2(0, 0),     // 升级按钮不使用
  Vec2(0, 0)      // 训练按钮不使用
};

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

  // ========== 初始化连续建造模式状态 ==========
  _isContinuousBuildMode = false;
  _continuousBuildingType = -1;
  _modeHintLabel = nullptr;
  _keyboardListener = nullptr;

  auto dataManager = VillageDataManager::getInstance();
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 1. 金币：图标 + 文字
    auto goldIcon = Sprite::create("ImageElements/coin_icon.png");
  if (goldIcon) {
    goldIcon->setScale(0.5f);  // 缩放到合适大小
    goldIcon->setAnchorPoint(Vec2(1, 0.5f));  // 右对齐
    goldIcon->setPosition(Vec2(origin.x + 70, origin.y + visibleSize.height - 30));
    this->addChild(goldIcon);
  }
   
  _goldLabel = Label::createWithTTF("0", "fonts/Marker Felt.ttf", 24);
  _goldLabel->setAnchorPoint(Vec2(0, 0.5f));  // 左对齐
  _goldLabel->setPosition(Vec2(origin.x + 75, origin.y + visibleSize.height - 30));
  _goldLabel->setColor(Color3B(255, 215, 0));  // 金色
  _goldLabel->enableOutline(Color4B::BLACK, 2);  // 黑色描边
  _goldLabel->enableShadow(Color4B(0, 0, 0, 150), Size(2, -2));  // 阴影
  this->addChild(_goldLabel);

  // 2. 圣水：图标 + 文字
  auto elixirIcon = Sprite::create("ImageElements/elixir_icon.png");
  if (elixirIcon) {
    elixirIcon->setScale(0.5f);
    elixirIcon->setAnchorPoint(Vec2(1, 0.5f));
    elixirIcon->setPosition(Vec2(origin.x + 270, origin.y + visibleSize.height - 30));
    this->addChild(elixirIcon);
  }

  _elixirLabel = Label::createWithTTF("0", "fonts/Marker Felt.ttf", 24);
  _elixirLabel->setAnchorPoint(Vec2(0, 0.5f));
  _elixirLabel->setPosition(Vec2(origin.x + 275, origin.y + visibleSize.height - 30));
  _elixirLabel->setColor(Color3B(255, 0, 255));  // 紫红色
  _elixirLabel->enableOutline(Color4B::BLACK, 2);
  _elixirLabel->enableShadow(Color4B(0, 0, 0, 150), Size(2, -2));
  this->addChild(_elixirLabel);

  // 3. 宝石：图标 + 文字
  auto gemIcon = Sprite::create("ImageElements/gem_icon.png");
  if (gemIcon) {
    gemIcon->setScale(0.5f);
    gemIcon->setAnchorPoint(Vec2(1, 0.5f));
    gemIcon->setPosition(Vec2(origin.x + 460, origin.y + visibleSize.height - 30));
    this->addChild(gemIcon);
  }

  _gemLabel = Label::createWithTTF("0", "fonts/Marker Felt.ttf", 24);
  _gemLabel->setAnchorPoint(Vec2(0, 0.5f));
  _gemLabel->setPosition(Vec2(origin.x + 465, origin.y + visibleSize.height - 30));
  _gemLabel->setColor(Color3B(0, 255, 0));  // 绿色
  _gemLabel->enableOutline(Color4B::BLACK, 2);
  _gemLabel->enableShadow(Color4B(0, 0, 0, 150), Size(2, -2));
  this->addChild(_gemLabel);

  // 4. 工人：图标 + 文字
  auto workerIcon = Sprite::create("ImageElements/worker_icon.png");
  if (workerIcon) {
    workerIcon->setScale(0.5f);
    workerIcon->setAnchorPoint(Vec2(1, 0.5f));
    workerIcon->setPosition(Vec2(origin.x + 620, origin.y + visibleSize.height - 30));
    this->addChild(workerIcon);
  }

  _workerLabel = Label::createWithTTF("1/1", "fonts/Marker Felt.ttf", 24);
  _workerLabel->setAnchorPoint(Vec2(0, 0.5f));
  _workerLabel->setPosition(Vec2(origin.x + 625, origin.y + visibleSize.height - 30));
  _workerLabel->setColor(COLOR_CYAN);
  _workerLabel->enableOutline(Color4B::BLACK, 2);
  _workerLabel->enableShadow(Color4B(0, 0, 0, 150), Size(2, -2));
  this->addChild(_workerLabel);


  // 立即更新一次
  updateWorkerDisplay();

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

      // ========== 检查是否在连续建造模式 ==========
      if (_isContinuousBuildMode) {
        CCLOG("HUDLayer: In continuous build mode, checking if can continue...");

        // 检查是否可以继续建造
        if (canContinueBuild()) {
          CCLOG("HUDLayer: Creating next wall...");
          createNextWall();
        } else {
          exitContinuousBuildMode("资源不足或数量达到上限");
        }
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

      // ========== 如果在连续建造模式，退出模式 ==========
      if (_isContinuousBuildMode) {
        exitContinuousBuildMode("用户取消");
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
  // ========== 监听工人状态变化事件 ==========
// 监听建筑建造完成事件（包括工人小屋瞬间完成）
  auto buildingUpgradedListener = EventListenerCustom::create("EVENT_BUILDING_UPGRADED",
                                                              [this](EventCustom* event) {
    CCLOG("HUDLayer: Building upgraded/completed, updating worker display");
    updateWorkerDisplay();
  });
  _eventDispatcher->addEventListenerWithSceneGraphPriority(buildingUpgradedListener, this);

  // 监听建筑建造完成事件（新建筑完成）
  auto buildingConstructedListener = EventListenerCustom::create("EVENT_BUILDING_CONSTRUCTED",
                                                                 [this](EventCustom* event) {
    CCLOG("HUDLayer: Building construction completed, updating worker display");
    updateWorkerDisplay();
  });
  _eventDispatcher->addEventListenerWithSceneGraphPriority(buildingConstructedListener, this);

  // 监听建筑建造开始事件（需要在 VillageDataManager 中触发）
  auto constructionStartedListener = EventListenerCustom::create("EVENT_CONSTRUCTION_STARTED",
                                                                 [this](EventCustom* event) {
    CCLOG("HUDLayer: Construction started, updating worker display");
    updateWorkerDisplay();
  });
  _eventDispatcher->addEventListenerWithSceneGraphPriority(constructionStartedListener, this);
  // ========== 设置键盘监听器（用于 ESC 退出）==========
  setupKeyboardListener();

  // ========== 监听资源溢出事件 ==========
  auto goldOverflowListener = EventListenerCustom::create("EVENT_GOLD_OVERFLOW",
                                                          [this](EventCustom* event) {
    CCLOG("HUDLayer: Gold storage overflow!");
    showTips("金币存储已满！\n请升级储金罐", Color3B::ORANGE);
  });
  _eventDispatcher->addEventListenerWithSceneGraphPriority(goldOverflowListener, this);

  auto elixirOverflowListener = EventListenerCustom::create("EVENT_ELIXIR_OVERFLOW",
                                                            [this](EventCustom* event) {
    CCLOG("HUDLayer: Elixir storage overflow!");
    showTips("圣水存储已满！\n请升级圣水瓶", Color3B::ORANGE);
  });
  _eventDispatcher->addEventListenerWithSceneGraphPriority(elixirOverflowListener, this);

  // ========== 调试按钮 - 美化版 ==========
  // 创建背景容器
  auto debugBtnBg = LayerColor::create(Color4B(255, 69, 0, 230), 70, 35);  // 橙红色背景，带透明度
  debugBtnBg->setPosition(Vec2(origin.x + visibleSize.width - 85, origin.y + visibleSize.height - 95));
  debugBtnBg->setAnchorPoint(Vec2(0, 0));
  this->addChild(debugBtnBg, 10);
  
  // 添加渐变效果
  auto gradient = LayerGradient::create(
    Color4B(255, 100, 0, 200),   // 顶部：亮橙色
    Color4B(200, 30, 0, 200)     // 底部：深橙红色
  );
  gradient->setContentSize(Size(70, 35));
  gradient->setPosition(Vec2::ZERO);
  debugBtnBg->addChild(gradient);
  
  // 创建文字标签
  auto debugLabel = Label::createWithTTF("DEV", "fonts/Marker Felt.ttf", 22);
  debugLabel->setPosition(Vec2(35, 17.5));  // 居中
  debugLabel->setColor(Color3B::WHITE);
  debugLabel->enableOutline(Color4B(0, 0, 0, 255), 2);  // 黑色描边
  debugLabel->enableShadow(Color4B(0, 0, 0, 180), Size(2, -2));  // 阴影
  debugBtnBg->addChild(debugLabel, 1);
  
  // 直接给背景层添加触摸监听器
  auto debugTouchListener = EventListenerTouchOneByOne::create();
  debugTouchListener->setSwallowTouches(true);
  
  debugTouchListener->onTouchBegan = [debugBtnBg, this](Touch* touch, Event* event) {
      // 检查触摸点是否在按钮范围内
      Vec2 locationInNode = debugBtnBg->convertToNodeSpace(touch->getLocation());
      Size size = debugBtnBg->getContentSize();
      Rect rect = Rect(0, 0, size.width, size.height);
      
      if (rect.containsPoint(locationInNode)) {
          // 点击缩放动画
          debugBtnBg->runAction(Sequence::create(
              ScaleTo::create(0.1f, 0.9f),
              ScaleTo::create(0.1f, 1.0f),
              CallFunc::create([this]() {
                  auto debugLayer = DebugLayer::create();
                  this->getScene()->addChild(debugLayer, 200);
              }),
              nullptr
          ));
          return true;
      }
      return false;
  };
  
  _eventDispatcher->addEventListenerWithSceneGraphPriority(debugTouchListener, debugBtnBg);
  
  // 添加呼吸灯效果（循环闪烁）
  auto breathe = Sequence::create(
      FadeTo::create(1.0f, 180),
      FadeTo::create(1.0f, 230),
      nullptr
  );
  debugBtnBg->runAction(RepeatForever::create(breathe));

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

  // ========== 清理键盘监听器 ==========
  if (_keyboardListener) {
    Director::getInstance()->getEventDispatcher()->removeEventListener(_keyboardListener);
    _keyboardListener = nullptr;
    CCLOG("HUDLayer::cleanup - Removed keyboard listener");
  }

  // 清理连续建造模式的 UI
  if (_modeHintLabel) {
    _modeHintLabel->removeFromParent();
    _modeHintLabel = nullptr;
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

  // 获取存储上限
  int goldCap = dataManager->getGoldStorageCapacity();
  int elixirCap = dataManager->getElixirStorageCapacity();

  if (_goldLabel) {
    _goldLabel->setString(StringUtils::format("%d/%d", gold, goldCap));
  }
  if (_elixirLabel) {
    _elixirLabel->setString(StringUtils::format("%d/%d", elixir, elixirCap));
  }
  if (_gemLabel) {
    _gemLabel->setString(StringUtils::format("%d", gem));  // 宝石无上限
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

    // ========== 检查0：工人是否空闲（最高优先级）==========
    if (!dataManager->hasIdleWorker()) {
      int idle = dataManager->getIdleWorkerCount();
      int total = dataManager->getTotalWorkers();

      std::string tip = "所有工人都在忙碌！\n";
      tip += "空闲工人: " + std::to_string(idle) + "/" + std::to_string(total);
      tip += "\n请购买建筑工人小屋（50宝石）";

      showTips(tip, Color3B::ORANGE);
      return;
    }

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
      // ==========  显示升级成功 + 工人状态 ==========
      int idle = dataManager->getIdleWorkerCount();
      int total = dataManager->getTotalWorkers();

      std::string tip = "升级开始！\n";
      tip += "剩余工人: " + std::to_string(idle) + "/" + std::to_string(total);

      showTips(tip, Color3B::GREEN);
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

  // 研究按钮（实验室专用）
  _btnResearch = Button::create("UI/laboratory/research.png");  // 实验室研究按钮
  _btnResearch->ignoreContentAdaptWithSize(false);
  _btnResearch->setContentSize(Size(btnSize, btnSize));
  _btnResearch->setVisible(false);  // 默认隐藏
  _btnResearch->addClickEventListener([this](Ref*) {
    CCLOG("点击了研究按钮");
    auto labLayer = LaboratoryLayer::create();
    this->getScene()->addChild(labLayer, 150);
  });
  _actionMenuNode->addChild(_btnResearch);

  // ========== 切换场景按钮（大本营专用）==========
  _btnThemeSwitch = Button::create("UI/Village/change_bg_btn.png");
  _btnThemeSwitch->ignoreContentAdaptWithSize(false);
  _btnThemeSwitch->setContentSize(Size(btnSize, btnSize));
  _btnThemeSwitch->setVisible(false);  // 默认隐藏
  _btnThemeSwitch->addClickEventListener([this](Ref*) {
      CCLOG("点击了切换场景按钮");
      onThemeSwitchClicked();
  });
  _actionMenuNode->addChild(_btnThemeSwitch);
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

  // 更新建筑名称（建筑工人小屋不显示等级）
  std::string title;
  if (buildingInstance->type == 201) {  // 建筑工人小屋
    title = config->name;  // 不显示等级
  } else {
    title = config->name + " (" + std::to_string(buildingInstance->level) + "级)";
  }
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
    // ========== 建筑工人小屋特殊处理（不能升级）==========
    else if (buildingInstance->type == 201) {
      _btnUpgrade->setVisible(false);
      _upgradeCostLabel->setVisible(false);
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

  // ========== 研究按钮显示逻辑（实验室 103）==========
  bool canResearch = (buildingInstance->type == 103);
  _btnResearch->setVisible(canResearch);

  // ========== 选择布局模板 ==========
  const ButtonLayout* layout;
  if (buildingInstance->type == 201) {
      // 建筑工人小屋：只有信息按钮，居中显示
      layout = &LAYOUT_ONE_BUTTON;
  }
  // ========== 新增：大本营显示3个按钮 ==========
  else if (buildingInstance->type == 1) {
      // 大本营：信息 + 升级 + 切换场景
      layout = &LAYOUT_THREE_BUTTONS;
      _btnThemeSwitch->setVisible(true);
      _btnTrain->setVisible(false);
      _btnResearch->setVisible(false);
  }
  // ==============================================
  else if (canTrain || canResearch) {
      // 兵营/训练营/实验室：信息 + 升级 + 训练/研究
      layout = &LAYOUT_THREE_BUTTONS;
      _btnThemeSwitch->setVisible(false);
  } else {
      // 其他建筑：信息 + 升级
      layout = &LAYOUT_TWO_BUTTONS;
      _btnThemeSwitch->setVisible(false);
  }

  _btnInfo->setPosition(layout->infoPos);
  _btnUpgrade->setPosition(layout->upgradePos);
  _btnSpeedup->setPosition(layout->upgradePos);
  _btnTrain->setPosition(layout->trainPos);
  _btnResearch->setPosition(layout->trainPos);  // 研究按钮位置同训练按钮
  _btnThemeSwitch->setPosition(layout->trainPos);
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

// ========== 进入连续建造模式 ==========
void HUDLayer::enterContinuousBuildMode(int buildingType) {
  CCLOG("HUDLayer: Entering continuous build mode for type %d", buildingType);

  _isContinuousBuildMode = true;
  _continuousBuildingType = buildingType;

  // ========== 核心修复：将提示标签移到确认按钮上方 ==========
  auto visibleSize = Director::getInstance()->getVisibleSize();

  _modeHintLabel = Label::createWithTTF(
    "连续建造模式 | 按ESC退出",
    FONT_PATH,
    20  // 字体稍微小一点，更紧凑
  );

  // 位置：Y=180，正好在确认按钮（Y=100）上方 80 像素
  _modeHintLabel->setPosition(visibleSize.width / 2, 180.0f);
  _modeHintLabel->setColor(Color3B::YELLOW);
  _modeHintLabel->enableOutline(Color4B::BLACK, 2);
  this->addChild(_modeHintLabel, 999);  // ZOrder 999，低于 PlacementConfirmUI
  // ==================================================================

  // 更新 UI 显示剩余资源
  updateContinuousModeUI();

  // 创建第一个城墙
  createNextWall();
}

// ========== 退出连续建造模式 ==========
void HUDLayer::exitContinuousBuildMode(const std::string& reason) {
  CCLOG("HUDLayer: Exiting continuous build mode - Reason: %s", reason.c_str());

  _isContinuousBuildMode = false;
  _continuousBuildingType = -1;

  // 移除提示标签
  if (_modeHintLabel) {
    _modeHintLabel->removeFromParent();
    _modeHintLabel = nullptr;
  }

  // 显示退出原因（可选）
  auto visibleSize = Director::getInstance()->getVisibleSize();
  auto exitLabel = Label::createWithTTF(
    "已退出连续建造模式: " + reason,
    FONT_PATH,
    20
  );
  exitLabel->setPosition(visibleSize.width / 2, visibleSize.height / 2);
  exitLabel->setColor(Color3B::GREEN);
  exitLabel->enableOutline(Color4B::BLACK, 2);
  this->addChild(exitLabel, 1000);

  // 2秒后自动消失
  exitLabel->runAction(Sequence::create(
    DelayTime::create(2.0f),
    RemoveSelf::create(),
    nullptr
  ));
}

// ========== 创建下一个城墙 ==========
void HUDLayer::createNextWall() {
  auto dataManager = VillageDataManager::getInstance();
  auto config = BuildingConfig::getInstance()->getConfig(_continuousBuildingType);

  if (!config) {
    exitContinuousBuildMode("配置错误");
    return;
  }

  // 1. 扣除资源
  int cost = config->initialCost;
  if (config->costType == "gold") {
    if (dataManager->getGold() < cost) {
      exitContinuousBuildMode("金币不足");
      return;
    }
    dataManager->spendGold(cost);
  } else if (config->costType == "elixir") {
    if (dataManager->getElixir() < cost) {
      exitContinuousBuildMode("圣水不足");
      return;
    }
    dataManager->spendElixir(cost);
  }

  // 2. 创建建筑数据（PLACING 状态）
  int buildingId = dataManager->addBuilding(
    _continuousBuildingType, 0, 0, 1, BuildingInstance::State::PLACING
  );
  if (buildingId < 0) {
    exitContinuousBuildMode("创建失败");
    return;
  }

  // 3. 通知 VillageLayer 创建精灵并启动放置流程
  auto scene = this->getScene();
  if (scene) {
    auto villageLayer = dynamic_cast<VillageLayer*>(scene->getChildByTag(1));
    if (villageLayer) {
      villageLayer->onBuildingPurchased(buildingId);
    }
  }

  // 直接更新 UI 即可
  updateContinuousModeUI();

  CCLOG("HUDLayer: Created next wall (ID=%d)", buildingId);
}

// ========== 检查是否可以继续建造 ==========
bool HUDLayer::canContinueBuild() {
  auto dataManager = VillageDataManager::getInstance();
  auto requirements = BuildingRequirements::getInstance();
  auto config = BuildingConfig::getInstance()->getConfig(_continuousBuildingType);

  if (!config) return false;

  // 检查1：资源是否足够
  int cost = config->initialCost;
  if (config->costType == "gold" && dataManager->getGold() < cost) {
    CCLOG("HUDLayer: Not enough gold (%d < %d)", dataManager->getGold(), cost);
    return false;
  }
  if (config->costType == "elixir" && dataManager->getElixir() < cost) {
    CCLOG("HUDLayer: Not enough elixir (%d < %d)", dataManager->getElixir(), cost);
    return false;
  }

  // 检查2：数量是否达到上限
  int currentTHLevel = dataManager->getTownHallLevel();
  int maxCount = requirements->getMaxCount(_continuousBuildingType, currentTHLevel);

  int currentCount = 0;
  for (const auto& building : dataManager->getAllBuildings()) {
    if (building.type == _continuousBuildingType &&
        building.state != BuildingInstance::State::PLACING) {
      currentCount++;
    }
  }

  if (currentCount >= maxCount) {
    CCLOG("HUDLayer: Reached max count (%d >= %d)", currentCount, maxCount);
    return false;
  }

  return true;
}

// ========== 更新连续模式 UI ==========
void HUDLayer::updateContinuousModeUI() {
  if (!_modeHintLabel) return;

  auto dataManager = VillageDataManager::getInstance();
  auto config = BuildingConfig::getInstance()->getConfig(_continuousBuildingType);

  if (!config) return;

  // 简化后的提示文字
  std::string text = "连续建造模式: " + config->name + " | 按ESC退出";

  _modeHintLabel->setString(text);
}

// ========== 设置键盘监听器 ==========
void HUDLayer::setupKeyboardListener() {
  _keyboardListener = EventListenerKeyboard::create();

  _keyboardListener->onKeyPressed = CC_CALLBACK_2(HUDLayer::onKeyPressed, this);

  _eventDispatcher->addEventListenerWithSceneGraphPriority(_keyboardListener, this);

  CCLOG("HUDLayer: Keyboard listener set up");
}

// ========== 键盘事件处理 ==========
void HUDLayer::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event) {
  if (keyCode == EventKeyboard::KeyCode::KEY_ESCAPE) {
    CCLOG("HUDLayer: ESC key pressed");

    if (_isContinuousBuildMode) {
      // 如果正在放置建筑，先取消当前建筑
      if (_placementController && _placementController->isPlacing()) {
        _placementController->cancelPlacement();
      }
      exitContinuousBuildMode("用户按ESC退出");
    }
  }
}

// ========== 工人状态显示更新 ==========
void HUDLayer::updateWorkerDisplay() {
  if (!_workerLabel) return;

  auto dataManager = VillageDataManager::getInstance();

  int idle = dataManager->getIdleWorkerCount();
  int total = dataManager->getTotalWorkers();

  // 显示格式："Workers: 空闲/总数"
  std::string text = StringUtils::format("%d/%d", idle, total);
  _workerLabel->setString(text);

  // 根据工人状态动态改变颜色
  if (idle == 0) {
    _workerLabel->setColor(Color3B::RED);     // 全部忙碌 - 红色警告
  } else if (idle < total) {
    _workerLabel->setColor(Color3B::ORANGE);  // 部分忙碌 - 橙色
  } else {
    _workerLabel->setColor(COLOR_CYAN);    // 全部空闲 - 青色
  }
}
// ========== 场景切换按钮回调 ==========
void HUDLayer::onThemeSwitchClicked() {
    CCLOG("HUDLayer: Opening theme switch panel");

    // 创建场景选择面板
    auto themeSwitchLayer = ThemeSwitchLayer::create();
    this->getScene()->addChild(themeSwitchLayer, 150);

    // 隐藏建筑菜单
    hideBuildingActions();
}
