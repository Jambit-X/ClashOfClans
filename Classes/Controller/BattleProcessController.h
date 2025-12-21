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

    // ========== 摧毁进度追踪系统 ==========
    /**
     * @brief 初始化摧毁追踪（战斗开始时调用）
     */
    void initDestructionTracking();

    /**
     * @brief 计算总血量（排除城墙和陷阱）
     * @return 所有有效建筑的总血量
     */
    int calculateTotalBuildingHP();

    /**
     * @brief 更新摧毁进度（建筑被摧毁时调用）
     */
    void updateDestructionProgress();

    /**
     * @brief 检查星级条件并发送事件
     * @param progress 当前摧毁进度 (0.0-100.0)
     * @param townHallDestroyed 大本营是否被摧毁
     */
    void checkStarConditions(float progress, bool townHallDestroyed);

    /**
     * @brief 获取当前摧毁进度
     * @return 进度百分比 (0.0-100.0)
     */
    float getDestructionProgress();

    /**
     * @brief 获取当前星数
     * @return 星数 (0-3)
     */
    int getCurrentStars();
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
    // ========== 摧毁进度追踪成员变量 ==========
    int _totalBuildingHP;        // 总血量（不含城墙和陷阱）
    int _currentStars;           // 当前星数 (0-3)
    bool _townHallDestroyed;     // 大本营是否已摧毁
    bool _star50Awarded;         // 50% 星是否已获得
    bool _star100Awarded;        // 100% 星是否已获得

    bool shouldAbandonWallForBetterPath(BattleUnitSprite* unit, int currentWallID);
};

// ========== 事件数据结构 ==========

/**
 * @brief 摧毁进度更新事件数据
 */
struct DestructionProgressEventData {
    float progress;  // 进度百分比 (0.0-100.0)
    int stars;       // 当前星数 (0-3)
};

/**
 * @brief 星星获得事件数据
 */
struct StarAwardedEventData {
    int starIndex;          // 获得的星星索引 (0/1/2 对应第1/2/3颗星)
    std::string reason;     // 获得原因 ("50%", "townhall", "100%")
};
