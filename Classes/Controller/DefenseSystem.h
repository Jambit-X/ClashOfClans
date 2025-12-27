// DefenseSystem.h
// 建筑防御系统声明，管理防御建筑的自动锁定和攻击逻辑

#ifndef __DEFENSE_SYSTEM_H__
#define __DEFENSE_SYSTEM_H__

#include "cocos2d.h"
#include <vector>
#include <set>

class BattleUnitSprite;
class BattleTroopLayer;
struct BuildingInstance;

// 建筑防御系统类
// 职责：防御建筑自动锁定目标、攻击逻辑、播放攻击动画
class DefenseSystem {
public:
    static DefenseSystem* getInstance();
    static void destroyInstance();
    
    // 更新建筑防御（每帧调用）
    void updateBuildingDefense(BattleTroopLayer* troopLayer);
    
    // 查找攻击范围内最近的兵种
    BattleUnitSprite* findNearestUnitInRange(
        const BuildingInstance& building, 
        float attackRange, 
        BattleTroopLayer* troopLayer);
    
    // 获取攻击范围内所有兵种
    std::vector<BattleUnitSprite*> getAllUnitsInRange(
        const BuildingInstance& building, 
        float attackRange, 
        BattleTroopLayer* troopLayer);

private:
    DefenseSystem() = default;
    ~DefenseSystem() = default;
    
    static DefenseSystem* _instance;
};

#endif // __DEFENSE_SYSTEM_H__
