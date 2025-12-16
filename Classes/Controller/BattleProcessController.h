#pragma once

#include "cocos2d.h"
#include "../Sprite/BattleUnitSprite.h"
#include "../Layer/BattleTroopLayer.h"
#include "../Model/VillageData.h"

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

private:
    BattleProcessController() = default;
    ~BattleProcessController() = default;
    
    BattleProcessController(const BattleProcessController&) = delete;
    BattleProcessController& operator=(const BattleProcessController&) = delete;
    
    static BattleProcessController* _instance;

    // ========== 目标选择 ==========C
    const BuildingInstance* findBestTargetBuilding(const cocos2d::Vec2& unitWorldPos);
    const BuildingInstance* findNearestBuilding(const cocos2d::Vec2& unitWorldPos);
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

    // ========== 常量 =========
    static constexpr float ATTACK_RANGE = 150.0f;
    static constexpr float PIXEL_DETOUR_THRESHOLD = 800.0f;
};
