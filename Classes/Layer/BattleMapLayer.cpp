#include "BattleMapLayer.h"
#include "Manager/BuildingManager.h"
#include "Manager/VillageDataManager.h"
#include "Controller/MoveMapController.h"

USING_NS_CC;

BattleMapLayer::~BattleMapLayer() {
    // 清理 MoveMapController（它会自动清理事件监听器）
    if (_inputController) {
        delete _inputController;
        _inputController = nullptr;
    }
    
    // 清理 BuildingManager
    if (_buildingManager) {
        delete _buildingManager;
        _buildingManager = nullptr;
    }
    
    CCLOG("BattleMapLayer: Destroyed and cleaned up resources");
}

bool BattleMapLayer::init() {
    if (!Layer::init()) return false;

    // 1. 创建地图背景
    _mapSprite = createMapSprite();
    this->addChild(_mapSprite);

    // 设置 Layer 大小与地图一致
    if (_mapSprite) {
        this->setContentSize(_mapSprite->getContentSize());
        this->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
    }

    // 2. 初始化建筑管理器（战斗场景：isBattleScene = true）
    _buildingManager = new BuildingManager(this, true);
    // 注意：BuildingManager 的构造函数已经调用了 loadBuildingsFromData()，不要重复调用

    // 3. 初始化地图移动控制器 (仅拖拽缩放)
    _inputController = new MoveMapController(this);
    _inputController->setupInputListeners();
    // 注意：这里我们故意不设置"建筑点击回调"，也不创建 MoveBuildingController
    // 从而实现"只读"效果

    // 【新增】启动定时更新，用于实时反映建筑受损状态（isDestroyed）
    this->scheduleUpdate();

    // 【新增】监听目标锁定事件，显示 Beacon
    auto listener = EventListenerCustom::create("EVENT_UNIT_TARGET_LOCKED", [this](EventCustom* event) {
        if (!_buildingManager) return;
        // 安全转换 ID
        intptr_t rawId = reinterpret_cast<intptr_t>(event->getUserData());
        int targetID = static_cast<int>(rawId);
        
        BuildingSprite* b = _buildingManager->getBuildingSprite(targetID);
        if (b) {
            b->showTargetBeacon();
        }
    });
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}

void BattleMapLayer::reloadMap() {
    CCLOG("BattleMapLayer: Reloading map with new random data...");
    
    // 1. 生成新的随机地图
    auto dataManager = VillageDataManager::getInstance();
    dataManager->generateRandomBattleMap(0);  // 随机难度
    
    // 2. 清理旧的 BuildingManager
    if (_buildingManager) {
        delete _buildingManager;
        _buildingManager = nullptr;
    }
    
    // 3. 创建新的 BuildingManager（会自动加载新的战斗地图数据）
    _buildingManager = new BuildingManager(this, true);
    
    CCLOG("BattleMapLayer: Map reloaded with %zu buildings", 
          dataManager->getBattleMapData().buildings.size());
}

Sprite* BattleMapLayer::createMapSprite() {
    // 复用原来的地图图片
    auto mapSprite = Sprite::create("Scene/LinedVillageScene.jpg");
    if (!mapSprite) {
        CCLOG("Error: Failed to load map image");
        return nullptr;
    }
    mapSprite->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
    mapSprite->setPosition(Vec2::ZERO);
    return mapSprite;
}

void BattleMapLayer::update(float dt) {
    Layer::update(dt);
    
    // 【关键修复】调用 BuildingManager 的更新方法，实时同步建筑的 isDestroyed 状态
    // 这会让战斗中被摧毁的建筑立即变红
    if (_buildingManager) {
        _buildingManager->update(dt);
    }
}
