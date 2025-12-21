// Model/ReplayData.h
#ifndef __REPLAY_DATA_H__
#define __REPLAY_DATA_H__

#include "cocos2d.h"
#include "Model/VillageData.h"
#include <vector>
#include <string>
#include <ctime>

// ========== 兵种部署事件 ==========
struct TroopDeployEvent {
    float timestamp;      // 相对战斗开始的时间（秒）
    int troopId;          // 兵种ID（1001=野蛮人，1002=弓箭手...）
    int gridX;            // 部署位置X
    int gridY;            // 部署位置Y

    // JSON 序列化
    cocos2d::ValueMap toValueMap() const;
    static TroopDeployEvent fromValueMap(const cocos2d::ValueMap& map);
};

// ========== 完整回放数据 ==========
struct BattleReplayData {
    // ========== 元数据 ==========
    int replayId;                                    // 回放ID（唯一标识）
    time_t timestamp;                                // 战斗时间戳
    std::string defenderName;                        // 防守者名称
    float battleDuration;                            // 战斗总时长（秒）

    // ========== 战斗结果 ==========
    int finalStars;                                  // 最终星数（0-3）
    int destructionPercentage;                       // 摧毁率（0-100）
    int lootedGold;                                  // 掠夺金币
    int lootedElixir;                                // 掠夺圣水
    std::map<int, int> usedTroops;                  // 消耗的兵种（troopId -> count）
    std::map<int, int> troopLevels;                 // 兵种等级（troopId -> level）

    // ========== 地图快照 ==========
    std::vector<BuildingInstance> initialBuildings; // 初始建筑布局
    int battleMapSeed;                               // 地图随机种子（用于重建）

    // ========== 兵种部署序列 ==========
    std::vector<TroopDeployEvent> troopEvents;      // 按时间排序的兵种部署事件

    // JSON 序列化
    cocos2d::ValueMap toValueMap() const;
    static BattleReplayData fromValueMap(const cocos2d::ValueMap& map);
};

// ========== 回放元数据（用于列表显示，轻量级）==========
struct ReplayMetadata {
    int replayId;
    time_t timestamp;
    std::string defenderName;
    int finalStars;
    int destructionPercentage;
    int lootedGold;
    int lootedElixir;
    std::map<int, int> usedTroops;    // ✅ 新增：显示兵种消耗
    float battleDuration;

    cocos2d::ValueMap toValueMap() const;
    static ReplayMetadata fromValueMap(const cocos2d::ValueMap& map);
};

#endif // __REPLAY_DATA_H__
