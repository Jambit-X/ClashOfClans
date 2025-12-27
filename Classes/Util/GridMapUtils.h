// GridMapUtils.h
// 网格地图工具类，提供等轴测网格坐标系统的转换和计算功能

#ifndef __GRID_MAP_UTILS_H__
#define __GRID_MAP_UTILS_H__

#include "cocos2d.h"

/**
 * @brief 网格地图工具类
 * 
 * ===================================================================================
 * 网格系统设计说明
 * ===================================================================================
 * 
 * 1. 网格类型：44x44 等轴测菱形网格（Isometric Diamond Grid）
 * 
 * 2. 坐标系统：
 *    - 网格坐标：(gridX, gridY)，范围 [0, 43]
 *    - 世界坐标：(pixelX, pixelY)，背景图坐标系，左下角为原点(0,0)
 * 
 * 3. 网格布局（实测数据）：
 *    - 菱形中心：(1893, 1370)
 *    - 左顶点：(660, 1365)  ← 网格原点(0,0)
 *    - 右顶点：(3128, 1365) ← 网格(44,44)
 *    - 上顶点：(1893, 2293) ← 网格(0,44)
 *    - 下顶点：(1893, 445)  ← 网格(44,0)
 * 
 * 4. 坐标转换公式：
 *    仿射变换：pixel = origin + gridX * xUnit + gridY * yUnit
 *    
 *    其中：
 *    - origin = (660, 1365)     // 网格原点
 *    - xUnit = (28.02, -20.91)  // 网格X方向单位向量
 *    - yUnit = (28.02, 21.09)   // 网格Y方向单位向量
 * 
 * 5. 网格特点：
 *    - X轴：向右下方向（+X → 右下）
 *    - Y轴：向右上方向（+Y → 右上）
 *    - 对称性：菱形关于中心对称
 * 
 * ===================================================================================
 */
class GridMapUtils {
public:
    // ========== 网格系统常量 ==========
    
    static const int GRID_WIDTH = 44;   // 网格宽度（X方向格数）
    static const int GRID_HEIGHT = 44;  // 网格高度（Y方向格数）
    
    // 网格原点坐标（左顶点）
    static const float GRID_ORIGIN_X;   // 660.0f
    static const float GRID_ORIGIN_Y;   // 1365.0f
    
    // 网格X轴单位向量（gridX+1的像素偏移）
    static const float GRID_X_UNIT_X;   // 28.02f  - 向右偏移
    static const float GRID_X_UNIT_Y;   // -20.91f - 向下偏移
    
    // 网格Y轴单位向量（gridY+1的像素偏移）
    static const float GRID_Y_UNIT_X;   // 28.02f  - 向右偏移
    static const float GRID_Y_UNIT_Y;   // 21.09f  - 向上偏移
    
    // ========== 坐标转换：世界坐标 ↔ 网格坐标 ==========
    
    /**
     * @brief 世界坐标转网格坐标（精确转换，可能得到小数）
     * @param pixelX 世界X坐标（像素）
     * @param pixelY 世界Y坐标（像素）
     * @return 网格坐标（Vec2，可能是小数）
     * 
     * 实现方式：
     * 1. 计算相对于原点的偏移：delta = pixel - origin
     * 2. 使用逆仿射变换求解gridX和gridY：
     *    [ GRID_X_UNIT_X  GRID_Y_UNIT_X ]   [ gridX ]   [ deltaX ]
     *    [ GRID_X_UNIT_Y  GRID_Y_UNIT_Y ] * [ gridY ] = [ deltaY ]
     * 3. 使用克拉默法则求逆矩阵
     * 
     * 应用场景：
     * - 鼠标点击位置转网格坐标
     * - 精灵世界坐标转网格坐标
     * - 寻路系统中的坐标转换
     */
    static cocos2d::Vec2 pixelToGrid(float pixelX, float pixelY);
    static cocos2d::Vec2 pixelToGrid(const cocos2d::Vec2& pixelPos);
    
    /**
     * @brief 网格坐标转世界坐标（网格左下角）
     * @param gridX 网格X坐标
     * @param gridY 网格Y坐标
     * @return 世界坐标（该网格单元左下角的像素位置）
     * 
     * 实现方式：
     * pixel = origin + gridX * xUnit + gridY * yUnit
     * 
     * 应用场景：
     * - 建筑放置时计算精灵位置
     * - 网格吸附功能
     */
    static cocos2d::Vec2 gridToPixel(int gridX, int gridY);
    static cocos2d::Vec2 gridToPixel(const cocos2d::Vec2& gridPos);
    
    /**
     * @brief 网格坐标转世界坐标（网格中心）
     * @param gridX 网格X坐标
     * @param gridY 网格Y坐标
     * @return 世界坐标（该网格单元中心的像素位置）
     * 
     * 实现方式：
     * center = gridToPixel(gridX, gridY) + (xUnit + yUnit) * 0.5
     * 
     * 应用场景：
     * - 单位移动目标点（站在格子中心）
     * - 攻击目标点计算
     */
    static cocos2d::Vec2 gridToPixelCenter(int gridX, int gridY);
    static cocos2d::Vec2 gridToPixelCenter(const cocos2d::Vec2& gridPos);
    
