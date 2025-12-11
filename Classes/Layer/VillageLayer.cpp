#include "VillageLayer.h"
#include "../Controller/MoveMapController.h"
#include "../Controller/MoveBuildingController.h"
#include "../Util/GridMapUtils.h"
#include "../proj.win32/Constants.h"
#include "Manager/BuildingManager.h"
#include "Manager/VillageDataManager.h"
#include "Sprite/BattleUnitSprite.h"
#include "ui/CocosGUI.h"
#include <iostream>
#include "Layer/HUDLayer.h"

USING_NS_CC;

bool VillageLayer::init() {
  if (!Layer::init()) {
    return false;
  }

  // 1. 创建地图精灵
  _mapSprite = createMapSprite();
  if (!_mapSprite) {
    return false;
  }
  this->addChild(_mapSprite);

  // 2. 初始化基本属性
  initializeBasicProperties();

  // 3. 初始化建筑管理器
  _buildingManager = new BuildingManager(this);

  // ✅ 4. 先初始化建筑移动控制器（优先级更高）
  _moveBuildingController = new MoveBuildingController(this, _buildingManager);
  _moveBuildingController->setupTouchListener();
  
  // ✅ 设置短按建筑回调
  _moveBuildingController->setOnBuildingTappedCallback([this](int buildingId) {
      CCLOG("VillageLayer: Building tapped ID=%d, showing menu", buildingId);
    
      // 获取 HUD 层并显示菜单
   auto scene = this->getScene();
      if (scene) {
          auto hudLayer = dynamic_cast<HUDLayer*>(scene->getChildByTag(100));
   if (hudLayer) {
        hudLayer->showBuildingActions(buildingId);
        }
      }
  });

  // ✅ 5. 后初始化地图移动控制器（优先级更低）
  _inputController = new MoveMapController(this);
  _inputController->setupInputListeners();

  // 6. 设置点击检测回调
  setupInputCallbacks();

  // 7. 启动建筑更新
  this->schedule([this](float dt) {
    _buildingManager->update(dt);
  }, 1.0f, "building_update");

  CCLOG("VillageLayer initialized successfully");
  CCLOG("  - MoveBuildingController added first (higher priority)");
  CCLOG("  - MoveMapController added second (lower priority)");

  // 在 VillageLayer::init() 中添加事件监听:
  auto upgradeListener = EventListenerCustom::create("EVENT_BUILDING_UPGRADED", 
    [this](EventCustom* event) {
        int buildingId = *(int*)event->getUserData();
        
        // 更新建筑精灵外观
        auto dataManager = VillageDataManager::getInstance();
        auto building = dataManager->getBuildingById(buildingId);
        if (building) {
            // 找到对应的 BuildingSprite 并更新
            for (auto child : this->getChildren()) {
                auto buildingSprite = dynamic_cast<BuildingSprite*>(child);
                if (buildingSprite && buildingSprite->getBuildingId() == buildingId) {
                    buildingSprite->updateLevel(building->level);
                    buildingSprite->updateState(building->state);
                    break;
                }
            }
        }
    });
  _eventDispatcher->addEventListenerWithSceneGraphPriority(upgradeListener, this);

  return true;
}

void VillageLayer::cleanup() {
  // 清理建筑移动控制器
  if (_moveBuildingController) {
    delete _moveBuildingController;
    _moveBuildingController = nullptr;
  }

  // 清理建筑管理器
  if (_buildingManager) {
    delete _buildingManager;
    _buildingManager = nullptr;
  }

  if (_inputController) {
    _inputController->cleanup();
    delete _inputController;
    _inputController = nullptr;
  }
  
  Layer::cleanup();
}

