// RandomBattleMapGenerator.cpp
// 随机战斗地图生成器实现，根据难度生成合理的敌方村庄布局

#pragma execution_character_set("utf-8")

#include "RandomBattleMapGenerator.h"
#include "cocos2d.h"
#include "Model/BuildingConfig.h"
#include "Model/BuildingRequirements.h"
#include <algorithm>
#include <chrono>

USING_NS_CC;

// 静态成员初始化
int RandomBattleMapGenerator::nextBuildingId = 10000;

// C++11静态constexpr成员的类外定义（ODR-used要求）
constexpr int RandomBattleMapGenerator::TOWNHALL;
constexpr int RandomBattleMapGenerator::CANNON;
constexpr int RandomBattleMapGenerator::ARCHER_TOWER;
constexpr int RandomBattleMapGenerator::WALL;
constexpr int RandomBattleMapGenerator::GOLD_MINE;
constexpr int RandomBattleMapGenerator::ELIXIR_COLLECTOR;
constexpr int RandomBattleMapGenerator::GOLD_STORAGE;
constexpr int RandomBattleMapGenerator::ELIXIR_STORAGE;
constexpr int RandomBattleMapGenerator::BOMB;
constexpr int RandomBattleMapGenerator::GIANT_BOMB;
constexpr int RandomBattleMapGenerator::MAP_MIN;
constexpr int RandomBattleMapGenerator::MAP_MAX;
constexpr int RandomBattleMapGenerator::CENTER_X;
constexpr int RandomBattleMapGenerator::CENTER_Y;

// ===================================================================================
// 难度配置
// ===================================================================================

RandomBattleMapGenerator::DifficultyConfig 
RandomBattleMapGenerator::getDifficultyConfig(int difficulty) {
    DifficultyConfig config;
    
    switch (difficulty) {
        case 1: // 简单难度：低级建筑，少量防御
            config.townHallLevel = 1;
            config.minDefense = 1;
            config.maxDefense = 2;
            config.minResource = 2;
            config.maxResource = 3;
            config.minWalls = 10;
            config.maxWalls = 20;
            config.minTraps = 0;
            config.maxTraps = 1;
            config.goldReward = 500;
            config.elixirReward = 500;
            break;
            
        case 2: // 中等难度：2级大本营，适量防御和资源
            config.townHallLevel = 2;
            config.minDefense = 2;
            config.maxDefense = 4;
            config.minResource = 3;
            config.maxResource = 5;
            config.minWalls = 20;
            config.maxWalls = 35;
            config.minTraps = 1;
            config.maxTraps = 2;
            config.goldReward = 1000;
            config.elixirReward = 1000;
            break;
            
        case 3: // 困难难度：3级大本营，大量防御和陷阱
        default:
            config.townHallLevel = 3;
            config.minDefense = 4;
            config.maxDefense = 6;
            config.minResource = 4;
            config.maxResource = 6;
            config.minWalls = 40;
            config.maxWalls = 50;
            config.minTraps = 2;
            config.maxTraps = 4;
            config.goldReward = 2000;
            config.elixirReward = 2000;
            break;
    }
    
    return config;
}

// ===================================================================================
// 辅助函数
// ===================================================================================

void RandomBattleMapGenerator::getBuildingSize(int type, int& outW, int& outH) {
    auto config = BuildingConfig::getInstance()->getConfig(type);
    if (config) {
        outW = config->gridWidth;
        outH = config->gridHeight;
    } else {
        // 默认尺寸（配置缺失时的备用值）
        outW = 3;
        outH = 3;
    }
}

bool RandomBattleMapGenerator::isPositionValid(int x, int y, int w, int h,
                                                const std::vector<BuildingInstance>& existing) {
    // 检查是否超出地图边界
    if (x < MAP_MIN || y < MAP_MIN || x + w > MAP_MAX || y + h > MAP_MAX) {
        return false;
    }
    
    // 检查是否与已有建筑重叠
    for (const auto& building : existing) {
        int bw, bh;
        getBuildingSize(building.type, bw, bh);
        
        // AABB碰撞检测
        bool overlapX = (x < building.gridX + bw) && (x + w > building.gridX);
        bool overlapY = (y < building.gridY + bh) && (y + h > building.gridY);
        
        if (overlapX && overlapY) {
            return false;
        }
    }
    
    return true;
}

