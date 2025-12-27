// GridMapUtils.cpp
// 网格地图工具实现，提供等轴测坐标系统的精确转换和计算

#include "GridMapUtils.h"
#include <cmath>

USING_NS_CC;

// ===================================================================================
// 网格坐标系统参数定义（基于实测数据精确计算）
// ===================================================================================

/**
 * 网格四个顶点实测数据：
 * - 中心点：(1893, 1370)
 * - 左顶点：(660, 1365)  ← 选作网格原点，对应(0,0)
 * - 右顶点：(3128, 1365) ← 对应(44,44)，菱形右端点
 * - 上顶点：(1893, 2293) ← 对应(0,44)，菱形上端点
 * - 下顶点：(1893, 445)  ← 对应(44,0)，菱形下端点
 * 
 * 网格规格：
 * - 总格数：44 x 44
 * - 有效范围：gridX, gridY ∈ [0, 43]
 */

// 网格原点：选择左顶点作为坐标系原点
const float GridMapUtils::GRID_ORIGIN_X = 660.0f;
const float GridMapUtils::GRID_ORIGIN_Y = 1365.0f;

/**
 * X轴单位向量计算：
 * 从左顶点到下顶点（纯X方向移动44格）
 * 计算公式：(下顶点 - 左顶点) / 44
 * 结果：((1893-660)/44, (445-1365)/44) = (1233/44, -920/44)
 */
const float GridMapUtils::GRID_X_UNIT_X = 28.02f;   // 水平分量：向右
const float GridMapUtils::GRID_X_UNIT_Y = -20.91f;  // 垂直分量：向下

/**
 * Y轴单位向量计算：
 * 从左顶点到上顶点（纯Y方向移动44格）
 * 计算公式：(上顶点 - 左顶点) / 44
 * 结果：((1893-660)/44, (2293-1365)/44) = (1233/44, 928/44)
 */
const float GridMapUtils::GRID_Y_UNIT_X = 28.02f;   // 水平分量：向右
const float GridMapUtils::GRID_Y_UNIT_Y = 21.09f;   // 垂直分量：向上

// ===================================================================================
// 坐标转换实现：世界坐标 ↔ 网格坐标
// ===================================================================================

/**
 * @brief 世界坐标转网格坐标（逆仿射变换）
 * 
 * 数学原理：
 * 已知仿射变换：pixel = origin + gridX * xUnit + gridY * yUnit
 * 
 * 展开：
 * pixelX = ORIGIN_X + gridX * X_UNIT_X + gridY * Y_UNIT_X
 * pixelY = ORIGIN_Y + gridX * X_UNIT_Y + gridY * Y_UNIT_Y
 * 
 * 定义偏移量：
 * deltaX = pixelX - ORIGIN_X
 * deltaY = pixelY - ORIGIN_Y
 * 
 * 写成矩阵形式：
 * | X_UNIT_X  Y_UNIT_X |   | gridX |   | deltaX |
 * | X_UNIT_Y  Y_UNIT_Y | * | gridY | = | deltaY |
 * 
 * 求解gridX和gridY需要计算逆矩阵。使用克拉默法则：
 * 
 * det = X_UNIT_X * Y_UNIT_Y - X_UNIT_Y * Y_UNIT_X
 * gridX = (deltaX * Y_UNIT_Y - deltaY * Y_UNIT_X) / det
 * gridY = (deltaY * X_UNIT_X - deltaX * X_UNIT_Y) / det
 * 
 * 实测数据验证：
 * - 左顶点(660,1365) → (0,0) ✓
 * - 下顶点(1893,445) → (44,0) ✓
 * - 上顶点(1893,2293) → (0,44) ✓
 */
Vec2 GridMapUtils::pixelToGrid(float pixelX, float pixelY) {
    // 步骤1：计算相对于原点的偏移
    float deltaX = pixelX - GRID_ORIGIN_X;
    float deltaY = pixelY - GRID_ORIGIN_Y;

    // 步骤2：计算2x2矩阵的行列式
    float det = GRID_X_UNIT_X * GRID_Y_UNIT_Y - GRID_X_UNIT_Y * GRID_Y_UNIT_X;
    
    if (std::abs(det) < 0.0001f) {
        // 行列式接近0，矩阵奇异（理论上不应该发生）
        CCLOG("GridMapUtils::pixelToGrid - Warning: Matrix is singular! det=%f", det);
        return Vec2(0, 0);
    }
    
    // 步骤3：应用克拉默法则求解
    float gridX = (deltaX * GRID_Y_UNIT_Y - deltaY * GRID_Y_UNIT_X) / det;
    float gridY = (deltaY * GRID_X_UNIT_X - deltaX * GRID_X_UNIT_Y) / det;

    return Vec2(gridX, gridY);
}

Vec2 GridMapUtils::pixelToGrid(const Vec2& pixelPos) {
    return pixelToGrid(pixelPos.x, pixelPos.y);
}

/**
 * @brief 网格坐标转世界坐标（正向仿射变换）
 * 
 * 数学原理：
 * pixel = origin + gridX * xUnit + gridY * yUnit
 * 
 * 展开：
 * pixelX = ORIGIN_X + gridX * X_UNIT_X + gridY * Y_UNIT_X
 * pixelY = ORIGIN_Y + gridX * X_UNIT_Y + gridY * Y_UNIT_Y
 * 
 * 说明：返回的是网格单元的左下角坐标
 */
Vec2 GridMapUtils::gridToPixel(int gridX, int gridY) {
    float pixelX = GRID_ORIGIN_X + gridX * GRID_X_UNIT_X + gridY * GRID_Y_UNIT_X;
    float pixelY = GRID_ORIGIN_Y + gridX * GRID_X_UNIT_Y + gridY * GRID_Y_UNIT_Y;

    return Vec2(pixelX, pixelY);
}

