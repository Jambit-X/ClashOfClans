#include "BattleTroopLayer.h"
#include "../Manager/AnimationManager.h"
#include "../Util/GridMapUtils.h"

USING_NS_CC;

BattleTroopLayer* BattleTroopLayer::create() {
    auto layer = new (std::nothrow) BattleTroopLayer();
    if (layer && layer->init()) {
        layer->autorelease();
        return layer;
    }
    CC_SAFE_DELETE(layer);
    return nullptr;
}

bool BattleTroopLayer::init() {
    if (!Layer::init()) {
        return false;
    }
    
    this->setAnchorPoint(Vec2::ZERO);
    this->setPosition(Vec2::ZERO);
    
    CCLOG("BattleTroopLayer: Initialized");
    
    return true;
}

// ========== 单位生成 ==========

BattleUnitSprite* BattleTroopLayer::spawnUnit(const std::string& unitType, int gridX, int gridY) {
    // 边界检查
    if (gridX < 0 || gridX >= GRID_WIDTH || gridY < 0 || gridY >= GRID_HEIGHT) {
        CCLOG("BattleTroopLayer: Invalid grid position (%d, %d)", gridX, gridY);
        return nullptr;
    }
    
    // 创建单位
    auto unit = BattleUnitSprite::create(unitType);
    if (!unit) {
        CCLOG("BattleTroopLayer: Failed to create unit '%s'", unitType.c_str());
        return nullptr;
    }
    
    // 设置位置
    unit->teleportToGrid(gridX, gridY);
    unit->playIdleAnimation();
    
    // 添加到层级
    // 【修改】添加到父节点 (MapLayer) 以便与建筑进行统一 Z 序排序
    auto mapLayer = this->getParent();
    if (mapLayer) {
        mapLayer->addChild(unit);
    } else {
        // Fallback (防崩): 如果还没加到 MapLayer，就加到自己身上
        this->addChild(unit); 
    }
    _units.push_back(unit);
    
    CCLOG("BattleTroopLayer: Spawned %s at grid(%d, %d)", unitType.c_str(), gridX, gridY);
    return unit;
}

void BattleTroopLayer::spawnUnitsGrid(const std::string& unitType, int spacing) {
    int count = 0;
    
    for (int gridY = 0; gridY < GRID_HEIGHT; gridY += spacing) {
        for (int gridX = 0; gridX < GRID_WIDTH; gridX += spacing) {
            if (spawnUnit(unitType, gridX, gridY)) {
                count++;
            }
        }
    }
    
    CCLOG("BattleTroopLayer: Spawned %d units in grid pattern", count);
}

void BattleTroopLayer::removeAllUnits() {
    for (auto unit : _units) {
        this->removeChild(unit);
    }
    _units.clear();
    CCLOG("BattleTroopLayer: Removed all units");
}