bool RandomBattleMapGenerator::findValidPosition(int gridW, int gridH,
                                                  int minX, int maxX, int minY, int maxY,
                                                  const std::vector<BuildingInstance>& existing,
                                                  int& outX, int& outY,
                                                  std::mt19937& rng) {
    // 在指定区域内随机尝试100次
    std::uniform_int_distribution<int> distX(minX, maxX - gridW);
    std::uniform_int_distribution<int> distY(minY, maxY - gridH);
    
    for (int attempt = 0; attempt < 100; ++attempt) {
        int x = distX(rng);
        int y = distY(rng);
        
        if (isPositionValid(x, y, gridW, gridH, existing)) {
            outX = x;
            outY = y;
            return true;
        }
    }
    
    return false;
}

// ===================================================================================
// 建筑放置：大本营
// ===================================================================================

void RandomBattleMapGenerator::placeTownHall(BattleMapData& map, int level) {
    BuildingInstance th;
    th.id = nextBuildingId++;
    th.type = TOWNHALL;
    th.level = level;
    th.gridX = CENTER_X - 2;  // 4x4建筑，居中放置
    th.gridY = CENTER_Y - 2;
    th.state = BuildingInstance::State::BUILT;
    th.finishTime = 0;
    th.isInitialConstruction = false;
    th.currentHP = 1500;
    th.isDestroyed = false;
    
    map.buildings.push_back(th);
}

// ===================================================================================
// 建筑放置：防御建筑
// ===================================================================================

void RandomBattleMapGenerator::placeDefenseBuildings(BattleMapData& map, int count, int townHallLevel, int innerRadius) {
    // 使用当前时间作为随机种子
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937 rng(static_cast<unsigned int>(seed));
    
    auto requirements = BuildingRequirements::getInstance();
    auto buildingConfig = BuildingConfig::getInstance();
    
    // 收集可用的防御建筑类型及其数量上限
    std::vector<std::pair<int, int>> availableDefenses;
    
    // 检查加农炮是否可用
    if (requirements->getMinTHLevel(CANNON) <= townHallLevel && buildingConfig->getConfig(CANNON)) {
        int maxCount = requirements->getMaxCount(CANNON, townHallLevel);
        if (maxCount > 0) {
            availableDefenses.push_back({CANNON, maxCount});
        }
    }
    
    // 检查箭塔是否可用
    if (requirements->getMinTHLevel(ARCHER_TOWER) <= townHallLevel && buildingConfig->getConfig(ARCHER_TOWER)) {
        int maxCount = requirements->getMaxCount(ARCHER_TOWER, townHallLevel);
        if (maxCount > 0) {
            availableDefenses.push_back({ARCHER_TOWER, maxCount});
        }
    }
    
    if (availableDefenses.empty()) {
        CCLOG("RandomBattleMapGenerator: No defense buildings available for TH level %d", townHallLevel);
        return;
    }
    
    // 跟踪每种建筑的已放置数量
    std::map<int, int> placedCount;
    for (const auto& def : availableDefenses) {
        placedCount[def.first] = 0;
    }
    
    // 定义内圈区域（城墙内部）
    int innerMinX = CENTER_X - innerRadius + 1;
    int innerMaxX = CENTER_X + innerRadius - 1;
    int innerMinY = CENTER_Y - innerRadius + 1;
    int innerMaxY = CENTER_Y + innerRadius - 1;
    
    // 定义外圈区域（城墙外围）
    int outerMinX = CENTER_X - 10;
    int outerMaxX = CENTER_X + 10;
    int outerMinY = CENTER_Y - 10;
    int outerMaxY = CENTER_Y + 10;
    
    int placed = 0;
    int attempts = 0;
    const int maxAttempts = count * 5;
    
    // 尝试放置防御建筑
    while (placed < count && attempts < maxAttempts) {
        attempts++;
        
        // 随机选择一种防御建筑类型
        std::uniform_int_distribution<int> typeDist(0, static_cast<int>(availableDefenses.size()) - 1);
        int idx = typeDist(rng);
        int type = availableDefenses[idx].first;
        int maxCount = availableDefenses[idx].second;
        
        // 检查该类型是否已达上限
        if (placedCount[type] >= maxCount) {
            continue;
        }
        
        int w, h;
        getBuildingSize(type, w, h);
        
        int x, y;
        bool found = false;
        
        // 80%概率优先尝试放在城墙内部
        std::bernoulli_distribution innerProb(0.8);
        bool tryInner = innerProb(rng);
        
        if (tryInner) {
             if (findValidPosition(w, h, innerMinX, innerMaxX, innerMinY, innerMaxY, map.buildings, x, y, rng)) {
                 found = true;
             }
        }
        
        // 如果内圈失败，尝试外圈
        if (!found) {
            if (findValidPosition(w, h, outerMinX, outerMaxX, outerMinY, outerMaxY, map.buildings, x, y, rng)) {
                found = true;
            }
        }
        
        if (found) {
            BuildingInstance building;
            building.id = nextBuildingId++;
            building.type = type;
            // 随机等级（1到大本营等级之间）
            std::uniform_int_distribution<int> levelDist(1, townHallLevel);
            building.level = levelDist(rng);
            building.gridX = x;
            building.gridY = y;
            building.state = BuildingInstance::State::BUILT;
            building.finishTime = 0;
            building.isInitialConstruction = false;
            
            auto config = BuildingConfig::getInstance()->getConfig(type);
            building.currentHP = config ? config->hitPoints : 500;
            building.isDestroyed = false;
            
            map.buildings.push_back(building);
            placedCount[type]++;
            placed++;
        }
    }
}

