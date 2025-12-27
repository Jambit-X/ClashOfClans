// TrapSystem.h
// 陷阱系统声明，管理陷阱的触发检测和爆炸逻辑

#ifndef __TRAP_SYSTEM_H__
#define __TRAP_SYSTEM_H__

#include "cocos2d.h"
#include <set>
#include <map>

class BattleUnitSprite;
class BattleTroopLayer;
struct BuildingInstance;

// 陷阱系统类
// 职责：检测兵种是否踩到陷阱、管理触发延迟、执行爆炸逻辑
class TrapSystem {
public:
    static TrapSystem* getInstance();
    static void destroyInstance();
    
    // 更新陷阱检测（每帧调用）
    void updateTrapDetection(BattleTroopLayer* troopLayer);
    
    // 重置陷阱状态（战斗开始时调用）
    void reset();

private:
    TrapSystem() = default;
    ~TrapSystem() = default;
    
    static TrapSystem* _instance;
    
    // 陷阱触发追踪
    std::set<int> _triggeredTraps;       // 已触发的陷阱ID
    std::map<int, float> _trapTimers;    // 陷阱ID -> 剩余延迟时间
    
    // 检查兵种是否在陷阱范围内
    bool isUnitInTrapRange(const BuildingInstance& trap, BattleUnitSprite* unit);
    
    // 执行陷阱爆炸
    void explodeTrap(BuildingInstance* trap, BattleTroopLayer* troopLayer);
};

#endif // __TRAP_SYSTEM_H__
