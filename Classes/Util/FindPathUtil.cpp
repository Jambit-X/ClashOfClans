// FindPathUtil.cpp
// 寻路工具实现，提供A*寻路算法和智能攻击路径查找功能

#include "FindPathUtil.h"
#include "../Manager/VillageDataManager.h"
#include "../Model/BuildingConfig.h"
#include "GridMapUtils.h"
#include <queue>
#include <algorithm>
#include <cmath>

USING_NS_CC;

FindPathUtil* FindPathUtil::_instance = nullptr;

FindPathUtil* FindPathUtil::getInstance() {
    if (!_instance) _instance = new FindPathUtil();
    return _instance;
}

void FindPathUtil::destroyInstance() {
    CC_SAFE_DELETE(_instance);
}

FindPathUtil::FindPathUtil()
    : _mapWidth(GridMapUtils::GRID_WIDTH)
    , _mapHeight(GridMapUtils::GRID_HEIGHT) {
    
    int mapSize = _mapWidth * _mapHeight;
    
    // 初始化寻路地图数组
    _pathfindingMap.resize(mapSize, 0);
    
    // 预分配A*算法所需的内存，避免频繁分配
    _gScore.resize(mapSize, INT_MAX);
    _cameFrom.resize(mapSize, -1);
    _closedSet.resize(mapSize, false);
    
    CCLOG("FindPathUtil: Initialized with optimized memory pools (map size: %dx%d = %d cells)", 
          _mapWidth, _mapHeight, mapSize);
    CCLOG("  -> Pathfinding map will be updated by VillageDataManager");
}

FindPathUtil::~FindPathUtil() {
    _pathfindingMap.clear();
    _gScore.clear();
    _cameFrom.clear();
    _closedSet.clear();
}

// ===================================================================================
// 核心功能：智能攻击寻路
// ===================================================================================

std::vector<Vec2> FindPathUtil::findPathToAttackBuilding(const Vec2& unitWorldPos, const BuildingInstance& building, int attackRange) {
    // 将单位的世界坐标转换为网格坐标
    Vec2 startGridPos = GridMapUtils::pixelToGrid(unitWorldPos);
    int startX = static_cast<int>(std::floor(startGridPos.x));
    int startY = static_cast<int>(std::floor(startGridPos.y));

    // 获取目标建筑的配置信息
    auto config = BuildingConfig::getInstance()->getConfig(building.type);
    if (!config) return {};

    int bX = building.gridX;
    int bY = building.gridY;
    int bW = config->gridWidth;
    int bH = config->gridHeight;

    // 候选攻击位置结构
    struct CandidateSpot {
        int x, y;
        float distSq;  // 到起点的距离平方
    };
    std::vector<CandidateSpot> candidates;

    // 搜索建筑周围符合攻击范围的所有可行位置
    for (int x = bX - attackRange; x <= bX + bW + attackRange - 1; ++x) {
        for (int y = bY - attackRange; y <= bY + bH + attackRange - 1; ++y) {
            // 跳过建筑内部的格子
            if (x >= bX && x < bX + bW && y >= bY && y < bY + bH) continue;

            // 计算到建筑的切比雪夫距离（最大坐标差）
            int distToBuilding = 0;
            
            if (x < bX) {
                distToBuilding = std::max(distToBuilding, bX - x);
            } else if (x >= bX + bW) {
                distToBuilding = std::max(distToBuilding, x - (bX + bW) + 1);
            }
            
            if (y < bY) {
                distToBuilding = std::max(distToBuilding, bY - y);
            } else if (y >= bY + bH) {
                distToBuilding = std::max(distToBuilding, y - (bY + bH) + 1);
            }
            
            // 检查距离是否符合攻击范围要求
            if (attackRange == 0) {
                if (distToBuilding != 0) continue;
            } else {
                if (distToBuilding > attackRange || distToBuilding == 0) continue;
            }

            // 检查该位置是否可通行
            if (isWalkable(x, y)) {
                float dx = static_cast<float>(x - startX);
                float dy = static_cast<float>(y - startY);
                float distSq = dx * dx + dy * dy;
                candidates.push_back({ x, y, distSq });
            }
        }
    }

    if (candidates.empty()) {
        return {};
    }

    // 按距离排序，优先尝试最近的位置
    std::sort(candidates.begin(), candidates.end(), [](const CandidateSpot& a, const CandidateSpot& b) {
        return a.distSq < b.distSq;
    });

    // 尝试前5个最近的候选位置
    int attempts = std::min((int)candidates.size(), 5);

    for (int i = 0; i < attempts; ++i) {
        int targetX = candidates[i].x;
        int targetY = candidates[i].y;

        // 使用A*算法查找路径
        std::vector<Vec2> gridPath = findPathGrid(Vec2(startX, startY), Vec2(targetX, targetY));

        if (!gridPath.empty()) {
            // 转换为世界坐标路径（跳过起点）
            std::vector<Vec2> worldPath;
            worldPath.reserve(gridPath.size());

            for (size_t k = 1; k < gridPath.size(); ++k) {
                worldPath.push_back(GridMapUtils::gridToPixelCenter((int)gridPath[k].x, (int)gridPath[k].y));
            }

            // 如果路径为空但有目标点，至少返回目标点
            if (worldPath.empty() && gridPath.size() >= 1) {
                worldPath.push_back(GridMapUtils::gridToPixelCenter(targetX, targetY));
            }

            return worldPath;
        }
    }

    return {};
}

