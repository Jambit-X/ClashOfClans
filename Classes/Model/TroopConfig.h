#pragma once
#ifndef __TROOP_CONFIG_H__
#define __TROOP_CONFIG_H__

#include "cocos2d.h"
#include <string>
#include <map>
#include <vector>

struct TroopInfo {
    int id;                 // ID (如 1001)
    std::string name;       // 名称
    std::string iconPath;   // 图标路径
    int unlockBarracksLvl;  // 解锁需要的训练营等级

    // --- 战斗属性 ---
    int housingSpace;       // 占用人口
    int moveSpeed;          // 移动速度
    int damagePerSecond;    // 秒伤
    int hitpoints;          // 生命值
    std::string damageType; // "单体" 或 "溅射"
    std::string target;     // "地面" 或 "空中"
    std::string favorite;   // 偏好 (如 "资源", "防御")
    int level;              // 等级 (目前固定为1)

    std::string description;
};

class TroopConfig {
public:
    static TroopConfig* getInstance();

    // 初始化数据
    void initConfig();

    // 获取单个兵种数据
    TroopInfo getTroopById(int id);

    // 获取所有兵种列表 (用于生成 UI)
    const std::vector<TroopInfo>& getAllTroops() const;

private:
    static TroopConfig* _instance;
    std::vector<TroopInfo> _troops;
    std::map<int, int> _idToIndex;
};

#endif // __TROOP_CONFIG_H__