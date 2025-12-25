#ifndef __TRAP_SYSTEM_H__
#define __TRAP_SYSTEM_H__

#include "cocos2d.h"
#include <set>
#include <map>

class BattleUnitSprite;
class BattleTroopLayer;
struct BuildingInstance;

/**
 * TrapSystem - 陷阱系统
 * 
 * 职责：
 * 1. 检测兵种是否踩到陷阱
 * 2. 管理陷阱触发延迟
 * 3. 执行陷阱爆炸逻辑
 */
class TrapSystem {
public:
    static TrapSystem* getInstance();
    static void destroyInstance();
    
    /**
     * @brief 更新陷阱检测（每帧调用）
     * @param troopLayer 兵种层
     */
    void updateTrapDetection(BattleTroopLayer* troopLayer);
    
    /**
     * @brief 重置陷阱状态（战斗开始时调用）
     */
    void reset();

private:
    TrapSystem() = default;
    ~TrapSystem() = default;
    
    static TrapSystem* _instance;
    
    // ========== 陷阱触发追踪 ==========
    std::set<int> _triggeredTraps;       // 已触发的陷阱ID（等待爆炸）
    std::map<int, float> _trapTimers;    // 陷阱ID -> 剩余延迟时间
    
    // ========== 私有方法 ==========
    /**
     * @brief 检查兵种是否在陷阱范围内
     */
    bool isUnitInTrapRange(const BuildingInstance& trap, BattleUnitSprite* unit);
    
    /**
     * @brief 执行陷阱爆炸
     */
    void explodeTrap(BuildingInstance* trap, BattleTroopLayer* troopLayer);
};

#endif // __TRAP_SYSTEM_H__
