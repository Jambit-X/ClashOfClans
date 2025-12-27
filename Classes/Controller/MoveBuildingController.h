// MoveBuildingController.h
// 建筑移动控制器声明，管理建筑拖动、位置验证和放置逻辑

#ifndef __MOVE_BUILDING_CONTROLLER_H__
#define __MOVE_BUILDING_CONTROLLER_H__

#include "cocos2d.h"
#include <map>

// 前向声明
class BuildingManager;
class BuildingSprite;

// 建筑位置信息结构
struct BuildingPositionInfo {
    cocos2d::Vec2 gridPos;      // 对齐后的网格坐标
    cocos2d::Vec2 worldPos;     // 对齐后的世界坐标
    bool isValid;               // 位置是否合法
};

// 建筑移动控制器类
// 职责：管理建筑移动业务逻辑、处理触摸输入、实时预览位置、检查放置合法性
class MoveBuildingController {
public:
    // 构造函数
    MoveBuildingController(cocos2d::Layer* layer, BuildingManager* buildingManager);
    ~MoveBuildingController();

    // 设置触摸监听器
    void setupTouchListener();

    // 开始移动建筑
    void startMoving(int buildingId);

    // 取消建筑移动
    void cancelMoving();

    // 是否正在移动建筑
    bool isMoving() const { return _isMoving; }

    // 获取正在移动的建筑ID（无则返回-1）
    int getMovingBuildingId() const { return _movingBuildingId; }

    // 设置建筑点击回调（短按时触发）
    void setOnBuildingTappedCallback(std::function<void(int)> callback) {
        _onBuildingTapped = callback;
    }

private:
    cocos2d::Layer* _parentLayer;        // 父层
    BuildingManager* _buildingManager;   // 建筑管理器引用
    
    bool _isMoving;                      // 是否正在移动建筑
    bool _isDragging;                    // 是否正在拖动
    int _movingBuildingId;               // 正在移动的建筑ID
    cocos2d::Vec2 _touchStartPos;        // 触摸开始位置
    
    // 原始位置存储（用于取消时恢复）
    std::map<int, cocos2d::Vec2> _originalPositions;

    // 触摸事件监听器
    cocos2d::EventListenerTouchOneByOne* _touchListener;

    // 长按检测相关
    float _touchDownTime;                // 触摸按下时间戳
    bool _isLongPressTriggered;          // 是否已触发长按
    int _touchedBuildingId;              // 触摸到的建筑ID
    
    static constexpr float LONG_PRESS_DURATION = 0.5f;  // 长按时长阈值（秒）

    // 回调函数
    std::function<void(int)> _onBuildingTapped;  // 短按建筑回调

    // 处理触摸开始
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);

    // 处理触摸移动
    void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);

    // 处理触摸结束
    void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);

    // 更新建筑预览位置
    void updatePreviewPosition(const cocos2d::Vec2& worldPos);

    // 完成建筑移动
    bool completeMove(const cocos2d::Vec2& worldPos);

    // 检查建筑是否可以放置在指定位置
    bool canPlaceBuildingAt(int buildingId, const cocos2d::Vec2& gridPos);

    // 保存建筑原始位置
    void saveOriginalPosition(int buildingId);
    
    // 计算建筑位置信息（统一处理坐标转换、对齐和合法性检查）
    BuildingPositionInfo calculatePositionInfo(const cocos2d::Vec2& touchWorldPos, int buildingId);

    // 检查是否达到长按时长
    bool checkLongPress();
};

#endif // __MOVE_BUILDING_CONTROLLER_H__
