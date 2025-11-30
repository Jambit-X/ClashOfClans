#include "GridMap.h"

USING_NS_CC;

// CoC 风格的等距投影需要一个起始偏移量，以保证地图中心在屏幕中央
Vec2 GridMap::getMapOriginOffset() {
  // 假设地图中心位于屏幕中央 (Vision::width / 2)
  // 地图总宽度: (MAP_WIDTH + MAP_HEIGHT) * TILE_WIDTH / 2

  // 为了简单，我们只计算 X 轴偏移，让网格 (0, 0) 的点显示在屏幕左下角
  // 实际项目中，你需要根据整个地图的总尺寸来居中
  return Vec2(MAP_HEIGHT * TILE_WIDTH / 2.0f, 0.0f);
}


// --- 核心等距转换公式实现 ---
Vec2 GridMap::gridToPixel(int gridX, int gridY) {
  // 获取地图原点偏移
  Vec2 offset = getMapOriginOffset();

  // 1. 等距计算：
  // X 坐标：(X - Y) * (TILE_WIDTH / 2)
  float px = (float)(gridX - gridY) * (TILE_WIDTH / 2.0f);

  // Y 坐标：(X + Y) * (TILE_HEIGHT / 2)
  float py = (float)(gridX + gridY) * (TILE_HEIGHT / 2.0f);

  // 2. 加上起始偏移
  px += offset.x;
  py += offset.y;

  // 注意：我们返回的是建筑的底部中心点，而不是 Sprite 的中心点
  // Sprite 的定位锚点（Anchor Point）需要设置在底部中央 (0.5, 0)

  return Vec2(px, py);
}


// --- 反向转换：像素转网格 (更复杂，但必须实现) ---
Vec2 GridMap::pixelToGrid(Vec2 pixelPos) {
  Vec2 offset = getMapOriginOffset();

  // 1. 减去起始偏移
  float px = pixelPos.x - offset.x;
  float py = pixelPos.y - offset.y;

  // 2. 将等距坐标系统旋转回正交系统
  // GridX = (Px / (W/2) + Py / (H/2)) / 2
  // GridY = (Py / (H/2) - Px / (W/2)) / 2

  float ratioX = px / (TILE_WIDTH / 2.0f);
  float ratioY = py / (TILE_HEIGHT / 2.0f);

  float gx = (ratioX + ratioY) / 2.0f;
  float gy = (ratioY - ratioX) / 2.0f;

  // 3. 四舍五入到最近的格子，并转换为 int
  int gridX = (int)round(gx);
  int gridY = (int)round(gy);

  // 边界检查
  if (gridX < 0) gridX = 0;
  if (gridY < 0) gridY = 0;
  if (gridX >= MAP_WIDTH) gridX = MAP_WIDTH - 1;
  if (gridY >= MAP_HEIGHT) gridY = MAP_HEIGHT - 1;

  return Vec2(gridX, gridY);
}