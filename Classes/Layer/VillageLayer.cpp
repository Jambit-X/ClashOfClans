#include "VillageLayer.h"
#include "../Controller/MoveMapController.h"
#include "../Controller/MoveBuildingController.h"
#include "../Util/GridMapUtils.h"
#include "../proj.win32/Constants.h"
#include "Manager/BuildingManager.h"
#include "Manager/VillageDataManager.h"
#include "ui/CocosGUI.h"
#include <iostream>

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

  // TODO: 调用 MoveMapController 开始建筑放置模式
  // _inputController->startBuildingPlacement(buildingId);

  // 暂时的临时实现：直接放置到随机位置
  auto dataManager = VillageDataManager::getInstance();
  auto building = dataManager->getBuildingById(buildingId);

  if (building) {
    // 临时：放置到 (0, 0) 位置
    dataManager->setBuildingPosition(buildingId,0,0);
    dataManager->setBuildingState(buildingId,
                                  BuildingInstance::State::CONSTRUCTING,
                                  time(nullptr) + 60);  // 1分钟后完成

    // 让 BuildingManager 创建精灵
    _buildingManager->addBuilding(*building);
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
    
    if (building) {
      CCLOG("VillageLayer: Tap detected on building ID=%d", building->getBuildingId());
      return TapTarget::BUILDING;
    }
    
    // TODO: 检测是否点击商店按钮
    // if (isShopButtonClicked(screenPos)) {
    //     return TapTarget::SHOP;
    // }
    
    return TapTarget::NONE;
  });
  
  // ========== 回调 2: 建筑选中（自动进入移动模式）==========
  _inputController->setOnBuildingSelectedCallback([this](const Vec2& screenPos) {
    auto building = getBuildingAtScreenPos(screenPos);
    
    if (building) {
      int buildingId = building->getBuildingId();
      CCLOG("VillageLayer: Auto-starting move mode for building ID=%d", buildingId);
      _moveBuildingController->startMoving(buildingId);
    } else {
      CCLOG("VillageLayer: ERROR - Building selected callback triggered but no building found!");
    }
  });
  
  CCLOG("VillageLayer: Input callbacks configured");
}