Vec2 GridMapUtils::gridToPixel(const Vec2& gridPos) {
    return gridToPixel((int)gridPos.x, (int)gridPos.y);
}

/**
 * @brief 网格坐标转世界坐标（网格中心）
 * 
 * 实现方式：
 * 1. 先获取网格左下角坐标
 * 2. 加上半个单元格的偏移量
 * 
 * 半个单元格 = (xUnit + yUnit) * 0.5
 * 
 * 几何意义：
 * 菱形网格的中心点位于两条对角线的交点
 */
Vec2 GridMapUtils::gridToPixelCenter(int gridX, int gridY) {
    // 获取左下角坐标
    Vec2 corner = gridToPixel(gridX, gridY);

    // 加上半个单元格的偏移
    float centerX = corner.x + (GRID_X_UNIT_X + GRID_Y_UNIT_X) * 0.5f;
    float centerY = corner.y + (GRID_X_UNIT_Y + GRID_Y_UNIT_Y) * 0.5f;

    return Vec2(centerX, centerY);
}

Vec2 GridMapUtils::gridToPixelCenter(const Vec2& gridPos) {
    return gridToPixelCenter((int)gridPos.x, (int)gridPos.y);
}

// ===================================================================================
// 建筑位置计算
// ===================================================================================

/**
 * @brief 计算建筑中心的世界坐标
 * 
 * 算法步骤：
 * 1. 计算建筑中心在网格坐标系中的位置
 *    centerGridX = gridX + width / 2
 *    centerGridY = gridY + height / 2
 * 
 * 2. 应用仿射变换转换为世界坐标
 *    pixelX = ORIGIN_X + centerGridX * X_UNIT_X + centerGridY * Y_UNIT_X
 *    pixelY = ORIGIN_Y + centerGridX * X_UNIT_Y + centerGridY * Y_UNIT_Y
 * 
 * 示例：
 * - 1x1建筑在(10,10)，中心在(10.5, 10.5)
 * - 3x3建筑在(10,10)，中心在(11.5, 11.5)
 * - 4x4建筑在(20,20)，中心在(22, 22)
 */
Vec2 GridMapUtils::getBuildingCenterPixel(int gridX, int gridY, int width, int height) {
    // 计算建筑中心的网格坐标（可能是小数）
    float centerGridX = gridX + width * 0.5f;
    float centerGridY = gridY + height * 0.5f;

    // 转换为世界坐标
    float pixelX = GRID_ORIGIN_X + centerGridX * GRID_X_UNIT_X + centerGridY * GRID_Y_UNIT_X;
    float pixelY = GRID_ORIGIN_Y + centerGridX * GRID_X_UNIT_Y + centerGridY * GRID_Y_UNIT_Y;

    return Vec2(pixelX, pixelY);
}

// ===================================================================================
// 边界检测实现
// ===================================================================================

/**
 * @brief 检查单个网格坐标是否有效
 * 
 * 有效范围：gridX, gridY ∈ [0, 43]
 */
bool GridMapUtils::isValidGridPosition(int gridX, int gridY) {
    return gridX >= 0 && gridX < GRID_WIDTH &&
           gridY >= 0 && gridY < GRID_HEIGHT;
}

bool GridMapUtils::isValidGridPosition(const Vec2& gridPos) {
    return isValidGridPosition((int)gridPos.x, (int)gridPos.y);
}

/**
 * @brief 检查建筑是否完全在边界内
 * 
 * 检查逻辑：
 * 1. 左下角：(gridX, gridY) 必须有效
 * 2. 右上角：(gridX + width - 1, gridY + height - 1) 必须有效
 * 
 * 示例：
 * - 3x3建筑在(0,0)：占用(0,0)到(2,2)，合法
 * - 3x3建筑在(42,42)：占用(42,42)到(44,44)，非法（超出43）
 */
bool GridMapUtils::isBuildingInBounds(int gridX, int gridY, int width, int height) {
    return isValidGridPosition(gridX, gridY) && 
           isValidGridPosition(gridX + width - 1, gridY + height - 1);
}

/**
 * @brief 矩形重叠检测（AABB算法）
 * 
 * 不重叠的四种情况：
 * 1. rect1在rect2左边：rect1.right <= rect2.left
 * 2. rect1在rect2右边：rect2.right <= rect1.left
 * 3. rect1在rect2下边：rect1.top <= rect2.bottom
 * 4. rect1在rect2上边：rect2.top <= rect1.bottom
 * 
 * 重叠 = 不满足上述任何一种情况
 */
bool GridMapUtils::isRectOverlap(
    const Vec2& pos1, const Size& size1,
    const Vec2& pos2, const Size& size2) {
    
    return !(pos1.x + size1.width <= pos2.x ||   // rect1在rect2左边
             pos2.x + size2.width <= pos1.x ||   // rect1在rect2右边
             pos1.y + size1.height <= pos2.y ||  // rect1在rect2下边
             pos2.y + size2.height <= pos1.y);   // rect1在rect2上边
}

/**
 * @brief 计算建筑的最终渲染位置
 * 
 * 流程：
 * 1. 将网格坐标转换为世界坐标
 * 2. 应用视觉偏移量（用于微调显示位置）
 * 
 * 应用场景：
 * 某些建筑（如塔楼）需要额外偏移，使其视觉上更居中或对齐
 */
Vec2 GridMapUtils::getVisualPosition(int gridX, int gridY, const Vec2& visualOffset) {
    // 步骤1：网格坐标转世界坐标
    Vec2 worldPos = gridToPixel(gridX, gridY);
    
    // 步骤2：应用视觉偏移
    worldPos += visualOffset;
    
    return worldPos;
}