// 购买建筑回调函数
void VillageLayer::onBuildingPurchased(int buildingId) {
  CCLOG("VillageLayer: 建筑已购买，ID=%d，准备放置", buildingId);

  auto dataManager = VillageDataManager::getInstance();
  auto building = dataManager->getBuildingById(buildingId);

  if (building) {
    // ✅ 临时：放置到更明显的位置 (地图中心偏下)
    dataManager->setBuildingPosition(buildingId, 10, 10);
    
    // ✅ 设置为正常状态 (BUILT)，而不是建造中
    dataManager->setBuildingState(buildingId,
 BuildingInstance::State::BUILT,
             0);  // 无完成时间

    // 让 BuildingManager 创建精灵
    _buildingManager->addBuilding(*building);
    
    CCLOG("建筑已放置: ID=%d, 位置=(10,10), 状态=BUILT", buildingId);
  } else {
    CCLOG("错误: 无法获取建筑数据，ID=%d", buildingId);
  }
}


//初始化方法
void VillageLayer::initializeBasicProperties() {
  auto mapSize = _mapSprite->getContentSize();
  this->setContentSize(mapSize);

  // 使用左下角锚点
  this->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);

  CCLOG("VillageLayer basic properties initialized:");
  CCLOG("  Map size: %.0fx%.0f", mapSize.width, mapSize.height);
}
#pragma endregion

// 辅助方法
Sprite* VillageLayer::createMapSprite() {
  auto mapSprite = Sprite::create("Scene/LinedVillageScene.jpg");
  if (!mapSprite) {
    CCLOG("Error: Failed to load map image");
    return nullptr;
  }

  // 使用左下角锚点，位置设为(0,0)
  mapSprite->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
  mapSprite->setPosition(Vec2::ZERO);
  return mapSprite;
}

BuildingSprite* VillageLayer::getBuildingAtScreenPos(const Vec2& screenPos) {
  Vec2 worldPos = this->convertToNodeSpace(screenPos);
  return _buildingManager->getBuildingAtWorldPos(worldPos);
}
#pragma endregion

// 设置输入回调
void VillageLayer::setupInputCallbacks() {
    CCLOG("VillageLayer: Setting up input callbacks");

    // ========== 回调 1: 点击检测（用于 MoveMapController 判断点击了什么）==========
    _inputController->setOnTapCallback([this](const Vec2& screenPos) -> TapTarget {
        auto building = getBuildingAtScreenPos(screenPos);

        // --- 获取 HUD 层 (通过 Tag 100) ---
        auto scene = this->getScene();
        HUDLayer* hudLayer = nullptr;
        if (scene) {
            hudLayer = dynamic_cast<HUDLayer*>(scene->getChildByTag(100));
        }
        // ----------------------------------------

        if (building) {
            CCLOG("VillageLayer: Tap detected on building ID=%d", building->getBuildingId());
            return TapTarget::BUILDING;
        }

        // --- 如果点击了空白处 -> 隐藏底部菜单 ---
        if (hudLayer) {
            hudLayer->hideBuildingActions();
        }
        // ---------------------------------------------

        // TODO: 检测是否点击商店按钮
        // if (isShopButtonClicked(screenPos)) {
        //      return TapTarget::SHOP;
        // }

        return TapTarget::NONE;
        });

    // ========== 回调 2: 建筑选中（自动进入移动模式）==========
    _inputController->setOnBuildingSelectedCallback([this](const Vec2& screenPos) {
        auto building = getBuildingAtScreenPos(screenPos);

        // --- 获取 HUD 层 ---
        auto scene = this->getScene();
        HUDLayer* hudLayer = nullptr;
        if (scene) {
            hudLayer = dynamic_cast<HUDLayer*>(scene->getChildByTag(100));
        }
        // ------------------------

        if (building) {
            int buildingId = building->getBuildingId();
            CCLOG("VillageLayer: Building selected ID=%d", buildingId);

            // --- 显示底部操作菜单 ---
            if (hudLayer) {
                hudLayer->showBuildingActions(buildingId);
            }
            // -----------------------------

            // 【注意】原来的自动移动逻辑：
            // 如果你希望点击建筑只显示菜单，不直接开始拖动，建议先注释掉下面这行。
            // 这样点击是“选中”，长按或点击菜单里的“移动”按钮才是“移动”（类似COC逻辑）。
            //_moveBuildingController->startMoving(buildingId); 

        }
        else {
            CCLOG("VillageLayer: ERROR - Building selected callback triggered but no building found!");
        }
        });

    CCLOG("VillageLayer: Input callbacks configured");
}