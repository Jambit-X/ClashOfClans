#include "VillageLayer.h"
#include "../Controller/InputController.h"
#include "../Util/GridMapUtils.h"
#include "../proj.win32/Constants.h"
#include "Manager/BuildingManager.h"
#include "Manager/VillageDataManager.h"
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

  // 3. 创建输入控制器（控制器会负责初始化缩放和位置）
  _inputController = new InputController(this);
  _inputController->setupInputListeners();

  // 3. 初始化建筑管理器（新增）
  _buildingManager = new BuildingManager(this);

  // 4. 启动建筑更新（检查建造完成）
  this->schedule([this](float dt) {
    _buildingManager->update(dt);
  }, 1.0f, "building_update");  // 每秒检查一次

  CCLOG("VillageLayer initialized with InputController");

  return true;
}

void VillageLayer::cleanup() {
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

  // TODO: 调用 InputController 开始建筑放置模式
  // _inputController->startBuildingPlacement(buildingId);

  // 暂时的临时实现：直接放置到随机位置
  auto dataManager = VillageDataManager::getInstance();
  auto building = dataManager->getBuildingById(buildingId);

  if (building) {
    // 临时：放置到 (10, 10) 位置
    dataManager->setBuildingPosition(buildingId, 10, 10);
    dataManager->setBuildingState(buildingId,
                                  BuildingInstance::State::CONSTRUCTING,
                                  time(nullptr) + 60);  // 1分钟后完成

    // 让 BuildingManager 创建精灵
    _buildingManager->addBuilding(*building);
  }
}


#pragma region 初始化方法
void VillageLayer::initializeBasicProperties() {
  auto mapSize = _mapSprite->getContentSize();
  this->setContentSize(mapSize);

  // 使用左下角锚点
  this->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);

  CCLOG("VillageLayer basic properties initialized:");
  CCLOG("  Map size: %.0fx%.0f", mapSize.width, mapSize.height);
}
#pragma endregion

#pragma region 辅助方法
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
#pragma endregion