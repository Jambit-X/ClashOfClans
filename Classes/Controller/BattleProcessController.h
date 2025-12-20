#pragma once

#include "cocos2d.h"
#include "../Sprite/BattleUnitSprite.h"
#include "../Layer/BattleTroopLayer.h"
#include "../Model/VillageData.h"
#include <map>

USING_NS_CC;

// Forward declarations
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

    // ========== 建筑防御系统 ==========
    BattleUnitSprite* findNearestUnitInRange(const BuildingInstance& building, float attackRange, BattleTroopLayer* troopLayer);
    std::vector<BattleUnitSprite*> getAllUnitsInRange(const BuildingInstance& building, float attackRange, BattleTroopLayer* troopLayer);
    
    // ========== 建筑防御自动更新 ==========
    void updateBuildingDefense(BattleTroopLayer* troopLayer);

    // 炸弹兵自爆攻击（单体伤害版本）
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
    
    // ========== 累积伤害系统 ==========
    std::map<BattleUnitSprite*, float> _accumulatedDamage;  // 兵种 -> 累积伤害

    // ========== 目标选择 ==========
    const BuildingInstance* findTargetWithResourcePriority(const cocos2d::Vec2& unitWorldPos, UnitTypeID unitType);
    const BuildingInstance* findTargetWithDefensePriority(const cocos2d::Vec2& unitWorldPos, UnitTypeID unitType);
    const BuildingInstance* findNearestBuilding(const cocos2d::Vec2& unitWorldPos, UnitTypeID unitType);
    const BuildingInstance* findNearestWall(const cocos2d::Vec2& unitWorldPos);  // 炸弹人专用
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
