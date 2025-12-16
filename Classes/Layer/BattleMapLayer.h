#pragma once
#ifndef __BATTLE_MAP_LAYER_H__
#define __BATTLE_MAP_LAYER_H__

#include "cocos2d.h"

class BuildingManager;
class MoveMapController;

class BattleMapLayer : public cocos2d::Layer {
public:
    virtual bool init() override;
    virtual ~BattleMapLayer();  // 添加析构函数声明
    virtual void update(float dt) override;  // 【新增】定时更新方法
    CREATE_FUNC(BattleMapLayer);

    // 重新加载地图 (用于"寻找下一个")
    void reloadMap();

private:
    cocos2d::Sprite* _mapSprite;
    BuildingManager* _buildingManager;
    MoveMapController* _inputController;

    void initializeMap();
    cocos2d::Sprite* createMapSprite();
};

#endif // __BATTLE_MAP_LAYER_H__
