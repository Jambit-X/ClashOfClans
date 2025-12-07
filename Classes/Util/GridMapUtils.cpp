#include "GridMapUtils.h"
#include <cmath>

USING_NS_CC;

// ========== 网格坐标系统参数（基于测试数据计算） ==========

// 网格四个顶点（测试数据）：
// - 中心：(1893, 1370)
// - 左顶点：(660, 1365) - 网格原点 (0, 0)
// - 右顶点：(3128, 1365) - 网格 (44, 44)
// - 上顶点：(1893, 2293) - 网格 (0, 44)
// - 下顶点：(1893, 445) - 网格 (44, 0)
// - 网格大小：44 x 44，范围 (0,0) 到 (43,43)

// 网格原点：左顶点，对应网格坐标(0,0)
const float GridMapUtils::GRID_ORIGIN_X = 660.0f;
const float GridMapUtils::GRID_ORIGIN_Y = 1365.0f;

// X轴单位向量：每移动1格沿X轴的像素偏移（从左顶点到下顶点）
// 计算：(下顶点 - 左顶点) / 44 = (1893-660, 445-1365) / 44
const float GridMapUtils::GRID_X_UNIT_X = 28.02f;   // (1893-660)/44 = 1233/44
const float GridMapUtils::GRID_X_UNIT_Y = -20.91f;  // (445-1365)/44 = -920/44

// Y轴单位向量：每移动1格沿Y轴的像素偏移（从左顶点到上顶点）
// 计算：(上顶点 - 左顶点) / 44 = (1893-660, 2293-1365) / 44
const float GridMapUtils::GRID_Y_UNIT_X = 28.02f;   // (1893-660)/44 = 1233/44
const float GridMapUtils::GRID_Y_UNIT_Y = 21.09f;   // (2293-1365)/44 = 928/44

// ========== 坐标转换实现 ==========

// 像素坐标 -> 网格坐标
Vec2 GridMapUtils::pixelToGrid(float pixelX, float pixelY) {
    // 1. 计算相对于原点的偏移
    float deltaX = pixelX - GRID_ORIGIN_X;
    float deltaY = pixelY - GRID_ORIGIN_Y;

    // 2. 仿射逆变换
    // 已知：pixel = origin + gridX * xUnit + gridY * yUnit
    // 求解：gridX 和 gridY
    // 
    // deltaX = gridX * GRID_X_UNIT_X + gridY * GRID_Y_UNIT_X
    // deltaY = gridX * GRID_X_UNIT_Y + gridY * GRID_Y_UNIT_Y
    //
    // 使用2x2矩阵求逆（克拉默法则）：
    // | GRID_X_UNIT_X  GRID_Y_UNIT_X |   | gridX |   | deltaX |
    // | GRID_X_UNIT_Y  GRID_Y_UNIT_Y | * | gridY | = | deltaY |
    
    float det = GRID_X_UNIT_X * GRID_Y_UNIT_Y - GRID_X_UNIT_Y * GRID_Y_UNIT_X;
    
    if (std::abs(det) < 0.0001f) {
        // 行列式为0，矩阵不可逆（理论上不应该发生）
        CCLOG("GridMapUtils::pixelToGrid - Matrix is singular!");
        return Vec2(0, 0);
    }
    
    // 逆矩阵计算
    float gridX = (deltaX * GRID_Y_UNIT_Y - deltaY * GRID_Y_UNIT_X) / det;
    float gridY = (deltaY * GRID_X_UNIT_X - deltaX * GRID_X_UNIT_Y) / det;

    return Vec2(gridX, gridY);
}

Vec2 GridMapUtils::pixelToGrid(const Vec2& pixelPos) {
    return pixelToGrid(pixelPos.x, pixelPos.y);
}

