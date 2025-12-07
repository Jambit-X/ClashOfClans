#ifndef __GRID_MAP_UTILS_H__
#define __GRID_MAP_UTILS_H__

#include "cocos2d.h"

/**
 * 网格地图工具类
 * 
 * 网格系统说明：
 * - 44x44 等轴测菱形网格（Isometric Grid）
 * - 网格坐标原点 (0,0) 对应菱形左顶点
 * - 网格坐标 (43,43) 对应菱形右顶点
 * - 世界坐标系：背景图左下角为(0,0)，Y轴向上
 * 
 * 网格区域参数：
 * - 中心：(1893, 1370)
 * - 左顶点：(660, 1365) ← 网格原点 (0, 0)
 * - 右顶点：(3128, 1365) ← 网格 (44, 44)
 * - 上顶点：(1893, 2293) ← 网格 (0, 44)
 * - 下顶点：(1893, 445) ← 网格 (44, 0)
 */
class GridMapUtils {
public:
    // 网格尺寸常量
    static const int GRID_WIDTH = 44;   // 网格宽度（格数）
    static const int GRID_HEIGHT = 44;  // 网格高度（格数）
    
    // 网格坐标系原点和单位向量（像素）
    static const float GRID_ORIGIN_X;   // 网格(0,0)对应的世界X坐标
    static const float GRID_ORIGIN_Y;   // 网格(0,0)对应的世界Y坐标
    
    static const float GRID_X_UNIT_X;   // 网格X+1方向的世界X偏移
    static const float GRID_X_UNIT_Y;   // 网格X+1方向的世界Y偏移
    
    static const float GRID_Y_UNIT_X;   // 网格Y+1方向的世界X偏移
    static const float GRID_Y_UNIT_Y;   // 网格Y+1方向的世界Y偏移
    
    // ========== 坐标转换 ==========
    
    /**
     * 世界坐标转网格坐标（精确）
     * @param pixelX 世界X坐标（像素）
     * @param pixelY 世界Y坐标（像素）
     * @return 网格坐标（可能是小数）
     */
    static cocos2d::Vec2 pixelToGrid(float pixelX, float pixelY);
    static cocos2d::Vec2 pixelToGrid(const cocos2d::Vec2& pixelPos);
    
    /**
     * 网格坐标转世界坐标（网格左下角）
     * @param gridX 网格X坐标
     * @param gridY 网格Y坐标
     * @return 世界坐标（该网格单元左下角的像素位置）
     */
    static cocos2d::Vec2 gridToPixel(int gridX, int gridY);
    static cocos2d::Vec2 gridToPixel(const cocos2d::Vec2& gridPos);
    
    /**
     * 网格坐标转世界坐标（网格中心）
     * @param gridX 网格X坐标
     * @param gridY 网格Y坐标
     * @return 世界坐标（该网格单元中心的像素位置）
     */
    static cocos2d::Vec2 gridToPixelCenter(int gridX, int gridY);
    static cocos2d::Vec2 gridToPixelCenter(const cocos2d::Vec2& gridPos);
    
    /**
     * 获取建筑中心的世界坐标
     * @param gridX 建筑左下角网格X坐标
     * @param gridY 建筑左下角网格Y坐标
     * @param width 建筑宽度（格数）
     * @param height 建筑高度（格数）
     * @return 建筑中心的世界坐标
     */
    static cocos2d::Vec2 getBuildingCenterPixel(int gridX, int gridY, int width, int height);
    
    // ========== 边界检测 ==========
    
    /**
     * 检查网格坐标是否在有效范围内
     * @param gridX 网格X坐标
     * @param gridY 网格Y坐标
     * @return 是否在 [0,43] 范围内
     */
    static bool isValidGridPosition(int gridX, int gridY);
    static bool isValidGridPosition(const cocos2d::Vec2& gridPos);
    
    /**
     * 检查建筑是否完全在网格范围内
     * @param gridX 建筑左下角网格X坐标
     * @param gridY 建筑左下角网格Y坐标
     * @param width 建筑宽度（格数）
     * @param height 建筑高度（格数）
     * @return 建筑所有占用的网格是否都在有效范围内
     */
    static bool isBuildingInBounds(int gridX, int gridY, int width, int height);
    
    /**
     * @brief 检查两个矩形是否重叠（用于碰撞检测）
     * @param pos1 第一个矩形的左下角位置（网格坐标）
     * @param size1 第一个矩形的大小（网格单位）
     * @param pos2 第二个矩形的左下角位置（网格坐标）
     * @param size2 第二个矩形的大小（网格单位）
     * @return 是否重叠
     */
    static bool isRectOverlap(
      const cocos2d::Vec2& pos1, const cocos2d::Size& size1,
      const cocos2d::Vec2& pos2, const cocos2d::Size& size2);
    
    /**
     * @brief 获取建筑的最终渲染位置（网格坐标 + 视觉偏移）
     * @param gridX 建筑左下角网格X坐标
     * @param gridY 建筑左下角网格Y坐标
     * @param visualOffset 视觉偏移量（从 BuildingSprite::getVisualOffset() 获取）
     * @return 最终渲染位置（世界坐标）
     */
    static cocos2d::Vec2 getVisualPosition(int gridX, int gridY, const cocos2d::Vec2& visualOffset);
};

#endif // __GRID_MAP_UTILS_H__
