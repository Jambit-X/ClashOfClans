#include "GridMapUtils.h"
#include <cmath>

USING_NS_CC;

// 静态成员变量初始化
float GridMapUtils::GRID_UNIT_X = 0.0f;
float GridMapUtils::GRID_UNIT_Y = 0.0f;
bool GridMapUtils::_initialized = false;

void GridMapUtils::initialize() {
    if (_initialized) return;
    
    // 计算等距网格的基向量
    // 从中心 (1893, 1370) 到右顶点 (3128, 1370) 是22个网格单位（半个44格宽度）
    // 从中心 (1893, 1370) 到上顶点 (1893, 2293) 是22个网格单位（半个44格高度）
    
    // 右方向基向量（每格在X轴上的分量）
    float rightVectorX = (RIGHT_VERTEX_X - CENTER_X) / 22.0f;  // (3128-1893)/22 = 56.136
    float rightVectorY = (RIGHT_VERTEX_Y - CENTER_Y) / 22.0f;  // (1370-1370)/22 = 0
    
    // 上方向基向量（每格在Y轴上的分量）
    float upVectorX = (TOP_VERTEX_X - CENTER_X) / 22.0f;      // (1893-1893)/22 = 0
    float upVectorY = (TOP_VERTEX_Y - CENTER_Y) / 22.0f;      // (2293-1370)/22 = 41.954
    
    GRID_UNIT_X = rightVectorX;  // 约 56.136 像素
    GRID_UNIT_Y = upVectorY;     // 约 41.954 像素
    
    _initialized = true;
    
    CCLOG("GridMapUtils initialized:");
    CCLOG("  Grid unit X: %.3f pixels", GRID_UNIT_X);
    CCLOG("  Grid unit Y: %.3f pixels", GRID_UNIT_Y);
    CCLOG("  Grid center: (%d, %d)", CENTER_X, CENTER_Y);
}

Vec2 GridMapUtils::pixelToGrid(const Vec2& pixelPos) {
    initialize();
    
    // 将像素坐标转换为相对于网格中心的坐标
    float relativeX = pixelPos.x - CENTER_X;
    float relativeY = pixelPos.y - CENTER_Y;
    
    // 等距网格转换公式
    // 在等距投影中：
    // pixelX = centerX + (gridX - gridY) * unitX
    // pixelY = centerY + (gridX + gridY) * unitY
    // 
    // 反向求解：
    // gridX = (relativeX / unitX + relativeY / unitY) / 2
    // gridY = (relativeY / unitY - relativeX / unitX) / 2
    
    float gridX = (relativeX / GRID_UNIT_X + relativeY / GRID_UNIT_Y) / 2.0f;
    float gridY = (relativeY / GRID_UNIT_Y - relativeX / GRID_UNIT_X) / 2.0f;
    
    // 网格原点 (0,0) 在左下角，中心是 (22, 22)
    gridX += 22.0f;
    gridY += 22.0f;
    
    return Vec2(gridX, gridY);
}

Vec2 GridMapUtils::gridToPixel(const Vec2& gridPos) {
    initialize();
    
    // 将网格坐标转换为相对于中心的坐标
    float relativeGridX = gridPos.x - 22.0f;
    float relativeGridY = gridPos.y - 22.0f;
    
    // 等距投影公式
    // pixelX = centerX + (gridX - gridY) * unitX
    // pixelY = centerY + (gridX + gridY) * unitY
    
    float pixelX = CENTER_X + (relativeGridX - relativeGridY) * GRID_UNIT_X;
    float pixelY = CENTER_Y + (relativeGridX + relativeGridY) * GRID_UNIT_Y;
    
    return Vec2(pixelX, pixelY);
}

Vec2 GridMapUtils::gridToPixelCenter(const Vec2& gridPos) {
    initialize();
    
    // 获取网格左下角的像素坐标
    Vec2 cornerPixel = gridToPixel(gridPos);
    
    // 移动到网格中心
    // 在等距网格中，中心偏移是 (0.5, 0.5) 个网格单位
    float centerOffsetX = 0.5f * GRID_UNIT_X - 0.5f * GRID_UNIT_X;  // = 0
    float centerOffsetY = 0.5f * GRID_UNIT_Y + 0.5f * GRID_UNIT_Y;  // = unitY
    
    return Vec2(cornerPixel.x, cornerPixel.y + GRID_UNIT_Y);
}

Vec2 GridMapUtils::snapToGrid(const Vec2& pixelPos) {
    Vec2 gridPos = pixelToGrid(pixelPos);
    
    // 四舍五入到最近的整数网格
    int gridX = static_cast<int>(std::round(gridPos.x));
    int gridY = static_cast<int>(std::round(gridPos.y));
    
    // 限制在有效范围内
    gridX = std::max(0, std::min(GRID_WIDTH - 1, gridX));
    gridY = std::max(0, std::min(GRID_HEIGHT - 1, gridY));
    
    return Vec2(static_cast<float>(gridX), static_cast<float>(gridY));
}

bool GridMapUtils::isValidGridPosition(int gridX, int gridY) {
    return gridX >= 0 && gridX < GRID_WIDTH && 
           gridY >= 0 && gridY < GRID_HEIGHT;
}

bool GridMapUtils::canPlaceBuilding(int gridX, int gridY, int width, int height) {
    // 检查建筑占用的所有网格是否都在有效范围内
    // 建筑从 (gridX, gridY) 到 (gridX + width - 1, gridY + height - 1)
    
    if (gridX < 0 || gridY < 0) {
        return false;
    }
    
    if (gridX + width > GRID_WIDTH || gridY + height > GRID_HEIGHT) {
        return false;
    }
    
    return true;
}
