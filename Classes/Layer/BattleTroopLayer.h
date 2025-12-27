// BattleTroopLayer.h
// 战斗单位层声明，管理所有军队单位的显示和墓碑系统

#pragma once

#include "cocos2d.h"
#include "../Sprite/BattleUnitSprite.h"
#include <vector>

USING_NS_CC;

// 战斗单位层 - 管理所有军队单位的显示
// 职责：
// 1. 管理所有战斗单位的生命周期
// 2. 提供单位生成、移除接口
// 3. 纯粹的显示层，不包含控制逻辑
// 4. 管理战斗墓碑显示
class BattleTroopLayer : public Layer {
public:
    static BattleTroopLayer* create();
    
    virtual bool init() override;
    
    // 在指定网格位置生成单位
    BattleUnitSprite* spawnUnit(const std::string& unitType, int gridX, int gridY);
    
    // 批量生成单位（用于测试/快速初始化）
    void spawnUnitsGrid(const std::string& unitType, int spacing = 3);
    
    // 移除所有单位
    void removeAllUnits();
    
    // 获取所有单位
    const std::vector<BattleUnitSprite*>& getAllUnits() const { return _units; }
    
    // 移除指定单位（死亡时调用）
    void removeUnit(BattleUnitSprite* unit);
    
    // 在指定位置生成墓碑
    void spawnTombstone(const Vec2& position, UnitTypeID unitType);

    // 清除所有墓碑（战斗结束时调用）
    void clearAllTombstones();
    
private:
    std::vector<BattleUnitSprite*> _units;  // 所有单位列表
    std::vector<Node*> _tombstones;         // 墓碑列表
    
    static const int GRID_WIDTH = 44;
    static const int GRID_HEIGHT = 44;
};
