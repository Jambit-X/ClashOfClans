// BattleMapData.h
// 战斗地图数据结构，存储敌方阵型和奖励信息

#pragma once
#include <vector>
#include "VillageData.h"

// 战斗地图数据（敌方阵型）
struct BattleMapData {
    int difficulty;  // 难度等级 1-3
    std::vector<BuildingInstance> buildings;  // 敌方建筑列表
    int goldReward;   // 胜利金币奖励
    int elixirReward; // 胜利药水奖励
    
    // 可掠夺资源统计
    int lootableGold;       // 总可掠夺金币
    int lootableElixir;     // 总可掠夺药水
    int goldStorageCount;   // 金币仓库数量
    int elixirStorageCount; // 药水仓库数量
    
    BattleMapData() 
        : difficulty(1)
        , goldReward(0)
        , elixirReward(0)
        , lootableGold(0)
        , lootableElixir(0)
        , goldStorageCount(0)
        , elixirStorageCount(0) {}
};