    /**
     * @brief 计算建筑中心的世界坐标
     * @param gridX 建筑左下角网格X坐标
     * @param gridY 建筑左下角网格Y坐标
     * @param width 建筑宽度（格数）
     * @param height 建筑高度（格数）
     * @return 建筑中心的世界坐标
     * 
     * 计算方式：
     * 1. 建筑中心在网格坐标系中为：(gridX + width/2, gridY + height/2)
     * 2. 转换为世界坐标：pixel = origin + centerGridX * xUnit + centerGridY * yUnit
     * 
     * 应用场景：
     * - 寻路系统计算建筑中心作为目标点
     * - 攻击范围判断
     * - 特效播放位置
     * 
     * 示例：3x3建筑在(10,10)，中心为(11.5, 11.5)
     */
    static cocos2d::Vec2 getBuildingCenterPixel(int gridX, int gridY, int width, int height);
    
    // ========== 边界检测 ==========
    
    /**
     * @brief 检查网格坐标是否在有效范围内
     * @param gridX 网格X坐标
     * @param gridY 网格Y坐标
     * @return 是否在 [0, 43] 范围内
     * 
     * 应用场景：
     * - 建筑放置前的合法性检查
     * - 寻路时检查目标点是否越界
     * - 边界碰撞检测
     */
    static bool isValidGridPosition(int gridX, int gridY);
    static bool isValidGridPosition(const cocos2d::Vec2& gridPos);
    
    /**
     * @brief 检查建筑是否完全在网格范围内
     * @param gridX 建筑左下角网格X坐标
     * @param gridY 建筑左下角网格Y坐标
     * @param width 建筑宽度（格数）
     * @param height 建筑高度（格数）
     * @return 建筑所有占用的网格是否都在有效范围内
     * 
     * 检查方式：
     * - 左下角：(gridX, gridY) 是否有效
     * - 右上角：(gridX + width - 1, gridY + height - 1) 是否有效
     * 
     * 应用场景：
     * - 建筑拖动时的边界检查
     * - 随机地图生成时的有效性判断
     */
    static bool isBuildingInBounds(int gridX, int gridY, int width, int height);
    
    /**
     * @brief 检查两个矩形是否重叠（用于碰撞检测）
     * @param pos1 第一个矩形的左下角位置（网格坐标）
     * @param size1 第一个矩形的大小（网格单位）
     * @param pos2 第二个矩形的左下角位置（网格坐标）
     * @param size2 第二个矩形的大小（网格单位）
     * @return 是否重叠
     * 
     * 判断逻辑：使用AABB包围盒碰撞检测
     * 不重叠的情况：
     * - rect1在rect2左边
     * - rect1在rect2右边
     * - rect1在rect2下边
     * - rect1在rect2上边
     * 
     * 应用场景：
     * - 建筑放置时的重叠检查
     * - 单位移动时的碰撞检测
     */
    static bool isRectOverlap(
      const cocos2d::Vec2& pos1, const cocos2d::Size& size1,
      const cocos2d::Vec2& pos2, const cocos2d::Size& size2);
    
    /**
     * @brief 获取建筑的最终渲染位置（网格坐标 + 视觉偏移）
     * @param gridX 建筑左下角网格X坐标
     * @param gridY 建筑左下角网格Y坐标
     * @param visualOffset 视觉偏移量（从BuildingSprite::getVisualOffset()获取）
     * @return 最终渲染位置（世界坐标）
     * 
     * 说明：
     * 某些建筑（如塔楼）需要额外的视觉偏移，使其看起来更居中
     * visualOffset通常从BuildingConfig中读取
     * 
     * 应用场景：
     * - 建筑精灵的最终显示位置
     * - 攻击特效的起始位置
     */
    static cocos2d::Vec2 getVisualPosition(int gridX, int gridY, const cocos2d::Vec2& visualOffset);

    // ========== 渲染层级（Z-Order）计算 ==========
    
    /**
     * @brief 计算统一的渲染层级（Z-Order）
     * @param gridX 网格X坐标
     * @param gridY 网格Y坐标
     * @return Z-Order值
     * 
     * 计算公式：
     * zOrder = gridX - gridY + 45
     * 
     * 设计原理：
     * 1. 等轴测视角中，X越大的物体应该显示在前面（+gridX）
     * 2. Y越大的物体应该显示在后面（-gridY）
     * 3. 加45是为了确保结果为正数（网格最大44，-44+45=1 > 0）
     * 
     * 应用场景：
     * - 建筑精灵的初始Z-Order
     * - 单位移动时动态更新Z-Order，确保透视正确
     * 
     * 示例：
     * - (0, 0) → 45
     * - (10, 5) → 50 （显示在前面）
     * - (5, 10) → 40 （显示在后面）
     */
    static int calculateZOrder(int gridX, int gridY) {
        return gridX - gridY + 45; 
    }
};

#endif // __GRID_MAP_UTILS_H__
