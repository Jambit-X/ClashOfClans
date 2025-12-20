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
    
    // 计算初始 Z-Order
    int zOrder = GridMapUtils::calculateZOrder(gridX, gridY);
    
    // 飞行单位（气球兵）额外加 1000 偏移，确保始终在地面单位之上
    if (unit->getUnitTypeID() == UnitTypeID::BALLOON) {
        zOrder += 1000;
    }
    
    if (mapLayer) {
        mapLayer->addChild(unit, zOrder);
    } else {
        // Fallback (防崩): 如果还没加到 MapLayer，就加到自己身上
        this->addChild(unit, zOrder); 
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

// ========== 单位移除 ==========

void BattleTroopLayer::removeUnit(BattleUnitSprite* unit) {
    if (!unit) {
        CCLOG("BattleTroopLayer::removeUnit - ERROR: unit is nullptr!");
        return;
    }
    
    // 【修复】在删除前保存兵种信息，避免访问野指针
    std::string unitType = unit->getUnitType();
    float posX = unit->getPositionX();
    float posY = unit->getPositionY();
    
    CCLOG("BattleTroopLayer::removeUnit - START: Removing unit %s at position (%.1f, %.1f)", 
          unitType.c_str(), posX, posY);
    
    // 从列表中移除
    auto it = std::find(_units.begin(), _units.end(), unit);
    if (it != _units.end()) {
        _units.erase(it);
        CCLOG("BattleTroopLayer::removeUnit - Unit removed from _units vector (size now: %zu)", _units.size());
    } else {
        CCLOG("BattleTroopLayer::removeUnit - WARNING: Unit not found in _units vector!");
    }
    
    // 从显示树移除
    auto mapLayer = this->getParent();
    CCLOG("BattleTroopLayer::removeUnit - MapLayer: %p", mapLayer);
    
    if (mapLayer) {
        Node* unitParent = unit->getParent();
        CCLOG("BattleTroopLayer::removeUnit - Unit's parent: %p (MapLayer: %p)", unitParent, mapLayer);
        
        if (unitParent == mapLayer) {
            CCLOG("BattleTroopLayer::removeUnit - Removing from MapLayer...");
            mapLayer->removeChild(unit);
            CCLOG("BattleTroopLayer::removeUnit - Successfully removed from MapLayer");
        } else {
            CCLOG("BattleTroopLayer::removeUnit - Unit parent mismatch, removing from TroopLayer...");
            this->removeChild(unit);
            CCLOG("BattleTroopLayer::removeUnit - Successfully removed from TroopLayer");
        }
    } else {
        CCLOG("BattleTroopLayer::removeUnit - No MapLayer, removing from TroopLayer...");
        this->removeChild(unit);
        CCLOG("BattleTroopLayer::removeUnit - Successfully removed from TroopLayer");
    }
    
    CCLOG("BattleTroopLayer::removeUnit - COMPLETE: Unit %s removed successfully", unitType.c_str());
}


// ========== 墓碑系统 ==========

void BattleTroopLayer::spawnTombstone(const Vec2& position, UnitTypeID unitType) {
    CCLOG("===== TOMBSTONE DEBUG START =====");
    CCLOG("BattleTroopLayer::spawnTombstone - Creating tombstone at (%.1f, %.1f)", position.x, position.y);

    Sprite* tombstone = nullptr;

    // 根据兵种类型加载对应的墓碑图片
    if (unitType == UnitTypeID::BALLOON) {
        // 气球兵使用独立墓碑图片
        tombstone = Sprite::create("Animation/troop/balloon/balloon_death.png");
    } else {
        // 其他兵种从精灵图集获取墓碑帧
        std::string frameName;
        switch (unitType) {
            case UnitTypeID::BARBARIAN:
                frameName = "barbarian175.0.png";
                break;
            case UnitTypeID::ARCHER:
                frameName = "archer53.0.png";
                break;
            case UnitTypeID::GOBLIN:
                frameName = "goblin41.0.png";
                break;
            case UnitTypeID::GIANT:
                frameName = "giant99.0.png";
                break;
            case UnitTypeID::WALL_BREAKER:
                frameName = "wall_breaker1.0.png";
                break;
            default:
                frameName = "barbarian175.0.png";
                break;
        }

        auto frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(frameName);
        if (frame) {
            tombstone = Sprite::createWithSpriteFrame(frame);
        }
    }

    if (!tombstone) {
        CCLOG("BattleTroopLayer::spawnTombstone - ERROR: Failed to create tombstone sprite!");
        return;
    }

    // 设置位置
    tombstone->setPosition(position);
    tombstone->setAnchorPoint(Vec2(0.5f, 0.0f));

    // 设置缩放
    float scale = 0.8f;
    switch (unitType) {
        case UnitTypeID::BARBARIAN: scale = 0.8f; break;
        case UnitTypeID::ARCHER: scale = 0.75f; break;
        case UnitTypeID::GOBLIN: scale = 0.7f; break;
        case UnitTypeID::GIANT: scale = 1.2f; break;
        case UnitTypeID::WALL_BREAKER: scale = 0.65f; break;
        case UnitTypeID::BALLOON: scale = 1.0f; break;
    }
    tombstone->setScale(scale);

    // ✅ 强制设置为白色（防止继承兵种的红色）
    tombstone->setColor(Color3B::WHITE);
    tombstone->setOpacity(255);

    CCLOG("BattleTroopLayer::spawnTombstone - Tombstone color set to WHITE");

    // 添加到地图层
    auto mapLayer = this->getParent();
    CCLOG("BattleTroopLayer::spawnTombstone - MapLayer: %p", mapLayer);

    if (mapLayer) {
        CCLOG("BattleTroopLayer::spawnTombstone - Adding to MapLayer with Z-order 100...");
        mapLayer->addChild(tombstone, 100);
        CCLOG("BattleTroopLayer::spawnTombstone - Tombstone added successfully");
    } else {
        CCLOG("BattleTroopLayer::spawnTombstone - WARNING: No MapLayer, adding to TroopLayer...");
        this->addChild(tombstone, 100);
    }

    // ✅ 墓碑自动消失：停留 3 秒后淡出
    auto sequence = Sequence::create(
        DelayTime::create(3.0f),         // 停留 3 秒
        FadeOut::create(1.0f),            // 用 1 秒淡出
        RemoveSelf::create(),             // 自动移除
        nullptr
    );
    tombstone->runAction(sequence);

    CCLOG("BattleTroopLayer::spawnTombstone - COMPLETE: Tombstone created and will fade out after 3s");
    CCLOG("===== TOMBSTONE DEBUG END =====");
}

void BattleTroopLayer::clearAllTombstones() {
    auto mapLayer = this->getParent();
    
    for (auto tombstone : _tombstones) {
        if (mapLayer && tombstone->getParent() == mapLayer) {
            mapLayer->removeChild(tombstone);
        } else {
            this->removeChild(tombstone);
        }
    }
    
    _tombstones.clear();
    CCLOG("BattleTroopLayer: Cleared all tombstones");
}
