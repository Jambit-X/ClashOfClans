#include "BattleMapLayer.h"
#include "Manager/BuildingManager.h"
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

    // 2. 初始化建筑管理器 (只读模式，不传递 Layer 进行交互)
    _buildingManager = new BuildingManager(this);
    // 注意：BuildingManager 的构造函数已经调用了 loadBuildingsFromData()，不要重复调用

    // 3. 初始化地图移动控制器 (仅拖拽缩放)
    _inputController = new MoveMapController(this);
    _inputController->setupInputListeners();
    // 注意：这里我们故意不设置"建筑点击回调"，也不创建 MoveBuildingController
    // 从而实现"只读"效果

    // 【新增】启动定时更新，用于实时反映建筑受损状态（isDestroyed）
    this->scheduleUpdate();

    return true;
}

void BattleMapLayer::reloadMap() {
    // 简单的刷新逻辑：清除现有建筑，重新加载
    // 在真实逻辑中，这里应该先去 VillageDataManager 切换敌方数据源
    if (_buildingManager) {
        // 先简单地刷新一下日志
        CCLOG("BattleMapLayer: Reloading map data...");
        // 实际项目中需要: _buildingManager->reloadFromData(newEnemyData);
    }
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
