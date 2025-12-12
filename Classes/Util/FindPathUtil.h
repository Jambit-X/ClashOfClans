#ifndef __FIND_PATH_UTIL_H__
#define __FIND_PATH_UTIL_H__

#include "cocos2d.h"
#include "../Model/VillageData.h"  // ✅ 修正：引用 BuildingInstance 定义
#include <vector>
#include <unordered_map>

class FindPathUtil {
public:
    // 网格类型定义
    enum class GridType : uint8_t {
        EMPTY = 0,
        BUILDING = 1,
        WALL = 2,
        DECORATION = 3
    };

    static FindPathUtil* getInstance();
    static void destroyInstance();

    // =============================================================
    // 🔥 核心接口：智能寻找攻击路径 🔥
    // 输入：单位当前世界坐标，目标建筑实例
    // 输出：一系列世界坐标点（路径），如果无法到达返回空
    // =============================================================
    std::vector<cocos2d::Vec2> findPathToAttackBuilding(const cocos2d::Vec2& unitWorldPos, const BuildingInstance& targetBuilding);

    // 重新同步地图数据（当建筑位置改变时调用）
    void updatePathfindingMap();

    // 辅助：判断某格是否可走
    bool isWalkable(int gridX, int gridY) const;

    // ✅ 基础寻路接口（网格坐标）
    std::vector<cocos2d::Vec2> findPath(const cocos2d::Vec2& startGridPos, const cocos2d::Vec2& endGridPos);
    
    // ✅ 基础寻路接口（世界坐标）
    std::vector<cocos2d::Vec2> findPathInWorld(const cocos2d::Vec2& startWorldPos, const cocos2d::Vec2& endWorldPos);
    
    // 辅助：获取两点间的基础 A* 路径 (网格坐标 -> 网格坐标)
    std::vector<cocos2d::Vec2> findPathGrid(const cocos2d::Vec2& startGrid, const cocos2d::Vec2& endGrid);

private:
    FindPathUtil();
    ~FindPathUtil();

    static FindPathUtil* _instance;

    int _mapWidth;
    int _mapHeight;
    std::vector<uint8_t> _pathfindingMap; // 扁平化的一维数组存储地图数据

    // ✅ 性能优化：复用的 A* 数据结构（避免频繁分配）
    std::vector<int> _gScore;           // G值缓存
    std::vector<int> _cameFrom;         // 路径回溯表
    std::vector<bool> _closedSet;       // 已访问集合
    
    // 内部 A* 算法实现
    std::vector<cocos2d::Vec2> aStarSearch(int startX, int startY, int endX, int endY);

    // 启发式函数
    int heuristic(int x1, int y1, int x2, int y2) const;

    // 索引转换工具
    inline int toIndex(int x, int y) const { return y * _mapWidth + x; }
    inline void fromIndex(int index, int& x, int& y) const { x = index % _mapWidth; y = index / _mapWidth; }
};

#endif // __FIND_PATH_UTIL_H__