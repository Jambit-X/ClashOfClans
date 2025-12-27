// RandomBattleMapGenerator.h
// 随机战斗地图生成器，根据难度等级生成敌方村庄布局

#pragma once
#include "Model/BattleMapData.h"
#include <random>

/**
 * @brief 随机战斗地图生成器
 * 
 * 功能说明：
 * - 根据难度等级（1-3）生成合理的敌方村庄
 * - 自动放置大本营、防御建筑、资源建筑、城墙和陷阱
 * - 确保建筑布局合理，城墙形成闭环
 * - 计算可掠夺资源
 * 
 * 使用方式：
 * BattleMapData map = RandomBattleMapGenerator::generate(2); // 生成中等难度地图
 */
class RandomBattleMapGenerator {
public:
    /**
     * @brief 生成随机战斗地图
     * @param difficulty 难度等级(1=简单, 2=中等, 3=困难, 0=随机)
     * @return 生成的战斗地图数据
     * 
     * 生成流程：
     * 1. 放置大本营（中心位置）
     * 2. 生成城墙围栏
     * 3. 放置防御建筑（优先城墙内部）
     * 4. 放置资源建筑
     * 5. 放置陷阱
     * 6. 计算可掠夺资源
     */
    static BattleMapData generate(int difficulty = 0);

private:
    // ========== 建筑类型常量 ==========
    
    static constexpr int TOWNHALL = 1;           // 大本营
    static constexpr int CANNON = 301;           // 加农炮
    static constexpr int ARCHER_TOWER = 302;     // 箭塔
    static constexpr int WALL = 303;             // 城墙
    static constexpr int GOLD_MINE = 202;        // 金矿
    static constexpr int ELIXIR_COLLECTOR = 203; // 圣水收集器
    static constexpr int GOLD_STORAGE = 204;     // 金库
    static constexpr int ELIXIR_STORAGE = 205;   // 圣水瓶
    static constexpr int BOMB = 401;             // 炸弹
    static constexpr int GIANT_BOMB = 404;       // 巨型炸弹

    // ========== 地图参数 ==========
    
    static constexpr int MAP_MIN = 5;      // 可放置区域最小坐标
    static constexpr int MAP_MAX = 40;     // 可放置区域最大坐标
    static constexpr int CENTER_X = 24;    // 地图中心X坐标
    static constexpr int CENTER_Y = 24;    // 地图中心Y坐标

    // ========== 难度配置结构 ==========
    
    /**
     * @brief 难度配置参数
     * 
     * 各字段说明：
     * - townHallLevel: 大本营等级
     * - minDefense/maxDefense: 防御建筑数量范围
     * - minResource/maxResource: 资源建筑数量范围
     * - minWalls/maxWalls: 城墙数量范围
     * - minTraps/maxTraps: 陷阱数量范围
     * - goldReward/elixirReward: 胜利奖励
     */
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

    // ========== 核心生成函数 ==========
    
    // 获取难度配置
    static DifficultyConfig getDifficultyConfig(int difficulty);

    // 放置大本营（4x4建筑，居中）
    static void placeTownHall(BattleMapData& map, int level);

    // 放置防御建筑（加农炮、箭塔等）
    // innerRadius: 城墙半径，用于优先内部放置
    static void placeDefenseBuildings(BattleMapData& map, int count, int townHallLevel, int innerRadius);

    // 放置资源建筑（金矿、圣水、存储建筑）
    static void placeResourceBuildings(BattleMapData& map, int count, int townHallLevel);

    // 放置城墙（形成闭合矩形）
    // 返回值：围墙半径，供防御建筑放置参考
    static int placeWalls(BattleMapData& map, int count, int townHallLevel);

    // 放置陷阱（炸弹、巨型炸弹）
    static void placeTraps(BattleMapData& map, int count, int townHallLevel);

    // 计算可掠夺资源（基于存储建筑容量）
    static void calculateLootableResources(BattleMapData& map);

    // ========== 辅助函数 ==========
    
    /**
     * @brief 在指定区域内查找有效放置位置
     * @param gridW 建筑宽度
     * @param gridH 建筑高度
     * @param minX 区域最小X
     * @param maxX 区域最大X
     * @param minY 区域最小Y
     * @param maxY 区域最大Y
     * @param existing 已有建筑列表
     * @param outX 输出：找到的X坐标
     * @param outY 输出：找到的Y坐标
     * @param rng 随机数生成器
     * @return 是否找到有效位置
     * 
     * 策略：随机尝试100次
     */
    static bool findValidPosition(int gridW, int gridH,
                                   int minX, int maxX, int minY, int maxY,
                                   const std::vector<BuildingInstance>& existing,
                                   int& outX, int& outY,
                                   std::mt19937& rng);

    /**
     * @brief 检查位置是否有效（边界和碰撞检测）
     * @param x 目标X坐标
     * @param y 目标Y坐标
     * @param w 建筑宽度
     * @param h 建筑高度
     * @param existing 已有建筑列表
     * @return 是否有效（不越界且不重叠）
     */
    static bool isPositionValid(int x, int y, int w, int h,
                                 const std::vector<BuildingInstance>& existing);

    /**
     * @brief 获取建筑尺寸
     * @param type 建筑类型ID
     * @param outW 输出：建筑宽度
     * @param outH 输出：建筑高度
     * 
     * 从BuildingConfig读取，失败时返回默认值3x3
     */
    static void getBuildingSize(int type, int& outW, int& outH);

    // 建筑ID生成器（从10000开始递增）
    static int nextBuildingId;
};