// ===================================================================================
// 建筑放置：资源建筑
// ===================================================================================

void RandomBattleMapGenerator::placeResourceBuildings(BattleMapData& map, int count, int townHallLevel) {
    // 使用不同的种子避免与防御建筑重复
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937 rng(static_cast<unsigned int>(seed + 1));
    
    auto requirements = BuildingRequirements::getInstance();
    auto buildingConfig = BuildingConfig::getInstance();
    
    std::map<int, int> maxCounts;
    std::map<int, int> placedCounts;
    
    // 资源建筑类型列表
    std::vector<int> resourceTypes = { GOLD_MINE, ELIXIR_COLLECTOR, GOLD_STORAGE, ELIXIR_STORAGE };
    
    // 检查每种资源建筑是否可用
    for (int type : resourceTypes) {
        if (requirements->getMinTHLevel(type) <= townHallLevel && buildingConfig->getConfig(type)) {
            maxCounts[type] = requirements->getMaxCount(type, townHallLevel);
            placedCounts[type] = 0;
        }
    }
    
    // 资源建筑放置在整个地图区域
    int minX = MAP_MIN;
    int maxX = MAP_MAX - 3;
    int minY = MAP_MIN;
    int maxY = MAP_MAX - 3;
    
    // 优先放置必需的存储建筑（金库和圣水瓶）
    std::vector<int> requiredTypes = { GOLD_STORAGE, ELIXIR_STORAGE };
    for (int type : requiredTypes) {
        if (maxCounts.find(type) == maxCounts.end() || maxCounts[type] <= 0) {
            continue;
        }
        
        int w, h;
        getBuildingSize(type, w, h);
        
        int x, y;
        if (findValidPosition(w, h, minX, maxX, minY, maxY, map.buildings, x, y, rng)) {
            BuildingInstance building;
            building.id = nextBuildingId++;
            building.type = type;
            std::uniform_int_distribution<int> levelDist(1, townHallLevel);
            building.level = levelDist(rng);
            building.gridX = x;
            building.gridY = y;
            building.state = BuildingInstance::State::BUILT;
            building.finishTime = 0;
            building.isInitialConstruction = false;
            
            auto config = BuildingConfig::getInstance()->getConfig(type);
            building.currentHP = config ? config->hitPoints : 400;
            building.isDestroyed = false;
            
            map.buildings.push_back(building);
            placedCounts[type]++;
        }
    }
    
    // 获取当前仍可放置的资源建筑类型
    auto getAvailableTypes = [&]() {
        std::vector<int> available;
        for (const auto& pair : maxCounts) {
            if (placedCounts[pair.first] < pair.second && buildingConfig->getConfig(pair.first)) {
                available.push_back(pair.first);
            }
        }
        return available;
    };
    
    // 已放置2个必需建筑
    int placed = 2;
    int attempts = 0;
    const int maxAttempts = count * 3;
    
    // 随机放置剩余的资源建筑
    while (placed < count && attempts < maxAttempts) {
        attempts++;
        
        auto availableTypes = getAvailableTypes();
        if (availableTypes.empty()) {
            break;
        }
        
        // 随机选择一种资源建筑类型
        std::uniform_int_distribution<int> typeDist(0, static_cast<int>(availableTypes.size()) - 1);
        int type = availableTypes[typeDist(rng)];
        
        int w, h;
        getBuildingSize(type, w, h);
        
        int x, y;
        if (findValidPosition(w, h, minX, maxX, minY, maxY, map.buildings, x, y, rng)) {
            BuildingInstance building;
            building.id = nextBuildingId++;
            building.type = type;
            std::uniform_int_distribution<int> levelDist(1, townHallLevel);
            building.level = levelDist(rng);
            building.gridX = x;
            building.gridY = y;
            building.state = BuildingInstance::State::BUILT;
            building.finishTime = 0;
            building.isInitialConstruction = false;
            
            auto config = BuildingConfig::getInstance()->getConfig(type);
            building.currentHP = config ? config->hitPoints : 400;
            building.isDestroyed = false;
            
            map.buildings.push_back(building);
            placedCounts[type]++;
            placed++;
        }
    }
}

