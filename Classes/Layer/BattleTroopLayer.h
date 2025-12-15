#pragma once

#include "cocos2d.h"
#include "../Sprite/BattleUnitSprite.h"
#include <vector>

USING_NS_CC;

/**
 * @brief 战斗单位层 - 管理所有军队单位的显示
 * 
 * 职责：
 * 1. 管理所有战斗单位的生命周期
 * 2. 提供单位生成、移除接口
 * 3. 纯粹的显示层，不包含控制逻辑
 */
class BattleTroopLayer : public Layer {
public:
    static BattleTroopLayer* create();
    
    virtual bool init() override;
    
    /**
     * @brief 在指定网格位置生成单位
     * @param unitType 单位类型，如 "Barbarian"
     * @param gridX 网格X坐标
     * @param gridY 网格Y坐标
     * @return 生成的单位精灵
     */
    BattleUnitSprite* spawnUnit(const std::string& unitType, int gridX, int gridY);
    
    /**
     * @brief 批量生成单位（用于测试/快速初始化）
     * @param unitType 单位类型
     * @param spacing 网格间隔距离
     */
    void spawnUnitsGrid(const std::string& unitType, int spacing = 3);
    
    /**
     * @brief 移除所有单位
     */
    void removeAllUnits();
    
    /**
     * @brief 获取所有单位
     */
    const std::vector<BattleUnitSprite*>& getAllUnits() const { return _units; }
    
private:
    // 所有单位列表
    std::vector<BattleUnitSprite*> _units;
    
    // 网格常量（从 GridMapUtils 获取）
    static const int GRID_WIDTH = 44;
    static const int GRID_HEIGHT = 44;
};
