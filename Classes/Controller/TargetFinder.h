// TargetFinder.h
// 战斗目标查找器声明，为不同类型兵种提供目标选择策略

#ifndef __TARGET_FINDER_H__
#define __TARGET_FINDER_H__

#include "cocos2d.h"

struct BuildingInstance;
enum class UnitTypeID;

/**
 * 战斗目标查找器类
 * 
 * 职责：为不同兵种找到合适的攻击目标、根据优先级选择目标
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