// ===================================================================================
// 地图数据更新
// ===================================================================================

void FindPathUtil::updatePathfindingMap() {
    // 清空地图数据
    std::fill(_pathfindingMap.begin(), _pathfindingMap.end(), 0);

    auto dataManager = VillageDataManager::getInstance();
    const auto& buildings = dataManager->getAllBuildings();

    for (const auto& b : buildings) {
        // 跳过正在放置的建筑
        if (b.state == BuildingInstance::State::PLACING) continue;
        
        // 跳过已摧毁的建筑
        if (b.isDestroyed || b.currentHP <= 0) continue;

        // 跳过陷阱（type 400-499），陷阱不阻挡寻路
        if (b.type >= 400 && b.type < 500) continue;

        auto config = BuildingConfig::getInstance()->getConfig(b.type);
        if (!config) continue;

        // 区分城墙和普通建筑
        GridType gridType;
        if (b.type == 303) {  // 城墙
            gridType = GridType::WALL;
        } else {
            gridType = GridType::BUILDING;
        }

        // 标记建筑占用的所有格子
        for (int x = b.gridX; x < b.gridX + config->gridWidth; ++x) {
            for (int y = b.gridY; y < b.gridY + config->gridHeight; ++y) {
                if (x >= 0 && x < _mapWidth && y >= 0 && y < _mapHeight) {
                    _pathfindingMap[toIndex(x, y)] = static_cast<uint8_t>(gridType);
                }
            }
        }
    }
}

bool FindPathUtil::isWalkable(int gridX, int gridY) const {
    if (gridX < 0 || gridX >= _mapWidth || gridY < 0 || gridY >= _mapHeight) return false;
    return _pathfindingMap[toIndex(gridX, gridY)] == static_cast<uint8_t>(GridType::EMPTY);
}

// ===================================================================================
// 忽略城墙的寻路（炸弹人专用）
// ===================================================================================

std::vector<Vec2> FindPathUtil::findPathIgnoringWalls(const Vec2& startWorldPos, const Vec2& endWorldPos) {
    // 世界坐标转网格坐标
    Vec2 startGridPos = GridMapUtils::pixelToGrid(startWorldPos);
    Vec2 endGridPos = GridMapUtils::pixelToGrid(endWorldPos);
    
    int startX = static_cast<int>(std::floor(startGridPos.x));
    int startY = static_cast<int>(std::floor(startGridPos.y));
    int endX = static_cast<int>(std::floor(endGridPos.x));
    int endY = static_cast<int>(std::floor(endGridPos.y));
    
    // 调用A*并启用忽略城墙模式
    std::vector<Vec2> gridPath = aStarSearch(startX, startY, endX, endY, true);
    
    if (gridPath.empty()) {
        return {};
    }
    
    // 移除起点
    if (gridPath.size() > 1) {
        gridPath.erase(gridPath.begin());
    }
    
    // 转换为世界坐标
    std::vector<Vec2> worldPath;
    worldPath.reserve(gridPath.size());
    
    for (const auto& gridPos : gridPath) {
        Vec2 worldPos = GridMapUtils::gridToPixelCenter(
            static_cast<int>(gridPos.x), 
            static_cast<int>(gridPos.y)
        );
        worldPath.push_back(worldPos);
    }
    
    return worldPath;
}

// ===================================================================================
// A*寻路算法核心实现
// ===================================================================================

