#include "VillageLayer.h"
#include "../Controller/MoveMapController.h"
#include "../Controller/MoveBuildingController.h"
#include "../Util/GridMapUtils.h"
#include "../proj.win32/Constants.h"
#include "Manager/BuildingManager.h"
#include "Manager/VillageDataManager.h"
#include "Sprite/BattleUnitSprite.h"
#include "Sprite/BuildingSprite.h"   
#include "Model/BuildingConfig.h"      
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

  //  4. 先初始化建筑移动控制器（优先级更高）
  _moveBuildingController = new MoveBuildingController(this, _buildingManager);
  _moveBuildingController->setupTouchListener();
  
  //  设置短按建筑回调
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

  // 5. 后初始化地图移动控制器（优先级更低）
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
  auto speedupListener = EventListenerCustom::create("EVENT_BUILDING_SPEEDUP_COMPLETE",
                                                     [this](EventCustom* event) {
    int buildingId = *(int*)event->getUserData();

    CCLOG("VillageLayer: Building %d speedup complete, updating UI", buildingId);

    // 获取更新后的建筑数据
    auto dataManager = VillageDataManager::getInstance();
    auto building = dataManager->getBuildingById(buildingId);

    if (building && _buildingManager) {
      auto sprite = _buildingManager->getBuildingSprite(buildingId);
      if (sprite) {
        // 强制隐藏进度条和完成建造动画
        sprite->hideConstructionProgress();
        sprite->finishConstruction();

        // 更新建筑等级和状态
        sprite->updateBuilding(*building);
      }
    }
  });
  _eventDispatcher->addEventListenerWithSceneGraphPriority(speedupListener, this);
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

void VillageLayer::onBuildingPurchased(int buildingId) {
  CCLOG("VillageLayer: Building purchased ID=%d, entering placement mode", buildingId);

  auto dataManager = VillageDataManager::getInstance();
  auto building = dataManager->getBuildingById(buildingId);

  if (building) {
    // 只创建建筑精灵，不启动 MoveBuildingController
    auto sprite = _buildingManager->addBuilding(*building);
    
    // 通知 HUDLayer 开始放置流程
    auto scene = this->getScene();
    if (scene) {
      auto hudLayer = dynamic_cast<HUDLayer*>(scene->getChildByTag(100));
      if (hudLayer) {
        hudLayer->startBuildingPlacement(buildingId);
      }
    }

    CCLOG("VillageLayer: Building sprite created, waiting for placement");
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

      auto scene = this->getScene();
      HUDLayer* hudLayer = nullptr;
      if (scene) {
        hudLayer = dynamic_cast<HUDLayer*>(scene->getChildByTag(100));
      }

      if (building) {
        int buildingId = building->getBuildingId();
        CCLOG("VillageLayer: Building selected ID=%d", buildingId);

        auto dataManager = VillageDataManager::getInstance();
        auto buildingData = dataManager->getBuildingById(buildingId);

        if (buildingData) {
          if (buildingData->state == BuildingInstance::State::BUILT) {
            // 已完成的建筑：显示完整操作菜单
            if (hudLayer) {
              hudLayer->showBuildingActions(buildingId);
            }
          } else if (buildingData->state == BuildingInstance::State::CONSTRUCTING) {
            // 建造中的建筑：只显示基本信息（不显示升级/训练按钮）
            // 可以选择不显示菜单，或显示简化版菜单
            CCLOG("VillageLayer: Constructing building tapped, can still be moved by long press");
            // 不显示操作菜单，但可以长按移动
          } else if (buildingData->state == BuildingInstance::State::PLACING) {
            CCLOG("VillageLayer: PLACING building should not trigger select callback");
          }
        }
      }
    });

    CCLOG("VillageLayer: Input callbacks configured");
}

void VillageLayer::removeBuildingSprite(int buildingId) {
  if (_buildingManager) {
    _buildingManager->removeBuildingSprite(buildingId);
    CCLOG("VillageLayer: Removed building sprite ID=%d", buildingId);
  }
}

void VillageLayer::updateBuildingDisplay(int buildingId) {
  auto dataManager = VillageDataManager::getInstance();
  auto building = dataManager->getBuildingById(buildingId);
  
  if (building && _buildingManager) {
    _buildingManager->updateBuilding(buildingId, *building);
    CCLOG("VillageLayer: Updated building display for ID=%d", buildingId);
  }
}

void VillageLayer::updateBuildingPreviewPosition(int buildingId, const cocos2d::Vec2& worldPos) {
  auto sprite = _buildingManager->getBuildingSprite(buildingId);
  if (!sprite) {
    CCLOG("VillageLayer: Building sprite not found ID=%d", buildingId);
    return;
  }

  // 使用 GridMapUtils 转换坐标
  cocos2d::Vec2 gridPosFloat = GridMapUtils::pixelToGrid(worldPos);

  auto config = BuildingConfig::getInstance()->getConfig(sprite->getBuildingType());
  if (!config) return;

  // 计算建筑左下角网格坐标（触摸点作为建筑中心）
  float leftBottomGridX = gridPosFloat.x - config->gridWidth * 0.5f;
  float leftBottomGridY = gridPosFloat.y - config->gridHeight * 0.5f;

  int gridX = (int)std::round(leftBottomGridX);
  int gridY = (int)std::round(leftBottomGridY);

  // 计算对齐后的世界坐标
  cocos2d::Vec2 alignedWorldPos = GridMapUtils::getVisualPosition(
    gridX, gridY, sprite->getVisualOffset()
  );

  // 更新精灵位置（平滑移动）
  cocos2d::Vec2 currentPos = sprite->getPosition();
  cocos2d::Vec2 smoothPos = currentPos.lerp(alignedWorldPos, 0.3f);
  sprite->setPosition(smoothPos);

  // 更新网格坐标
  sprite->setGridPos(cocos2d::Vec2(gridX, gridY));

  // 更新数据层坐标（用于碰撞检测）
  auto dataManager = VillageDataManager::getInstance();
  dataManager->setBuildingPosition(buildingId, gridX, gridY);

  //  修复：isAreaOccupied() 需要 5 个参数
  bool canPlace = !dataManager->isAreaOccupied(
    gridX,
    gridY,
    config->gridWidth,    //  添加宽度参数
    config->gridHeight,   //  添加高度参数
    buildingId            //  忽略自己
  );

  // 显示视觉反馈
  sprite->setDraggingMode(true);
  sprite->setPlacementPreview(canPlace);

  // 通知 HUDLayer 更新按钮状态
  auto scene = this->getScene();
  if (scene) {
    auto hudLayer = dynamic_cast<HUDLayer*>(scene->getChildByTag(100));
    if (hudLayer) {
      hudLayer->updatePlacementUIState(canPlace);
    }
  }

  CCLOG("VillageLayer: Preview at grid(%d, %d) - %s",
        gridX, gridY, canPlace ? "VALID" : "INVALID");
}