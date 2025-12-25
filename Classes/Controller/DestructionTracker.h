#ifndef __DESTRUCTION_TRACKER_H__
#define __DESTRUCTION_TRACKER_H__

#include "cocos2d.h"
#include <string>

/**
 * DestructionTracker - 摧毁进度追踪系统
 * 
 * 职责：
 * 1. 计算建筑总血量
 * 2. 追踪战斗中的摧毁进度
 * 3. 检查和发送星级条件事件
 */
class DestructionTracker {
public:
    static DestructionTracker* getInstance();
    static void destroyInstance();
    
    /**
     * @brief 初始化摧毁追踪（战斗开始时调用）
     */
    void initTracking();
    
    /**
     * @brief 计算总血量（排除城墙和陷阱）
     * @return 所有有效建筑的总血量
     */
    int calculateTotalBuildingHP();
    
    /**
     * @brief 更新摧毁进度（建筑被摧毁时调用）
     */
    void updateProgress();
    
    /**
     * @brief 获取当前摧毁进度
     * @return 进度百分比 (0.0-100.0)
     */
    float getProgress();
    
    /**
     * @brief 获取当前星数
     * @return 星数 (0-3)
     */
    int getStars();
    
    /**
     * @brief 重置追踪状态
     */
    void reset();

private:
    DestructionTracker() = default;
    ~DestructionTracker() = default;
    
    static DestructionTracker* _instance;
    
    /**
     * @brief 检查星级条件并发送事件
     */
    void checkStarConditions(float progress, bool townHallDestroyed);
    
    // ========== 成员变量 ==========
    int _totalBuildingHP = 0;        // 总血量（不含城墙和陷阱）
    int _currentStars = 0;           // 当前星数 (0-3)
    bool _townHallDestroyed = false; // 大本营是否已摧毁
    bool _star50Awarded = false;     // 50% 星是否已获得
    bool _star100Awarded = false;    // 100% 星是否已获得
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

#endif // __DESTRUCTION_TRACKER_H__
