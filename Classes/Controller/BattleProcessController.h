#pragma once

#include "cocos2d.h"
#include "../Model/VillageData.h"
#include <map>
#include <set>
#include <functional>

USING_NS_CC;

// Forward declarations
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
    
    void startUnitAI(BattleUnitSprite* unit, BattleTroopLayer* troopLayer);
    void startCombatLoop(BattleUnitSprite* unit, BattleTroopLayer* troopLayer);
    void startCombatLoopWithForcedTarget(BattleUnitSprite* unit, BattleTroopLayer* troopLayer, const BuildingInstance* forcedTarget);
    void resetBattleState();

    // ========== 常量 =========
    static constexpr float PIXEL_DETOUR_THRESHOLD = 800.0f;

    // ========== 建筑防御系统（已迁移到DefenseSystem）==========
    // 使用 DefenseSystem::getInstance()->updateBuildingDefense()

    // ========== 陷阱系统（已迁移到TrapSystem）==========
    // 使用 TrapSystem::getInstance()->updateTrapDetection()

    // 炸弹兵自爆攻击（单体伤害版本）
    void performWallBreakerSuicideAttack(
        BattleUnitSprite* unit,
        BuildingInstance* target,
        BattleTroopLayer* troopLayer,
        const std::function<void()>& onComplete
    );

    // ========== 摧毁进度追踪系统（已迁移到DestructionTracker）==========
    // 使用 DestructionTracker::getInstance()->initTracking()
    // 使用 DestructionTracker::getInstance()->updateProgress()
    // 使用 DestructionTracker::getInstance()->getProgress()
    // 使用 DestructionTracker::getInstance()->getStars()
private:
    BattleProcessController() = default;
    ~BattleProcessController() = default;
    
    BattleProcessController(const BattleProcessController&) = delete;
    BattleProcessController& operator=(const BattleProcessController&) = delete;
    
    static BattleProcessController* _instance;
    
    // ========== 累积伤害系统 ==========
    std::map<BattleUnitSprite*, float> _accumulatedDamage;  // 兵种 -> 累积伤害

    // ========== 陷阱系统（已迁移到TrapSystem）==========

    // ========== 目标选择（委托给 TargetFinder）==========
    // 使用 TargetFinder::getInstance()->findTarget() 等方法
    const BuildingInstance* getFirstWallInLine(const cocos2d::Vec2& startPixel, const cocos2d::Vec2& endPixel);

    // ========== 核心攻击逻辑 =========
    void executeAttack(
        BattleUnitSprite* unit,
        BattleTroopLayer* troopLayer,
        int targetID,
        bool isForcedTarget,
        const std::function<void()>& onTargetDestroyed,
        const std::function<void()>& onContinueAttack
    );





    bool shouldAbandonWallForBetterPath(BattleUnitSprite* unit, int currentWallID);
};

// 事件数据结构已迁移到 DestructionTracker.h
