#include "Building.h"

Building::Building(int id, Type type, int gridX, int gridY, int width, int height)
  : _id(id), _type(type), _gridX(gridX), _gridY(gridY), _width(width), _height(height) {}

void Building::setGridPosition(int x, int y) {
  _gridX = x;
  _gridY = y;
}