// 网格坐标 -> 像素坐标（网格元素左下角）
Vec2 GridMapUtils::gridToPixel(int gridX, int gridY) {
    // 仿射矩阵正变换：pixel = origin + gridX * xUnit + gridY * yUnit
    float pixelX = GRID_ORIGIN_X + gridX * GRID_X_UNIT_X + gridY * GRID_Y_UNIT_X;
    float pixelY = GRID_ORIGIN_Y + gridX * GRID_X_UNIT_Y + gridY * GRID_Y_UNIT_Y;

    return Vec2(pixelX, pixelY);
}

Vec2 GridMapUtils::gridToPixel(const Vec2& gridPos) {
    return gridToPixel((int)gridPos.x, (int)gridPos.y);
}

// 网格坐标 -> 像素坐标（网格元素中心）
Vec2 GridMapUtils::gridToPixelCenter(int gridX, int gridY) {
    // 先获取左下角坐标
    Vec2 corner = gridToPixel(gridX, gridY);

    // 加上半个单元格的偏移，到达中心
    float centerX = corner.x + (GRID_X_UNIT_X + GRID_Y_UNIT_X) * 0.5f;
    float centerY = corner.y + (GRID_X_UNIT_Y + GRID_Y_UNIT_Y) * 0.5f;

    return Vec2(centerX, centerY);
}

Vec2 GridMapUtils::gridToPixelCenter(const Vec2& gridPos) {
    return gridToPixelCenter((int)gridPos.x, (int)gridPos.y);
}

// ========== 建筑位置计算 ==========

Vec2 GridMapUtils::getBuildingCenterPixel(int gridX, int gridY, int width, int height) {
    // 建筑的左下角： (gridX, gridY)
    // 建筑的中心： (gridX + width/2, gridY + height/2) 在网格坐标系
    // 注意：比如 3x3 建筑，中心在 (gridX+1.5, gridY+1.5)

    float centerGridX = gridX + width * 0.5f;
    float centerGridY = gridY + height * 0.5f;

    // 转换为像素坐标
    float pixelX = GRID_ORIGIN_X + centerGridX * GRID_X_UNIT_X + centerGridY * GRID_Y_UNIT_X;
    float pixelY = GRID_ORIGIN_Y + centerGridX * GRID_X_UNIT_Y + centerGridY * GRID_Y_UNIT_Y;

    return Vec2(pixelX, pixelY);
}

// ========== 边界检测 ==========

bool GridMapUtils::isValidGridPosition(int gridX, int gridY) {
    return gridX >= 0 && gridX < GRID_WIDTH &&
           gridY >= 0 && gridY < GRID_HEIGHT;
}

bool GridMapUtils::isValidGridPosition(const Vec2& gridPos) {
    return isValidGridPosition((int)gridPos.x, (int)gridPos.y);
}

bool GridMapUtils::isBuildingInBounds(int gridX, int gridY, int width, int height) {
    // 检查左下角和右上角是否都在范围内
    return isValidGridPosition(gridX, gridY) && 
           isValidGridPosition(gridX + width - 1, gridY + height - 1);
}

bool GridMapUtils::isRectOverlap(
    const Vec2& pos1, const Size& size1,
    const Vec2& pos2, const Size& size2) {
    
    // 如果一个矩形在另一个矩形的左边、右边、上边或下边，则不重叠
    return !(pos1.x + size1.width <= pos2.x ||   // rect1 在 rect2 左边
             pos2.x + size2.width <= pos1.x ||   // rect1 在 rect2 右边
             pos1.y + size1.height <= pos2.y ||  // rect1 在 rect2 下边
             pos2.y + size2.height <= pos1.y);   // rect1 在 rect2 上边
}

Vec2 GridMapUtils::getVisualPosition(int gridX, int gridY, const Vec2& visualOffset) {
    // 1. 网格坐标转世界坐标
    Vec2 worldPos = gridToPixel(gridX, gridY);
    
    // 2. 应用视觉偏移量
    worldPos += visualOffset;
    
    return worldPos;
}
