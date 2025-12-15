#pragma once
#ifndef __TROOP_UPGRADE_CONFIG_H__
#define __TROOP_UPGRADE_CONFIG_H__

#include "cocos2d.h"
#include <string>
#include <map>
#include <vector>

// 单个等级的兵种数据
struct TroopLevelData {
    int level;              // 等级
    int hitpoints;          // 生命值
    int damagePerSecond;    // 每秒伤害
    int upgradeCost;        // 升到下一级的费用（圣水）
    int upgradeTime;        // 升级时间（秒）
    int requiredLabLevel;   // 需要的实验室等级
};

// 兵种升级配置（包含所有等级数据）
struct TroopUpgradeData {
    int troopId;
    std::string name;
    int maxLevel;
    std::vector<TroopLevelData> levels;  // levels[0] = 1级数据
};

/**
 * @brief 兵种升级配置管理器（单例）
 * 提供兵种各等级的属性和升级费用查询
 */
class TroopUpgradeConfig {
public:
    static TroopUpgradeConfig* getInstance();
    static void destroyInstance();

    // 获取兵种指定等级的数据
    const TroopLevelData* getLevelData(int troopId, int level) const;

    // 获取兵种升级费用（从 currentLevel 升到 currentLevel+1）
    int getUpgradeCost(int troopId, int currentLevel) const;

    // 获取兵种升级时间（从 currentLevel 升到 currentLevel+1）
    int getUpgradeTime(int troopId, int currentLevel) const;

    // 获取升级所需的实验室等级
    int getRequiredLabLevel(int troopId, int targetLevel) const;

    // 获取兵种最大等级
    int getMaxLevel(int troopId) const;

    // 检查兵种是否可以升级（基于实验室等级）
    bool canUpgradeWithLabLevel(int troopId, int currentTroopLevel, int labLevel) const;

    // 获取属性增量（用于UI显示 "原属性 + 增量"）
    int getHpIncrease(int troopId, int currentLevel) const;
    int getDpsIncrease(int troopId, int currentLevel) const;

private:
    TroopUpgradeConfig();
    ~TroopUpgradeConfig();

    void initConfigs();

    static TroopUpgradeConfig* _instance;
    std::map<int, TroopUpgradeData> _upgradeData;  // <troopId, upgradeData>
};

#endif // __TROOP_UPGRADE_CONFIG_H__