// ===================================================================================
// 建筑放置：城墙
// ===================================================================================

int RandomBattleMapGenerator::placeWalls(BattleMapData& map, int count, int townHallLevel) {
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937 rng(static_cast<unsigned int>(seed + 2));
    
    auto requirements = BuildingRequirements::getInstance();
    
    // 检查城墙是否已解锁
    if (requirements->getMinTHLevel(WALL) > townHallLevel) {
        CCLOG("RandomBattleMapGenerator: Walls not unlocked for TH level %d", townHallLevel);
        return 0;
    }
    
    // 获取城墙数量上限
    int maxWalls = requirements->getMaxCount(WALL, townHallLevel);
    
    // 实际可用数量：取难度限制和游戏上限的较小值
    int availableWalls = (std::min)(count, maxWalls);
    
    // 计算围墙半径
    // 最小半径：3（7x7框架，中心5x5可放下4x4大本营）
    // 最大半径：根据周长估算 Perimeter ≈ 8*R，因此 R_max ≈ availableWalls/8
    int minRadius = 3;
    int maxRadius = availableWalls / 8;
    
    // 限制最大半径不超过地图边界
    int mapBoundRadius = (MAP_MAX - MAP_MIN) / 2 - 2;
    if (maxRadius > mapBoundRadius) maxRadius = mapBoundRadius;
    
    if (maxRadius < minRadius) maxRadius = minRadius;
    
    // 随机选择一个半径，增加布局多样性
    std::uniform_int_distribution<int> radiusDist(minRadius, maxRadius);
    int actualRadius = radiusDist(rng);
    
    // 计算该半径需要的城墙数量（矩形周长：8*R）
    int wallsNeeded = 8 * actualRadius;
    
    // 如果城墙不够，回退到较小半径
    if (wallsNeeded > availableWalls) {
        actualRadius = availableWalls / 8;
        if (actualRadius < minRadius) actualRadius = minRadius;
    }
    
    CCLOG("RandomBattleMapGenerator: Wall Planning - Available: %d, Radius: %d (Range %d-%d)", 
          availableWalls, actualRadius, minRadius, maxRadius);

    // 随机城墙等级
    std::uniform_int_distribution<int> levelDist(1, townHallLevel);
    
    // 计算围墙的边界坐标
    int left = CENTER_X - actualRadius;
    int right = CENTER_X + actualRadius;
    int bottom = CENTER_Y - actualRadius;
    int top = CENTER_Y + actualRadius;
    
    int placed = 0;
    
    // 放置单块城墙的Lambda函数
    auto placeWallBlock = [&](int x, int y) {
        if (placed >= availableWalls) return;
        if (isPositionValid(x, y, 1, 1, map.buildings)) {
            BuildingInstance wall;
            wall.id = nextBuildingId++;
            wall.type = WALL;
            wall.level = levelDist(rng);
            wall.gridX = x;
            wall.gridY = y;
            wall.state = BuildingInstance::State::BUILT;
            wall.finishTime = 0;
            wall.isInitialConstruction = false;
            wall.currentHP = 300;
            wall.isDestroyed = false;
            
            map.buildings.push_back(wall);
            placed++;
        }
    };
    
    // 生成闭合矩形围墙
    // 顺序：上边 → 右边 → 下边 → 左边
    for (int x = left; x <= right; ++x) placeWallBlock(x, top);
    for (int y = top - 1; y > bottom; --y) placeWallBlock(right, y);
    for (int x = right; x >= left; --x) placeWallBlock(x, bottom);
    for (int y = bottom + 1; y < top; ++y) placeWallBlock(left, y);
    
    CCLOG("RandomBattleMapGenerator: Placed %d walls forming radius %d ring", placed, actualRadius);
    
    // 如果有大量剩余城墙，可以考虑放置外围城墙（当前未实现）
    int remaining = availableWalls - placed;
    if (remaining > 10) {
        // 预留扩展：可在外围随机放置额外城墙增加难度
    }
    
    return actualRadius;
}

