#pragma once
#include <vector>
#include "VillageData.h"

/**
 * @brief 战斗地图数据（敌方阵型）
 * 用于存储随机生成或预设的敌方村庄布局
 */
struct BattleMapData {
    int difficulty;  // 难度等级 1-3
    std::vector<BuildingInstance> buildings;  // 敌方建筑列表
    int goldReward;   // 胜利奖励金币
    int elixirReward; // 胜利奖励圣水
    
    // 可掠夺资源（生成时计算）
    int lootableGold;       // 总可掠夺金币
    int lootableElixir;     // 总可掠夺圣水
    int goldStorageCount;   // 储金罐数量
    int elixirStorageCount; // 圣水瓶数量
    
    BattleMapData() 
        : difficulty(1)
        , goldReward(0)
        , elixirReward(0)
        , lootableGold(0)
        , lootableElixir(0)
        , goldStorageCount(0)
        , elixirStorageCount(0) {}
};
