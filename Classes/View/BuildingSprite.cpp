#include "BuildingSprite.h"
#include "../System/GridMap.h" // 引用网格计算工具

USING_NS_CC;

BuildingSprite* BuildingSprite::createWithBuilding(Building* building) {
  BuildingSprite* sprite = new (std::nothrow) BuildingSprite();
  if (sprite && sprite->initWithBuilding(building)) {
    sprite->autorelease();
    return sprite;
  }
  CC_SAFE_DELETE(sprite);
  return nullptr;
}

bool BuildingSprite::initWithBuilding(Building* building) {
  // 为了防止没有图片导致崩溃，我们暂时用纯色方块代替
  // 实际项目中这里应该是 Sprite::initWithFile("town_hall.png");
  if (!Sprite::init()) {
    return false;
  }

  _buildingData = building;

  // 1. 设置外观 (临时用颜色块)
  // 等距投影下，建筑占地 WxH 个格子。
  // 像素宽度主要由 TILE_WIDTH 决定，像素高度主要由 TILE_HEIGHT 决定。
  // 为了简化 MVP，我们让 Sprite 的像素尺寸是建筑占地格子数 * 单个格子像素尺寸
  float pixelWidth = building->getWidth() * GridMap::TILE_WIDTH;
  float pixelHeight = building->getHeight() * GridMap::TILE_HEIGHT;

  // 实际 CoC 效果中，建筑 Sprite 的像素高度要更高 (为了容纳房顶)。
  // 这里暂时使用 TILE_HEIGHT，但为了让它看起来大一些，我们乘以一个系数。
  // 假设建筑的高度是它占地格子高度的 2 倍 (容纳屋顶)
  float displayHeight = building->getHeight() * GridMap::TILE_HEIGHT * 2;

  // 设置纹理区域 (创建一个纯色矩形)
  this->setTextureRect(Rect(0, 0, pixelWidth, displayHeight)); // 使用修正后的高度

  // 设置纹理区域 (创建一个纯色矩形)
  this->setTextureRect(Rect(0, 0, pixelWidth, pixelHeight));

  // 根据类型设置不同颜色
  switch (building->getType()) {
    case Building::Type::TOWN_HALL: this->setColor(Color3B::BLUE); break;
    case Building::Type::GOLD_MINE: this->setColor(Color3B::YELLOW); break;
    default: this->setColor(Color3B::GRAY); break;
  }

  // 2. 设置锚点 (关键修改)
  // 在等距投影中，Sprite 的底部中点应该与网格位置对齐
  this->setAnchorPoint(Vec2(0.5f, 0.0f)); // 锚点设为底部中心

  // 3. 关键修改：旋转 Sprite 以模拟底座的等距角度
  // 我们将其旋转 45 度，使矩形看起来像一个斜放的底座
  this->setRotation(45.0f); // <-- 新增的旋转角度

  // 4. 设置透明度
  this->setOpacity(200);

  // 5. 更新位置
  updatePositionFromGrid();

  return true;
}

void BuildingSprite::updatePositionFromGrid() {
  if (_buildingData) {
    // 调用我们写的工具类，把 (GridX, GridY) 转成 (PixelX, PixelY)
    Vec2 pos = GridMap::gridToPixel(_buildingData->getGridX(), _buildingData->getGridY());
    this->setPosition(pos);
  }
}