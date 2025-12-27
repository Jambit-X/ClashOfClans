// BattleProcessController.h
// 战斗流程控制器声明，管理战斗中的单位AI和行为逻辑

#pragma once

#include "cocos2d.h"
#include "../Model/VillageData.h"
#include <map>
#include <set>
#include <functional>

USING_NS_CC;

class BattleUnitSprite;
class BattleTroopLayer;
struct BuildingInstance;

/**
 * @brief 战斗流程控制器 - 管理战斗中的单位AI和行为逻辑
 */
class BattleProcessController {
public:
    static BattleProcessController* getInstance();
    static void destroyInstance();
    
    // 启动单位AI
    void startUnitAI(BattleUnitSprite* unit, BattleTroopLayer* troopLayer);
    
    // 启动战斗循环
    void startCombatLoop(BattleUnitSprite* unit, BattleTroopLayer* troopLayer);
    void startCombatLoopWithForcedTarget(BattleUnitSprite* unit, BattleTroopLayer* troopLayer, const BuildingInstance* forcedTarget);
    
    // 重置战斗状态
    void resetBattleState();

    // 绕路阈值常量（像素）
    static constexpr float PIXEL_DETOUR_THRESHOLD = 800.0f;

    // 炸弹兵自爆攻击
    void performWallBreakerSuicideAttack(
        BattleUnitSprite* unit,
        BuildingInstance* target,
        BattleTroopLayer* troopLayer,
        const std::function<void()>& onComplete
    );

private:
    BattleProcessController() = default;
    ~BattleProcessController() = default;
    
    BattleProcessController(const BattleProcessController&) = delete;
    BattleProcessController& operator=(const BattleProcessController&) = delete;
    
    static BattleProcessController* _instance;
    
    // 累积伤害系统
    std::map<BattleUnitSprite*, float> _accumulatedDamage;

    // 获取直线上第一个城墙
    const BuildingInstance* getFirstWallInLine(const cocos2d::Vec2& startPixel, const cocos2d::Vec2& endPixel);

    // 执行攻击逻辑
    void executeAttack(
        BattleUnitSprite* unit,
        BattleTroopLayer* troopLayer,
        int targetID,
        bool isForcedTarget,
        const std::function<void()>& onTargetDestroyed,
        const std::function<void()>& onContinueAttack
    );

    // 判断是否应放弃当前城墙寻找更优路径
    bool shouldAbandonWallForBetterPath(BattleUnitSprite* unit, int currentWallID);
};
