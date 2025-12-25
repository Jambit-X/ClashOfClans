#ifndef __TARGET_FINDER_H__
#define __TARGET_FINDER_H__

#include "cocos2d.h"

struct BuildingInstance;
enum class UnitTypeID;

/**
 * TargetFinder - 战斗目标查找器
 * 
 * 职责：
 * 1. 为不同类型的兵种找到合适的攻击目标
 * 2. 根据兵种优先级（资源/防御）选择目标
 * 3. 为炸弹兵查找最近城墙
 */
class TargetFinder {
public:
    static TargetFinder* getInstance();
    static void destroyInstance();

    // ========== 目标查找接口 ==========
    
    // 通用入口：根据兵种类型自动选择策略
    const BuildingInstance* findTarget(const cocos2d::Vec2& unitWorldPos, UnitTypeID unitType);

    // 资源优先查找（哥布林等）
    const BuildingInstance* findTargetWithResourcePriority(const cocos2d::Vec2& unitWorldPos, UnitTypeID unitType);
    
    // 防御优先查找（巨人、气球等）
    const BuildingInstance* findTargetWithDefensePriority(const cocos2d::Vec2& unitWorldPos, UnitTypeID unitType);
    
    // 查找最近城墙（炸弹兵专用）
    const BuildingInstance* findNearestWall(const cocos2d::Vec2& unitWorldPos);

private:
    TargetFinder() = default;
    ~TargetFinder() = default;
    
    static TargetFinder* _instance;
};

#endif // __TARGET_FINDER_H__