// ===================================================================================
// 建筑放置：陷阱
// ===================================================================================

void RandomBattleMapGenerator::placeTraps(BattleMapData& map, int count, int townHallLevel) {
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937 rng(static_cast<unsigned int>(seed + 3));
    
    auto requirements = BuildingRequirements::getInstance();
    auto buildingConfig = BuildingConfig::getInstance();
    
    // 收集可用的陷阱类型及其数量上限
    std::vector<std::pair<int, int>> availableTraps;
    
    // 检查炸弹是否可用
    if (requirements->getMinTHLevel(BOMB) <= townHallLevel && buildingConfig->getConfig(BOMB)) {
        int maxCount = requirements->getMaxCount(BOMB, townHallLevel);
        if (maxCount > 0) availableTraps.push_back({BOMB, maxCount});
    }
    
    // 检查巨型炸弹是否可用
    if (requirements->getMinTHLevel(GIANT_BOMB) <= townHallLevel && buildingConfig->getConfig(GIANT_BOMB)) {
        int maxCount = requirements->getMaxCount(GIANT_BOMB, townHallLevel);
        if (maxCount > 0) availableTraps.push_back({GIANT_BOMB, maxCount});
    }
    
    if (availableTraps.empty()) {
        CCLOG("RandomBattleMapGenerator: No traps available for TH level %d", townHallLevel);
        return;
    }
    
    // 跟踪每种陷阱的已放置数量
    std::map<int, int> placedCount;
    for (const auto& trap : availableTraps) {
        placedCount[trap.first] = 0;
    }
    
    // 陷阱放置在城墙附近区域
    int minX = CENTER_X - 6;
    int maxX = CENTER_X + 6;
    int minY = CENTER_Y - 6;
    int maxY = CENTER_Y + 6;
    
    int placed = 0;
    int attempts = 0;
    const int maxAttempts = count * 3;
    
    while (placed < count && attempts < maxAttempts) {
        attempts++;
        
        // 随机选择一种陷阱类型
        std::uniform_int_distribution<int> typeDist(0, static_cast<int>(availableTraps.size()) - 1);
        int idx = typeDist(rng);
        int type = availableTraps[idx].first;
        int maxCount = availableTraps[idx].second;
        
        // 检查该类型是否已达数量上限
        if (placedCount[type] >= maxCount) {
            continue;
        }
        
        int w, h;
        getBuildingSize(type, w, h);
        
        int x, y;
        if (findValidPosition(w, h, minX, maxX, minY, maxY, map.buildings, x, y, rng)) {
            BuildingInstance trap;
            trap.id = nextBuildingId++;
            trap.type = type;
            std::uniform_int_distribution<int> levelDist(1, townHallLevel);
            trap.level = levelDist(rng);
            trap.gridX = x;
            trap.gridY = y;
            trap.state = BuildingInstance::State::BUILT;
            trap.finishTime = 0;
            trap.isInitialConstruction = false;
            trap.currentHP = 1;
            trap.isDestroyed = false;
            
            map.buildings.push_back(trap);
            placedCount[type]++;
            placed++;
        }
    }
    
    CCLOG("RandomBattleMapGenerator: Placed %d traps", placed);
}

// ===================================================================================
// 主生成函数
// ===================================================================================

