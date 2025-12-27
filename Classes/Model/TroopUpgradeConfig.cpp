// TroopUpgradeConfig.cpp
// 兵种升级配置管理器实现，初始化各兵种的等级数据

#pragma execution_character_set("utf-8")
#include "TroopUpgradeConfig.h"

TroopUpgradeConfig* TroopUpgradeConfig::_instance = nullptr;

TroopUpgradeConfig* TroopUpgradeConfig::getInstance() {
    if (!_instance) {
        _instance = new TroopUpgradeConfig();
    }
    return _instance;
}

void TroopUpgradeConfig::destroyInstance() {
    if (_instance) {
        delete _instance;
        _instance = nullptr;
    }
}

TroopUpgradeConfig::TroopUpgradeConfig() {
    initConfigs();
}

TroopUpgradeConfig::~TroopUpgradeConfig() {
    _upgradeData.clear();
}

void TroopUpgradeConfig::initConfigs() {
    _upgradeData.clear();

    // 野蛮人（基础: HP=45, DPS=8）
    {
        TroopUpgradeData data;
        data.troopId = 1001;
        data.name = "野蛮人";
        data.maxLevel = 3;
        data.levels = {
            {1, 45, 8, 10000, 30, 1},   // 1级: HP=45, DPS=8, 升2级费用1万, 30秒, 需1级实验室
            {2, 59, 10, 20000, 60, 2},  // 2级: HP=59(+30%), DPS=10, 升3级费用2万, 60秒, 需2级实验室
            {3, 72, 13, 0, 0, 3}        // 3级: HP=72(+60%), DPS=13, 满级
        };
        _upgradeData[1001] = data;
    }

    // 弓箭手（基础: HP=20, DPS=7）
    {
        TroopUpgradeData data;
        data.troopId = 1002;
        data.name = "弓箭手";
        data.maxLevel = 3;
        data.levels = {
            {1, 20, 7, 15000, 30, 1},
            {2, 26, 9, 30000, 60, 2},
            {3, 32, 11, 0, 0, 3}
        };
        _upgradeData[1002] = data;
    }

    // 哥布林（基础: HP=25, DPS=11）
    {
        TroopUpgradeData data;
        data.troopId = 1003;
        data.name = "哥布林";
        data.maxLevel = 3;
        data.levels = {
            {1, 25, 11, 20000, 30, 1},
            {2, 33, 14, 40000, 60, 2},
            {3, 40, 18, 0, 0, 3}
        };
        _upgradeData[1003] = data;
    }

    // 巨人（基础: HP=300, DPS=11）
    {
        TroopUpgradeData data;
        data.troopId = 1004;
        data.name = "巨人";
        data.maxLevel = 3;
        data.levels = {
            {1, 300, 11, 25000, 30, 1},
            {2, 390, 14, 50000, 60, 2},
            {3, 480, 18, 0, 0, 3}
        };
        _upgradeData[1004] = data;
    }

    // 炸弹人（基础: HP=20, DPS=12）
    {
        TroopUpgradeData data;
        data.troopId = 1005;
        data.name = "炸弹人";
        data.maxLevel = 3;
        data.levels = {
            {1, 20, 12, 30000, 30, 1},
            {2, 26, 16, 60000, 60, 2},
            {3, 32, 19, 0, 0, 3}
        };
        _upgradeData[1005] = data;
    }

    // 气球兵（基础: HP=150, DPS=25）
    {
        TroopUpgradeData data;
        data.troopId = 1006;
        data.name = "气球兵";
        data.maxLevel = 3;
        data.levels = {
            {1, 150, 25, 35000, 30, 1},
            {2, 195, 33, 70000, 60, 2},
            {3, 240, 40, 0, 0, 3}
        };
        _upgradeData[1006] = data;
    }

    CCLOG("TroopUpgradeConfig: Initialized %lu troop upgrade configs", _upgradeData.size());
}

const TroopLevelData* TroopUpgradeConfig::getLevelData(int troopId, int level) const {
    auto it = _upgradeData.find(troopId);
    if (it == _upgradeData.end()) {
        return nullptr;
    }

    const auto& levels = it->second.levels;
    if (level < 1 || level > (int)levels.size()) {
        return nullptr;
    }

    return &levels[level - 1];  // levels是0-indexed
}

int TroopUpgradeConfig::getUpgradeCost(int troopId, int currentLevel) const {
    const TroopLevelData* data = getLevelData(troopId, currentLevel);
    if (!data) {
        return 0;
    }
    return data->upgradeCost;
}

int TroopUpgradeConfig::getUpgradeTime(int troopId, int currentLevel) const {
    const TroopLevelData* data = getLevelData(troopId, currentLevel);
    if (!data) {
        return 0;
    }
    return data->upgradeTime;
}

int TroopUpgradeConfig::getRequiredLabLevel(int troopId, int targetLevel) const {
    // 要升到targetLevel，需要查询(targetLevel-1)级的数据
    int currentLevel = targetLevel - 1;
    const TroopLevelData* data = getLevelData(troopId, currentLevel);
    if (!data) {
        return 999;
    }
    return data->requiredLabLevel;
}

int TroopUpgradeConfig::getMaxLevel(int troopId) const {
    auto it = _upgradeData.find(troopId);
    if (it == _upgradeData.end()) {
        return 1;
    }
    return it->second.maxLevel;
}

bool TroopUpgradeConfig::canUpgradeWithLabLevel(int troopId, int currentTroopLevel, int labLevel) const {
    int targetLevel = currentTroopLevel + 1;
    int requiredLab = getRequiredLabLevel(troopId, targetLevel);
    return labLevel >= requiredLab;
}

int TroopUpgradeConfig::getHpIncrease(int troopId, int currentLevel) const {
    const TroopLevelData* current = getLevelData(troopId, currentLevel);
    const TroopLevelData* next = getLevelData(troopId, currentLevel + 1);

    if (!current || !next) {
        return 0;
    }

    return next->hitpoints - current->hitpoints;
}

int TroopUpgradeConfig::getDpsIncrease(int troopId, int currentLevel) const {
    const TroopLevelData* current = getLevelData(troopId, currentLevel);
    const TroopLevelData* next = getLevelData(troopId, currentLevel + 1);

    if (!current || !next) {
        return 0;
    }

    return next->damagePerSecond - current->damagePerSecond;
}
