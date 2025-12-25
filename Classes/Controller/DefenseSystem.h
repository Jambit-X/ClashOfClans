#ifndef __DEFENSE_SYSTEM_H__
#define __DEFENSE_SYSTEM_H__

#include "cocos2d.h"
#include <vector>
#include <set>

class BattleUnitSprite;
class BattleTroopLayer;
struct BuildingInstance;

/**
 * DefenseSystem - 建筑防御系统
 * 
 * 职责：
 * 1. 防御建筑自动锁定目标
 * 2. 防御建筑攻击逻辑
 * 3. 播放攻击动画
 */
class DefenseSystem {
public:
    static DefenseSystem* getInstance();
    static void destroyInstance();
    
    /**
     * @brief 更新建筑防御（每帧调用）
     * @param troopLayer 兵种层
     */
    void updateBuildingDefense(BattleTroopLayer* troopLayer);
    
    /**
     * @brief 查找攻击范围内最近的兵种
     */
    BattleUnitSprite* findNearestUnitInRange(
        const BuildingInstance& building, 
        float attackRange, 
        BattleTroopLayer* troopLayer);
    
    /**
     * @brief 获取攻击范围内所有兵种
     */
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