BattleMapData RandomBattleMapGenerator::generate(int difficulty) {
    // 重置建筑ID计数器
    nextBuildingId = 10000;
    
    // 如果难度无效，随机选择1-3
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937 rng(static_cast<unsigned int>(seed));
    
    if (difficulty <= 0 || difficulty > 3) {
        std::uniform_int_distribution<int> diffDist(1, 3);
        difficulty = diffDist(rng);
    }
    
    CCLOG("RandomBattleMapGenerator: Generating map with difficulty %d", difficulty);
    
    auto config = getDifficultyConfig(difficulty);
    
    BattleMapData map;
    map.difficulty = difficulty;
    map.goldReward = config.goldReward;
    map.elixirReward = config.elixirReward;
    
    // 生成步骤（顺序很重要）
    
    // 步骤1：放置大本营（中心位置）
    placeTownHall(map, config.townHallLevel);
    
    // 步骤2：生成城墙（优先放置，返回围墙半径供防御建筑使用）
    std::uniform_int_distribution<int> wallDist(config.minWalls, config.maxWalls);
    int wallLimit = wallDist(rng);
    int wallRadius = placeWalls(map, wallLimit, config.townHallLevel);
    
    // 步骤3：放置防御建筑（优先放在城墙内部）
    std::uniform_int_distribution<int> defDist(config.minDefense, config.maxDefense);
    int defenseCount = defDist(rng);
    placeDefenseBuildings(map, defenseCount, config.townHallLevel, wallRadius);
    
    // 步骤4：放置资源建筑（包含必需的金库和圣水瓶）
    std::uniform_int_distribution<int> resDist(config.minResource, config.maxResource);
    int resourceCount = resDist(rng);
    placeResourceBuildings(map, resourceCount + 2, config.townHallLevel);
    
    // 步骤5：放置陷阱
    std::uniform_int_distribution<int> trapDist(config.minTraps, config.maxTraps);
    int trapCount = trapDist(rng);
    placeTraps(map, trapCount, config.townHallLevel);
    
    // 步骤6：计算可掠夺资源
    calculateLootableResources(map);
    
    CCLOG("RandomBattleMapGenerator: Generated map with %zu buildings, lootable gold=%d, lootable elixir=%d", 
          map.buildings.size(), map.lootableGold, map.lootableElixir);
    
    return map;
}

// ===================================================================================
// 资源计算
// ===================================================================================

void RandomBattleMapGenerator::calculateLootableResources(BattleMapData& map) {
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937 rng(static_cast<unsigned int>(seed + 100));
    
    int goldStorageCount = 0;
    int elixirStorageCount = 0;
    int totalGoldCapacity = 0;
    int totalElixirCapacity = 0;
    
    auto buildingConfig = BuildingConfig::getInstance();
    
    // 统计所有存储建筑的数量和总容量
    for (const auto& building : map.buildings) {
        if (building.type == GOLD_STORAGE) {
            goldStorageCount++;
            int capacity = buildingConfig->getStorageCapacityByLevel(building.type, building.level);
            totalGoldCapacity += capacity;
        }
        else if (building.type == ELIXIR_STORAGE) {
            elixirStorageCount++;
            int capacity = buildingConfig->getStorageCapacityByLevel(building.type, building.level);
            totalElixirCapacity += capacity;
        }
    }
    
    // 记录存储建筑数量（用于战斗结算）
    map.goldStorageCount = goldStorageCount;
    map.elixirStorageCount = elixirStorageCount;
    
    // 随机生成可掠夺资源（总容量的25%-100%之间）
    if (totalGoldCapacity > 0) {
        std::uniform_int_distribution<int> goldDist(totalGoldCapacity / 4, totalGoldCapacity);
        map.lootableGold = goldDist(rng);
    }
    
    if (totalElixirCapacity > 0) {
        std::uniform_int_distribution<int> elixirDist(totalElixirCapacity / 4, totalElixirCapacity);
        map.lootableElixir = elixirDist(rng);
    }
    
    CCLOG("RandomBattleMapGenerator: Lootable resources - Gold: %d/%d (%d storages), Elixir: %d/%d (%d storages)",
          map.lootableGold, totalGoldCapacity, goldStorageCount,
          map.lootableElixir, totalElixirCapacity, elixirStorageCount);
}
