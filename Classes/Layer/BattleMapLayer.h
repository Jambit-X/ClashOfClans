// BattleMapLayer.h
// 战斗地图层声明，管理战斗场景的地图、建筑和输入控制

#pragma once
#ifndef __BATTLE_MAP_LAYER_H__
#define __BATTLE_MAP_LAYER_H__

#include "cocos2d.h"

class BuildingManager;
class MoveMapController;

class BattleMapLayer : public cocos2d::Layer {
public:
    virtual bool init() override;
    virtual ~BattleMapLayer();
    virtual void update(float dt) override;  // 定时更新方法
    CREATE_FUNC(BattleMapLayer);

    // 重新加载地图（用于"寻找下一个"）
    void reloadMap();

    // 获取BuildingManager
    BuildingManager* getBuildingManager() const { return _buildingManager; }

    void reloadMapFromData();  // 从现有数据重建（回放模式）
    
private:
    cocos2d::Sprite* _mapSprite;
    BuildingManager* _buildingManager;
    MoveMapController* _inputController;

    void initializeMap();
    cocos2d::Sprite* createMapSprite();

    // 输出建筑布局信息
    void logBuildingLayout(const std::string& context);
};

#endif // __BATTLE_MAP_LAYER_H__
