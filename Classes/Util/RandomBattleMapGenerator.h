#pragma once
#include "Model/BattleMapData.h"
#include <random>

/**
 * @brief 随机战斗地图生成器
 * 根据难度等级生成合理的敌方村庄布局
 */
class RandomBattleMapGenerator {
public:
    /**
     * @brief 生成随机地图
     * @param difficulty 难度等级(1-3)，0表示随机难度
     * @return 生成的战斗地图数据
     */
    static BattleMapData generate(int difficulty = 0);

private:
    // 建筑类型常量
    static constexpr int TOWNHALL = 1;
    static constexpr int CANNON = 301;
    static constexpr int ARCHER_TOWER = 302;
    static constexpr int WALL = 303;
    static constexpr int GOLD_MINE = 202;
    static constexpr int ELIXIR_COLLECTOR = 203;
    static constexpr int GOLD_STORAGE = 204;
    static constexpr int ELIXIR_STORAGE = 205;
    static constexpr int BOMB = 401;
    static constexpr int GIANT_BOMB = 404;

    // 地图边界（有效放置区域）
    static constexpr int MAP_MIN = 5;
    static constexpr int MAP_MAX = 40;
    static constexpr int CENTER_X = 24;
    static constexpr int CENTER_Y = 24;

    // 难度配置
    struct DifficultyConfig {
        int townHallLevel;
        int minDefense;
        int maxDefense;
        int minResource;
        int maxResource;
        int minWalls;
        int maxWalls;
        int minTraps;
        int maxTraps;
        int goldReward;
        int elixirReward;
    };

    static DifficultyConfig getDifficultyConfig(int difficulty);

    // 放置大本营
    static void placeTownHall(BattleMapData& map, int level);

    // 放置防御建筑 (需要知道围墙范围)
    static void placeDefenseBuildings(BattleMapData& map, int count, int townHallLevel, int innerRadius);

    // 放置资源建筑
    static void placeResourceBuildings(BattleMapData& map, int count, int townHallLevel);

    // 放置城墙 (返回围墙的半径)
    static int placeWalls(BattleMapData& map, int count, int townHallLevel);

    // 放置陷阱
    static void placeTraps(BattleMapData& map, int count, int townHallLevel);

    // 计算可掠夺资源
    static void calculateLootableResources(BattleMapData& map);

    // 查找有效位置
    static bool findValidPosition(int gridW, int gridH,
                                   int minX, int maxX, int minY, int maxY,
                                   const std::vector<BuildingInstance>& existing,
                                   int& outX, int& outY,
                                   std::mt19937& rng);

    // 检查位置是否有效
    static bool isPositionValid(int x, int y, int w, int h,
                                 const std::vector<BuildingInstance>& existing);

    // 获取建筑尺寸
    static void getBuildingSize(int type, int& outW, int& outH);

    // 生成下一个建筑ID
    static int nextBuildingId;
};
