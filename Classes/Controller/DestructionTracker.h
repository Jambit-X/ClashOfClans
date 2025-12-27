// DestructionTracker.h
// 摧毁进度追踪系统声明，管理战斗中建筑摧毁进度和星级判定

#ifndef __DESTRUCTION_TRACKER_H__
#define __DESTRUCTION_TRACKER_H__

#include "cocos2d.h"
#include <string>

// 摧毁进度追踪系统类
// 职责：计算总血量、追踪摧毁进度、检查和发送星级事件
class DestructionTracker {
public:
    static DestructionTracker* getInstance();
    static void destroyInstance();
    
    // 初始化摧毁追踪（战斗开始时调用）
    void initTracking();
    
    // 计算总血量（排除城墙和陷阱）
    int calculateTotalBuildingHP();
    
    // 更新摧毁进度（建筑被摧毁时调用）
    void updateProgress();
    
    // 获取当前摧毁进度百分比 (0.0-100.0)
    float getProgress();
    
    // 获取当前星数 (0-3)
    int getStars();
    
    // 重置追踪状态
    void reset();

private:
    DestructionTracker() = default;
    ~DestructionTracker() = default;
    
    static DestructionTracker* _instance;
    
    // 检查星级条件并发送事件
    void checkStarConditions(float progress, bool townHallDestroyed);
    
    int _totalBuildingHP = 0;        // 总血量（不含城墙和陷阱）
    int _currentStars = 0;           // 当前星数
    bool _townHallDestroyed = false; // 大本营是否已摧毁
    bool _star50Awarded = false;     // 50%星是否已获得
    bool _star100Awarded = false;    // 100%星是否已获得
};

// 摧毁进度更新事件数据
struct DestructionProgressEventData {
    float progress;  // 进度百分比 (0.0-100.0)
    int stars;       // 当前星数 (0-3)
};

// 星星获得事件数据
struct StarAwardedEventData {
    int starIndex;          // 获得的星星索引 (0/1/2)
    std::string reason;     // 获得原因 ("50%", "townhall", "100%")
};

#endif // __DESTRUCTION_TRACKER_H__
