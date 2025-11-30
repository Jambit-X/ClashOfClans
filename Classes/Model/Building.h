#ifndef __BUILDING_H__
#define __BUILDING_H__

#include "cocos2d.h"

class Building {
public:
  // 建筑类型枚举
  enum class Type {
    TOWN_HALL, // 大本营
    GOLD_MINE, // 金矿
    BARRACKS   // 兵营
  };

  Building(int id, Type type, int gridX, int gridY, int width, int height);

  // Getters
  int getId() const { return _id; }
  Type getType() const { return _type; }
  int getGridX() const { return _gridX; }
  int getGridY() const { return _gridY; }
  int getWidth() const { return _width; }
  int getHeight() const { return _height; }

  // Setters (用于移动建筑)
  void setGridPosition(int x, int y);

private:
  int _id;        // 唯一ID
  Type _type;     // 类型
  int _gridX;     // 网格 X 坐标
  int _gridY;     // 网格 Y 坐标
  int _width;     // 占地宽度 (格)
  int _height;    // 占地高度 (格)
};

#endif // __BUILDING_H__