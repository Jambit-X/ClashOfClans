// 【必须加在第一行】强制使用 UTF-8 编码
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

// C++11 需要静态 constexpr 成员的类外定义（ODR-used 时）
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

RandomBattleMapGenerator::DifficultyConfig 
RandomBattleMapGenerator::getDifficultyConfig(int difficulty) {
    DifficultyConfig config;
    
    switch (difficulty) {
        case 1: // 简单
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
            
        case 2: // 中等
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
            
        case 3: // 困难
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

void RandomBattleMapGenerator::getBuildingSize(int type, int& outW, int& outH) {
    auto config = BuildingConfig::getInstance()->getConfig(type);
    if (config) {
        outW = config->gridWidth;
        outH = config->gridHeight;
    } else {
        // 默认尺寸
        outW = 3;
        outH = 3;
    }
}

bool RandomBattleMapGenerator::isPositionValid(int x, int y, int w, int h,
                                                const std::vector<BuildingInstance>& existing) {
    // 边界检查
    if (x < MAP_MIN || y < MAP_MIN || x + w > MAP_MAX || y + h > MAP_MAX) {
        return false;
    }
    
    // 碰撞检测
    for (const auto& building : existing) {
        int bw, bh;
        getBuildingSize(building.type, bw, bh);
        
        // 检查矩形是否重叠
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
    // 尝试100次随机位置
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
    th.currentHP = 1500;  // 默认满血
    th.isDestroyed = false;
    
    map.buildings.push_back(th);
}

void RandomBattleMapGenerator::placeDefenseBuildings(BattleMapData& map, int count, int townHallLevel, int innerRadius) {
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937 rng(static_cast<unsigned int>(seed));
    
    auto requirements = BuildingRequirements::getInstance();
    auto buildingConfig = BuildingConfig::getInstance();
    
    std::vector<std::pair<int, int>> availableDefenses;
    
    if (requirements->getMinTHLevel(CANNON) <= townHallLevel && buildingConfig->getConfig(CANNON)) {
        int maxCount = requirements->getMaxCount(CANNON, townHallLevel);
        if (maxCount > 0) {
            availableDefenses.push_back({CANNON, maxCount});
        }
    }
    
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
    
    std::map<int, int> placedCount;
    for (const auto& def : availableDefenses) {
        placedCount[def.first] = 0;
    }
    
    int innerMinX = CENTER_X - innerRadius + 1;
    int innerMaxX = CENTER_X + innerRadius - 1;
    int innerMinY = CENTER_Y - innerRadius + 1;
    int innerMaxY = CENTER_Y + innerRadius - 1;
    
    int outerMinX = CENTER_X - 10;
    int outerMaxX = CENTER_X + 10;
    int outerMinY = CENTER_Y - 10;
    int outerMaxY = CENTER_Y + 10;
    
    int placed = 0;
    int attempts = 0;
    const int maxAttempts = count * 5;
    
    while (placed < count && attempts < maxAttempts) {
        attempts++;
        
        std::uniform_int_distribution<int> typeDist(0, static_cast<int>(availableDefenses.size()) - 1);
        int idx = typeDist(rng);
        int type = availableDefenses[idx].first;
        int maxCount = availableDefenses[idx].second;
        
        if (placedCount[type] >= maxCount) {
            continue;
        }
        
        int w, h;
        getBuildingSize(type, w, h);
        
        int x, y;
        bool found = false;
        
        std::bernoulli_distribution innerProb(0.8);
        bool tryInner = innerProb(rng);
        
        if (tryInner) {
             if (findValidPosition(w, h, innerMinX, innerMaxX, innerMinY, innerMaxY, map.buildings, x, y, rng)) {
                 found = true;
             }
        }
        
        if (!found) {
            if (findValidPosition(w, h, outerMinX, outerMaxX, outerMinY, outerMaxY, map.buildings, x, y, rng)) {
                found = true;
            }
        }
        
        if (found) {
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
            building.currentHP = config ? config->hitPoints : 500;
            building.isDestroyed = false;
            
            map.buildings.push_back(building);
            placedCount[type]++;
            placed++;
        }
    }
}

void RandomBattleMapGenerator::placeResourceBuildings(BattleMapData& map, int count, int townHallLevel) {
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937 rng(static_cast<unsigned int>(seed + 1));
    
    auto requirements = BuildingRequirements::getInstance();
    auto buildingConfig = BuildingConfig::getInstance();
    
    std::map<int, int> maxCounts;
    std::map<int, int> placedCounts;
    
    std::vector<int> resourceTypes = { GOLD_MINE, ELIXIR_COLLECTOR, GOLD_STORAGE, ELIXIR_STORAGE };
    
    for (int type : resourceTypes) {
        if (requirements->getMinTHLevel(type) <= townHallLevel && buildingConfig->getConfig(type)) {
            maxCounts[type] = requirements->getMaxCount(type, townHallLevel);
            placedCounts[type] = 0;
        }
    }
    
    int minX = MAP_MIN;
    int maxX = MAP_MAX - 3;
    int minY = MAP_MIN;
    int maxY = MAP_MAX - 3;
    
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
    
    auto getAvailableTypes = [&]() {
        std::vector<int> available;
        for (const auto& pair : maxCounts) {
            if (placedCounts[pair.first] < pair.second && buildingConfig->getConfig(pair.first)) {
                available.push_back(pair.first);
            }
        }
        return available;
    };
    
    int placed = 2;
    int attempts = 0;
    const int maxAttempts = count * 3;
    
    while (placed < count && attempts < maxAttempts) {
        attempts++;
        
        auto availableTypes = getAvailableTypes();
        if (availableTypes.empty()) {
            break;
        }
        
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

int RandomBattleMapGenerator::placeWalls(BattleMapData& map, int count, int townHallLevel) {
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937 rng(static_cast<unsigned int>(seed + 2));
    
    auto requirements = BuildingRequirements::getInstance();
    
    // 检查城墙是否解锁
    if (requirements->getMinTHLevel(WALL) > townHallLevel) {
        CCLOG("RandomBattleMapGenerator: Walls not unlocked for TH level %d", townHallLevel);
        return 0;
    }
    
    // 获取城墙数量上限（真实数据）
    int maxWalls = requirements->getMaxCount(WALL, townHallLevel);
    
    // 实际可用数量：不能超过count（难度限制）也不能超过游戏上限
    int availableWalls = (std::min)(count, maxWalls);
    
    // 计算围墙半径逻辑
    // 最小半径：3 (7x7的框，中心5x5，刚好放下大本营4x4和一点空隙)
    // 最大半径：根据周长估算，Perimeter ≈ 8 * R
    // R_max = floor((N - 4) / 8) -> 甚至可以简单估算 N / 8
    
    int minRadius = 3;
    int maxRadius = availableWalls / 8;
    
    // 限制最大半径不超过地图边界
    int mapBoundRadius = (MAP_MAX - MAP_MIN) / 2 - 2;
    if (maxRadius > mapBoundRadius) maxRadius = mapBoundRadius;
    
    if (maxRadius < minRadius) maxRadius = minRadius;
    
    // 随机选择一个半径 (核心逻辑：不一定选最大的)
    // 使用 geometric distribution 或简单 uniform，这里用 uniform 增加多样性
    std::uniform_int_distribution<int> radiusDist(minRadius, maxRadius);
    int actualRadius = radiusDist(rng);
    
    // 计算这个半径需要的城墙数量
    // 矩形周长：(2*R + 1) * 4 - 4 = 8*R
    int wallsNeeded = 8 * actualRadius;
    
    // 如果实际墙不够（理论上上面计算过maxRadius应该够，但防止意外），回退
    if (wallsNeeded > availableWalls) {
        actualRadius = availableWalls / 8;
        if (actualRadius < minRadius) actualRadius = minRadius; // 强制最小
    }
    
    CCLOG("RandomBattleMapGenerator: Wall Planning - Available: %d, Radius: %d (Range %d-%d)", 
          availableWalls, actualRadius, minRadius, maxRadius);

    // 建筑等级
    std::uniform_int_distribution<int> levelDist(1, townHallLevel);
    
    // 构建围墙 (中心点 CENTER_X, CENTER_Y)
    int left = CENTER_X - actualRadius;
    int right = CENTER_X + actualRadius;
    int bottom = CENTER_Y - actualRadius;
    int top = CENTER_Y + actualRadius;
    
    int placed = 0;
    
    // Lambda: 放置单块墙
    auto placeWallBlock = [&](int x, int y) {
        if (placed >= availableWalls) return; // 虽然按计算应该够，但做个保护
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
    
    // 生成闭合矩形
    // 上边 (Left -> Right)
    for (int x = left; x <= right; ++x) placeWallBlock(x, top);
    // 右边 (Top-1 -> Bottom+1)
    for (int y = top - 1; y > bottom; --y) placeWallBlock(right, y);
    // 下边 (Right -> Left)
    for (int x = right; x >= left; --x) placeWallBlock(x, bottom);
    // 左边 (Bottom+1 -> Top-1)
    for (int y = bottom + 1; y < top; ++y) placeWallBlock(left, y);
    
    CCLOG("RandomBattleMapGenerator: Placed %d walls forming radius %d ring", placed, actualRadius);
    
    // 如果还有剩余很多的墙，且足够围第二圈（间隔1格，半径+2）
    // 或者我们直接把剩余的墙作为“外围干扰”随机放
    int remaining = availableWalls - placed;
    if (remaining > 10) { // 随便定的阈值
        int outerRadius = actualRadius + 2;
        // 尝试在外围放一些
        // ... 这里简单处理，为了增加随机性，不在外围围死，而是随机放几断
        // 简单起见，这里就不处理了，或者只放几个
    }
    
    return actualRadius;
}

void RandomBattleMapGenerator::placeTraps(BattleMapData& map, int count, int townHallLevel) {
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937 rng(static_cast<unsigned int>(seed + 3));
    
    auto requirements = BuildingRequirements::getInstance();
    auto buildingConfig = BuildingConfig::getInstance();
    
    // 根据大本营等级获取可用的陷阱类型及其最大数量（只包含有图片资源的）
    std::vector<std::pair<int, int>> availableTraps; // <type, maxCount>
    
    // 炸弹 401
    if (requirements->getMinTHLevel(BOMB) <= townHallLevel && buildingConfig->getConfig(BOMB)) {
        int maxCount = requirements->getMaxCount(BOMB, townHallLevel);
        if (maxCount > 0) availableTraps.push_back({BOMB, maxCount});
    }
    // 巨型炸弹 404
    if (requirements->getMinTHLevel(GIANT_BOMB) <= townHallLevel && buildingConfig->getConfig(GIANT_BOMB)) {
        int maxCount = requirements->getMaxCount(GIANT_BOMB, townHallLevel);
        if (maxCount > 0) availableTraps.push_back({GIANT_BOMB, maxCount});
    }
    
    if (availableTraps.empty()) {
        CCLOG("RandomBattleMapGenerator: No traps available for TH level %d", townHallLevel);
        return;
    }
    
    // 跟踪每种陷阱已放置数量
    std::map<int, int> placedCount;
    for (const auto& trap : availableTraps) {
        placedCount[trap.first] = 0;
    }
    
    // 陷阱放在城墙附近
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
        
        // 检查数量限制
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

BattleMapData RandomBattleMapGenerator::generate(int difficulty) {
    // 重置建筑ID
    nextBuildingId = 10000;
    
    // 如果difficulty为0，随机选择1-3
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
    
    // 1. 放置大本营（必须）
    placeTownHall(map, config.townHallLevel);
    
    // 2. 放置城墙（优先放置，确保形成闭环，并获取围墙半径）
    // 使用配置中的 min/max walls 或者直接使用 真实上限 (这里placeWalls内部会处理)
    // 为了增加随机性，我们传入一个 基于难度的 "建议" 数量
    std::uniform_int_distribution<int> wallDist(config.minWalls, config.maxWalls);
    int wallLimit = wallDist(rng);
    // 注意：我们将 placeWalls 改为了返回 围墙半径
    int wallRadius = placeWalls(map, wallLimit, config.townHallLevel);
    
    // 3. 放置防御建筑（优先尝试放在 wallRadius 内部）
    std::uniform_int_distribution<int> defDist(config.minDefense, config.maxDefense);
    int defenseCount = defDist(rng);
    placeDefenseBuildings(map, defenseCount, config.townHallLevel, wallRadius);
    
    // 4. 放置资源建筑（包含必须的储金罐和圣水瓶）
    std::uniform_int_distribution<int> resDist(config.minResource, config.maxResource);
    int resourceCount = resDist(rng);
    placeResourceBuildings(map, resourceCount + 2, config.townHallLevel);  // +2 因为必须的
    
    // 5. 放置陷阱
    std::uniform_int_distribution<int> trapDist(config.minTraps, config.maxTraps);
    int trapCount = trapDist(rng);
    placeTraps(map, trapCount, config.townHallLevel);
    
    // 6. 计算可掠夺资源
    calculateLootableResources(map);
    
    CCLOG("RandomBattleMapGenerator: Generated map with %zu buildings, lootable gold=%d, lootable elixir=%d", 
          map.buildings.size(), map.lootableGold, map.lootableElixir);
    
    return map;
}

void RandomBattleMapGenerator::calculateLootableResources(BattleMapData& map) {
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937 rng(static_cast<unsigned int>(seed + 100));
    
    int goldStorageCount = 0;
    int elixirStorageCount = 0;
    int totalGoldCapacity = 0;
    int totalElixirCapacity = 0;
    
    auto buildingConfig = BuildingConfig::getInstance();
    
    // 统计储存建筑数量和总容量
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
    
    // 记录储存建筑数量
    map.goldStorageCount = goldStorageCount;
    map.elixirStorageCount = elixirStorageCount;
    
    // 随机生成可掠夺资源（0 到 总容量之间）
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
