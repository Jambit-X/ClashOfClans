#ifndef __GRID_MAP_H__
#define __GRID_MAP_H__

#include "cocos2d.h"

class GridMap {
public:
  // 修改为等距投影的尺寸
  static const int TILE_WIDTH = 80;   // 菱形格子的像素宽度
  static const int TILE_HEIGHT = 40;  // 菱形格子的像素高度

  // 地图尺寸保持不变 (例如 50x50 格)
  static const int MAP_WIDTH = 50;
  static const int MAP_HEIGHT = 50;

  // --- 核心修改的函数签名 ---
  // 将网格坐标 (GridX, GridY) 转换为 屏幕像素坐标 (Vec2)
  static cocos2d::Vec2 gridToPixel(int gridX, int gridY);

  // 将 屏幕像素坐标 转换为 网格坐标 (反向转换更复杂，我们先实现正向)
  static cocos2d::Vec2 pixelToGrid(cocos2d::Vec2 pixelPos);

  // 获取地图左下角（0, 0）网格在屏幕上的起始像素点
  static cocos2d::Vec2 getMapOriginOffset();
};

#endif // __GRID_MAP_H__