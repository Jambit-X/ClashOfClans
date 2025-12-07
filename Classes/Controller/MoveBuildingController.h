#ifndef __MOVE_BUILDING_CONTROLLER_H__
#define __MOVE_BUILDING_CONTROLLER_H__

#include "cocos2d.h"
#include <map>

// 前向声明
class BuildingManager;
class BuildingSprite;

/**
 * 建筑位置信息（用于统一坐标转换和合法性检查）
 */
struct BuildingPositionInfo {
    cocos2d::Vec2 gridPos;      // 对齐后的网格坐标
    cocos2d::Vec2 worldPos;     // 对齐后的世界坐标
    bool isValid;               // 位置是否合法
};

/**
 * 建筑移动控制器
 * 
 * 职责：
 * - 管理建筑移动的业务逻辑
 * - 处理触摸输入（拖动建筑）
 * - 实时预览建筑位置
 * - 检查放置合法性
 * - 处理建筑移动的完成或取消
 */
class MoveBuildingController {
public:
    /**
     * 构造函数
     * @param layer 父层（VillageLayer）
     * @param buildingManager 建筑管理器引用
     */
    MoveBuildingController(cocos2d::Layer* layer, BuildingManager* buildingManager);
    ~MoveBuildingController();

    /**
     * 设置触摸监听器（由 VillageLayer 调用一次）
     */
    void setupTouchListener();

    /**
     * 开始移动建筑
     * @param buildingId 要移动的建筑ID
     */
    void startMoving(int buildingId);

    /**
     * 取消建筑移动
     */
    void cancelMoving();

    /**
     * 检查是否正在移动建筑
     * @return 是否处于移动状态
     */
    bool isMoving() const { return _isMoving; }

    /**
     * 获取正在移动的建筑ID
     * @return 建筑ID，如果没有返回-1
     */
    int getMovingBuildingId() const { return _movingBuildingId; }

private:
    cocos2d::Layer* _parentLayer;        // 父层（用于坐标转换）
    BuildingManager* _buildingManager;   // 建筑管理器引用
    
    bool _isMoving;                      // 是否正在移动建筑（准备移动状态）
    bool _isDragging;                    // 是否正在拖动建筑
    int _movingBuildingId;               // 正在移动的建筑ID
    cocos2d::Vec2 _touchStartPos;        // 触摸开始位置（用于判断是否拖动）
    
    // 存储移动建筑的原始位置（用于取消时恢复）
    std::map<int, cocos2d::Vec2> _originalPositions;

    // 触摸事件监听器
    cocos2d::EventListenerTouchOneByOne* _touchListener;

    /**
     * 处理触摸开始
     */
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);

    /**
     * 处理触摸移动
     */
    void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);

    /**
     * 处理触摸结束
     */
    void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);

    /**
     * 更新建筑预览位置（拖动时调用）
     * @param worldPos 世界坐标
     */
    void updatePreviewPosition(const cocos2d::Vec2& worldPos);

    /**
     * 完成建筑移动（松手时调用）
     * @param worldPos 最终世界坐标
     * @return 是否成功放置
     */
    bool completeMove(const cocos2d::Vec2& worldPos);

    /**
     * 检查建筑是否可以放置在指定位置
     * @param buildingId 建筑ID
     * @param gridPos 网格坐标
     * @return 是否可以放置
     */
    bool canPlaceBuildingAt(int buildingId, const cocos2d::Vec2& gridPos);

    /**
     * 保存建筑原始位置
     * @param buildingId 建筑ID
     */
    void saveOriginalPosition(int buildingId);
    
    /**
     * 计算建筑位置信息（统一处理坐标转换、对齐和合法性检查）
     * @param touchWorldPos 触摸点的世界坐标
     * @param buildingId 建筑ID
     * @return 位置信息结构体
     */
    BuildingPositionInfo calculatePositionInfo(const cocos2d::Vec2& touchWorldPos, int buildingId);
};

#endif // __MOVE_BUILDING_CONTROLLER_H__
