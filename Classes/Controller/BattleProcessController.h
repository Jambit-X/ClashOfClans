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
 * 
 * 职责：
 * 1. 控制单位的AI行为（寻路、攻击、目标选择）
 * 2. 管理战斗逻辑（伤害计算、战斗状态等）
 * 3. 分离控制逻辑与显示层
 */
class BattleProcessController {
public:
    /**
     * @brief 获取单例
     */
    static BattleProcessController* getInstance();
    
    /**
     * @brief 销毁单例
     */
    static void destroyInstance();
    
    /**
     * @brief 启动单位AI（寻路+攻击）
     * @param unit 要控制的单位
     * @param troopLayer 单位所在的层（用于获取坐标系）
     */
    void startUnitAI(BattleUnitSprite* unit, BattleTroopLayer* troopLayer);
    
    /**
     * @brief 开始持续战斗循环（攻击完成后自动寻找新目标）
     * @param unit 要控制的单位
     * @param troopLayer 单位所在的层
     */
    void startCombatLoop(BattleUnitSprite* unit, BattleTroopLayer* troopLayer);
    
    /**
     * @brief 开始持续战斗循环（攻击完成后自动寻找新目标），强制指定目标
     * @param unit 要控制的单位
     * @param troopLayer 单位所在的层
     * @param forcedTarget 强制攻击的目标建筑
     */
    void startCombatLoopWithForcedTarget(BattleUnitSprite* unit, BattleTroopLayer* troopLayer, const BuildingInstance* forcedTarget);
    
    /**
     * @brief 持续单位战斗逻辑（如攻击、施法等）
     * @param unit 要控制的单位
     * @param dt 时间间隔
     */
    void updateUnitBattleLogic(BattleUnitSprite* unit, float dt);
    
    /**
     * @brief 在战斗结束或退出战斗场景时重置所有建筑的战斗状态（恢复HP、清除摧毁标志）
     * 注意：该函数不会修改存档，只在内存中恢复。
     */
    void resetBattleState();

private:
    BattleProcessController() = default;
    ~BattleProcessController() = default;
    
    // 禁止拷贝和赋值
    BattleProcessController(const BattleProcessController&) = delete;
    BattleProcessController& operator=(const BattleProcessController&) = delete;
    
    static BattleProcessController* _instance;

    // ========== 智能目标选择 ==========
    struct TargetCandidate {
        const BuildingInstance* building;
        float directDistance;      // 直线距离
        float pathLength;            // 实际路径长度
        bool needsBreakingWalls;   // 是否需要破墙
        float score;               // 综合评分
    };

    // 寻找最佳攻击目标（考虑距离和城墙）
    const BuildingInstance* findBestTargetBuilding(const cocos2d::Vec2& unitWorldPos);
   
    // 旧方法（保留兼容性）
    const BuildingInstance* findNearestBuilding(const cocos2d::Vec2& unitWorldPos);

    // 新增：射线检测寻找路径上第一个城墙（使用世界像素坐标）
    const BuildingInstance* getFirstWallInLine(const cocos2d::Vec2& startPixel, const cocos2d::Vec2& endPixel);

    // 绕路阈值：允许绕多少格来避免破墙
    static constexpr int WALL_DETOUR_THRESHOLD = 2;
    
    // 搜索范围扩展：基础5个 + 阈值2个 = 7个候选建筑
    static constexpr int BASE_SEARCH_COUNT = 5;
    static constexpr int EXPANDED_SEARCH_COUNT = BASE_SEARCH_COUNT + WALL_DETOUR_THRESHOLD;

};