std::vector<Vec2> FindPathUtil::aStarSearch(int startX, int startY, int endX, int endY, bool ignoreWalls) {
    // A*节点结构
    struct AStarNode {
        int x, y;
        int fCost;  // 总代价：g + h

        bool operator>(const AStarNode& other) const {
            return fCost > other.fCost;
        }
    };
    
    // 根据ignoreWalls参数定义通行性判断逻辑
    auto isWalkableInternal = [&](int gridX, int gridY) -> bool {
        if (gridX < 0 || gridX >= _mapWidth || gridY < 0 || gridY >= _mapHeight) return false;
        
        uint8_t cellType = _pathfindingMap[toIndex(gridX, gridY)];
        
        if (cellType == static_cast<uint8_t>(GridType::EMPTY)) {
            return true;  // 空地可通行
        }
        
        if (ignoreWalls && cellType == static_cast<uint8_t>(GridType::WALL)) {
            return true;  // 忽略城墙模式下，城墙可通行
        }
        
        return false;  // 其他建筑不可通行
    };
    
    // 边界与合法性检查
    if (!isWalkableInternal(startX, startY)) return {};
    if (!isWalkableInternal(endX, endY)) return {};
    if (startX == endX && startY == endY) return {};

    // 复用预分配的内存，只重置数据
    std::fill(_gScore.begin(), _gScore.end(), INT_MAX);
    std::fill(_cameFrom.begin(), _cameFrom.end(), -1);
    std::fill(_closedSet.begin(), _closedSet.end(), false);

    // 优先队列（最小堆）
    std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> openSet;

    int startIndex = toIndex(startX, startY);
    _gScore[startIndex] = 0;
    openSet.push({ startX, startY, heuristic(startX, startY, endX, endY) });

    // 8方向移动
    const int dirs[8][2] = {
        {0, 1}, {0, -1}, {-1, 0}, {1, 0},      // 上下左右
        {-1, -1}, {1, -1}, {-1, 1}, {1, 1}     // 对角线
    };

    while (!openSet.empty()) {
        AStarNode current = openSet.top();
        openSet.pop();

        int currIndex = toIndex(current.x, current.y);

        // 到达终点，重建路径
        if (current.x == endX && current.y == endY) {
            std::vector<Vec2> path;
            int traceIndex = currIndex;
            
            while (traceIndex != -1) {
                int tx, ty;
                fromIndex(traceIndex, tx, ty);
                path.push_back(Vec2(tx, ty));
                traceIndex = _cameFrom[traceIndex];
            }
            
            std::reverse(path.begin(), path.end());
            return path;
        }

        // 跳过已访问的节点
        if (_closedSet[currIndex]) continue;
        _closedSet[currIndex] = true;

        // 遍历8个邻居
        for (int i = 0; i < 8; ++i) {
            int nx = current.x + dirs[i][0];
            int ny = current.y + dirs[i][1];

            if (!isWalkableInternal(nx, ny)) continue;

            // 对角线移动代价14，直线移动代价10（近似√2*10≈14）
            int moveCost = (dirs[i][0] != 0 && dirs[i][1] != 0) ? 14 : 10;
            int tentativeG = _gScore[currIndex] + moveCost;
            int neighborIndex = toIndex(nx, ny);

            // 如果找到更优路径，更新节点
            if (tentativeG < _gScore[neighborIndex]) {
                _cameFrom[neighborIndex] = currIndex;
                _gScore[neighborIndex] = tentativeG;
                int f = tentativeG + heuristic(nx, ny, endX, endY);
                openSet.push({ nx, ny, f });
            }
        }
    }

    return {};  // 无路径
}

// ===================================================================================
// 辅助函数和基础接口
// ===================================================================================

// 启发式函数：八方向曼哈顿距离（考虑对角线）
int FindPathUtil::heuristic(int x1, int y1, int x2, int y2) const {
    int dx = std::abs(x1 - x2);
    int dy = std::abs(y1 - y2);
    // 对角线距离：min(dx,dy)段斜走，剩余段直走
    return 10 * (dx + dy) - 6 * std::min(dx, dy);
}

std::vector<Vec2> FindPathUtil::findPathGrid(const Vec2& startGrid, const Vec2& endGrid) {
    return aStarSearch((int)startGrid.x, (int)startGrid.y, (int)endGrid.x, (int)endGrid.y);
}

std::vector<Vec2> FindPathUtil::findPath(const Vec2& startGridPos, const Vec2& endGridPos) {
    int startX = static_cast<int>(startGridPos.x);
    int startY = static_cast<int>(startGridPos.y);
    int endX = static_cast<int>(endGridPos.x);
    int endY = static_cast<int>(endGridPos.y);
    
    return aStarSearch(startX, startY, endX, endY);
}

std::vector<Vec2> FindPathUtil::findPathInWorld(const Vec2& startWorldPos, const Vec2& endWorldPos) {
    // 世界坐标转网格坐标
    Vec2 startGridPos = GridMapUtils::pixelToGrid(startWorldPos);
    Vec2 endGridPos = GridMapUtils::pixelToGrid(endWorldPos);
    
    int startX = static_cast<int>(std::floor(startGridPos.x));
    int startY = static_cast<int>(std::floor(startGridPos.y));
    int endX = static_cast<int>(std::floor(endGridPos.x));
    int endY = static_cast<int>(std::floor(endGridPos.y));
    
    // 寻路
    std::vector<Vec2> gridPath = aStarSearch(startX, startY, endX, endY);
    
    if (gridPath.empty()) {
        return {};
    }
    
    // 移除起点
    if (gridPath.size() > 1) {
        gridPath.erase(gridPath.begin());
    }
    
    // 转换为世界坐标
    std::vector<Vec2> worldPath;
    worldPath.reserve(gridPath.size());
    
    for (const auto& gridPos : gridPath) {
        Vec2 worldPos = GridMapUtils::gridToPixelCenter(
            static_cast<int>(gridPos.x), 
            static_cast<int>(gridPos.y)
        );
        worldPath.push_back(worldPos);
    }
    
    return worldPath;
}
