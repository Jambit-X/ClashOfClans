#ifndef __GRID_MAP_UTILS_H__
#define __GRID_MAP_UTILS_H__

#include "cocos2d.h"

/**
 * 网格地图工具类
 * 
 * 网格系统说明：
 * - 44x44 等距菱形网格（Isometric Grid）
 * - 网格坐标原点 (0,0) 在网格区域左下角
 * - 网格坐标 (43,43) 在网格区域右上角
 * - 像素坐标锚点在背景图左下角
 * 
 * 网格区域参数：
 * - 背景图尺寸：3705 x 2545 像素
 * - 网格区域中心：(1893, 1370)
 * - 网格区域顶点：
 *   - 左顶点：(660, 1371)
 *   - 右顶点：(3128, 1370)
 *   - 上顶点：(1893, 2293)
 *   - 下顶点：(1893, 445)
 */
class GridMapUtils {
public:
    // 网格参数常量
    static const int GRID_WIDTH = 44;   // 网格宽度（格数）
    static const int GRID_HEIGHT = 44;  // 网格高度（格数）
    
    // 网格区域中心像素坐标
    static const int CENTER_X = 1893;
    static const int CENTER_Y = 1370;
    
    // 网格区域菱形顶点
    static const int LEFT_VERTEX_X = 660;
    static const int LEFT_VERTEX_Y = 1371;
    static const int RIGHT_VERTEX_X = 3128;
    static const int RIGHT_VERTEX_Y = 1370;
    static const int TOP_VERTEX_X = 1893;
    static const int TOP_VERTEX_Y = 2293;
    static const int BOTTOM_VERTEX_X = 1893;
    static const int BOTTOM_VERTEX_Y = 445;
    
    /**
     * 像素坐标转网格坐标
     * @param pixelPos 像素坐标（锚点为背景图左下角）
     * @return 网格坐标（可能是小数，表示在网格内的精确位置）
     */
    static cocos2d::Vec2 pixelToGrid(const cocos2d::Vec2& pixelPos);
    
    /**
     * 网格坐标转像素坐标（网格左下角）
     * @param gridPos 网格坐标
     * @return 像素坐标（网格单元左下角的像素位置）
     */
    static cocos2d::Vec2 gridToPixel(const cocos2d::Vec2& gridPos);
    
    /**
     * 网格坐标转像素坐标（网格中心）
     * @param gridPos 网格坐标
     * @return 像素坐标（网格单元中心的像素位置）
     */
    static cocos2d::Vec2 gridToPixelCenter(const cocos2d::Vec2& gridPos);
    
    /**
     * 像素坐标对齐到最近的网格
     * @param pixelPos 像素坐标
     * @return 对齐后的网格坐标（整数）
     */
    static cocos2d::Vec2 snapToGrid(const cocos2d::Vec2& pixelPos);
    
    /**
     * 检查网格坐标是否在有效范围内
     * @param gridX 网格X坐标
     * @param gridY 网格Y坐标
     * @return 是否在 [0,43] 范围内
     */
    static bool isValidGridPosition(int gridX, int gridY);
    
    /**
     * 检查建筑是否可以放置在指定网格位置
     * @param gridX 网格X坐标（建筑左下角）
     * @param gridY 网格Y坐标（建筑左下角）
     * @param width 建筑宽度（格数）
     * @param height 建筑高度（格数）
     * @return 是否所有占用的网格都在有效范围内
     */
    static bool canPlaceBuilding(int gridX, int gridY, int width, int height);

private:
    // 等距网格的单位向量（像素）
    // 从网格中心到右顶点的向量 / 22（半个网格宽度）
    static float GRID_UNIT_X;  // X方向单位向量长度
    static float GRID_UNIT_Y;  // Y方向单位向量长度
    
    // 初始化网格参数
    static void initialize();
    static bool _initialized;
};

#endif
