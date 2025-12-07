#include "VillageDataManager.h"
#include "../Util/GridMapUtils.h"
#include "../Model/BuildingConfig.h"
#include <algorithm>
#include "cocos2d.h"

USING_NS_CC;

// 静态成员初始化
VillageDataManager* VillageDataManager::_instance = nullptr;

VillageDataManager::VillageDataManager()
    : _nextBuildingId(1) {
    
    // 初始化网格占用表（44x44，全部设为0表示空闲）
    _gridOccupancy.resize(GridMapUtils::GRID_WIDTH);
    for (auto& row : _gridOccupancy) {
        row.resize(GridMapUtils::GRID_HEIGHT, 0);
    }
    
    // 初始化默认资源
    _data.gold = 1000;
    _data.elixir = 1000;
    
    CCLOG("VillageDataManager: Initialized with grid occupancy table (44x44)");
}

VillageDataManager::~VillageDataManager() {
    _data.buildings.clear();
    _gridOccupancy.clear();
}

VillageDataManager* VillageDataManager::getInstance() {
    if (!_instance) {
        _instance = new VillageDataManager();
    }
    return _instance;
}

void VillageDataManager::destroyInstance() {
    if (_instance) {
        delete _instance;
        _instance = nullptr;
    }
}

// ========== 资源接口 ==========

int VillageDataManager::getGold() const {
    return _data.gold;
}

int VillageDataManager::getElixir() const {
    return _data.elixir;
}

void VillageDataManager::addGold(int amount) {
    _data.gold += amount;
    notifyResourceChanged();
}

void VillageDataManager::addElixir(int amount) {
    _data.elixir += amount;
    notifyResourceChanged();
}

bool VillageDataManager::spendGold(int amount) {
    if (_data.gold >= amount) {
        _data.gold -= amount;
        notifyResourceChanged();
        return true;
    }
    return false;
}

bool VillageDataManager::spendElixir(int amount) {
    if (_data.elixir >= amount) {
        _data.elixir -= amount;
        notifyResourceChanged();
        return true;
    }
    return false;
}

void VillageDataManager::notifyResourceChanged() {
    // TODO: 发送资源变化通知（用于更新UI）
}

// ========== 建筑接口 ==========

const std::vector<BuildingInstance>& VillageDataManager::getAllBuildings() const {
    return _data.buildings;
}

BuildingInstance* VillageDataManager::getBuildingById(int id) {
    for (auto& building : _data.buildings) {
        if (building.id == id) {
            return &building;
        }
    }
    return nullptr;
}

int VillageDataManager::addBuilding(int type, int level, int gridX, int gridY, 
                                    BuildingInstance::State state, long long finishTime) {
    BuildingInstance building;
    building.id = _nextBuildingId++;
    building.type = type;
    building.level = level;
    building.gridX = gridX;
    building.gridY = gridY;
    building.state = state;
    building.finishTime = finishTime;
    
    _data.buildings.push_back(building);
    
    // 更新网格占用表（只有非PLACING状态的建筑才占用）
    if (state != BuildingInstance::State::PLACING) {
        updateGridOccupancy();
    }
    
    CCLOG("VillageDataManager: Added building ID=%d, type=%d at grid(%d, %d)", 
          building.id, type, gridX, gridY);
    
    return building.id;
}

void VillageDataManager::upgradeBuilding(int id, int newLevel, long long finishTime) {
    auto* building = getBuildingById(id);
    if (building) {
        building->level = newLevel;
        building->state = BuildingInstance::State::CONSTRUCTING;
        building->finishTime = finishTime;
        
        CCLOG("VillageDataManager: Building ID=%d upgraded to level %d", id, newLevel);
    }
}

void VillageDataManager::setBuildingPosition(int id, int gridX, int gridY) {
    auto* building = getBuildingById(id);
    if (building) {
        building->gridX = gridX;
        building->gridY = gridY;
        
        // 更新网格占用表
        updateGridOccupancy();
        
        CCLOG("VillageDataManager: Building ID=%d moved to grid(%d, %d)", id, gridX, gridY);
    }
}

void VillageDataManager::setBuildingState(int id, BuildingInstance::State state, long long finishTime) {
    auto* building = getBuildingById(id);
    if (building) {
        building->state = state;
        building->finishTime = finishTime;
        
        // 更新网格占用表（状态改变可能影响占用）
        updateGridOccupancy();
        
        CCLOG("VillageDataManager: Building ID=%d state changed to %d", id, (int)state);
    }
}

// ========== 网格占用查询（性能优化：O(1) 查询）==========

bool VillageDataManager::isAreaOccupied(int startX, int startY, int width, int height, int ignoreBuildingId) const {
    // 边界检查
    if (startX < 0 || startY < 0 || 
        startX + width > GridMapUtils::GRID_WIDTH || 
        startY + height > GridMapUtils::GRID_HEIGHT) {
        return true;  // 越界视为被占用
    }
    
    // 检查区域内的每个格子
    for (int x = startX; x < startX + width; ++x) {
        for (int y = startY; y < startY + height; ++y) {
            int occupyingId = _gridOccupancy[x][y];
            
            // 如果被占用，且不是要忽略的建筑ID
            if (occupyingId != 0 && occupyingId != ignoreBuildingId) {
                return true;
            }
        }
    }
    
    return false;
}

void VillageDataManager::updateGridOccupancy() {
    // 清空占用表
    for (auto& row : _gridOccupancy) {
        std::fill(row.begin(), row.end(), 0);
    }
    
    // 重新标记所有非PLACING状态的建筑占用的格子
    for (const auto& building : _data.buildings) {
        // 跳过待放置状态的建筑
        if (building.state == BuildingInstance::State::PLACING) {
            continue;
        }
        
        // 获取建筑配置（需要知道尺寸）
        auto config = BuildingConfig::getInstance()->getConfig(building.type);
        if (!config) continue;
        
        // 标记占用区域
        for (int x = building.gridX; x < building.gridX + config->gridWidth; ++x) {
            for (int y = building.gridY; y < building.gridY + config->gridHeight; ++y) {
                // 边界检查
                if (x >= 0 && x < GridMapUtils::GRID_WIDTH && 
                    y >= 0 && y < GridMapUtils::GRID_HEIGHT) {
                    _gridOccupancy[x][y] = building.id;
                }
            }
        }
    }
    
    CCLOG("VillageDataManager: Grid occupancy table updated");
}

// ========== 存档/读档 ==========

void VillageDataManager::loadFromFile(const std::string& filename) {
    // TODO: 实现从文件加载
    CCLOG("VillageDataManager: Loading from %s (not implemented)", filename.c_str());
}

void VillageDataManager::saveToFile(const std::string& filename) {
    // TODO: 实现保存到文件
    CCLOG("VillageDataManager: Saving to %s (not implemented)", filename.c_str());
}