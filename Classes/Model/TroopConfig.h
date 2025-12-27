// TroopConfig.h
// 兵种配置管理器，提供所有兵种的静态数据

#pragma once
#ifndef __TROOP_CONFIG_H__
#define __TROOP_CONFIG_H__

#include "cocos2d.h"
#include <string>
#include <map>
#include <vector>

// 兵种信息数据
struct TroopInfo {
    int id;                 // 兵种ID
    std::string name;       // 兵种名称
    std::string iconPath;   // 图标路径
    int unlockBarracksLvl;  // 解锁需要的训练营等级

    // 战斗属性
    int housingSpace;       // 占用人口
    int moveSpeed;          // 移动速度
    int damagePerSecond;    // 秒伤
    int hitpoints;          // 生命值
    std::string damageType; // 伤害类型（单体/溅射）
    std::string target;     // 攻击目标（地面/空中）
    std::string favorite;   // 攻击偏好（资源/防御）
    int level;              // 等级

    std::string description;
    float splashRadius = 0.0f;  // 溅射伤害半径（格子单位），0表示单体攻击
};

// 兵种配置管理器（单例）
class TroopConfig {
public:
    static TroopConfig* getInstance();

    // 初始化兵种数据
    void initConfig();

    // 根据ID获取兵种数据
    TroopInfo getTroopById(int id);

    // 获取所有兵种列表
    const std::vector<TroopInfo>& getAllTroops() const;

private:
    static TroopConfig* _instance;
    std::vector<TroopInfo> _troops;
    std::map<int, int> _idToIndex;
};

#endif // __TROOP_CONFIG_H__
