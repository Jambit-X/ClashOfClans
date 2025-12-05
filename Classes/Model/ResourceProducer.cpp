// Model/Building.cpp
#include "Building.h"

Building::Building(int id, Type type, int width, int height)
    : _id(id)
    , _type(type)
    , _width(width)
    , _height(height)
    , _gridX(0)             // 默认初始坐标为 0,0
    , _gridY(0)
    , _state(State::PREVIEW) // 默认状态为"预览中/拖拽中"
{
    // 构造函数体为空，因为初始化列表已经完成了所有工作
}

// 静态工厂方法实现
Building* Building::createTownHall(int id, int gridX, int gridY) {
    auto* building = new Building(id, Type::TOWN_HALL, 3, 3);
    building->setGridPosition(gridX, gridY);
    building->setState(State::PLACED);
    return building;
}

Building* Building::createCannon(int id, int gridX, int gridY) {
    auto* building = new Building(id, Type::CANNON, 3, 3);
    building->setGridPosition(gridX, gridY);
    building->setState(State::PLACED);
    return building;
}

Building* Building::createGoldMine(int id, int gridX, int gridY) {
    auto* building = new Building(id, Type::GOLD_MINE, 3, 3);
    building->setGridPosition(gridX, gridY);
    building->setState(State::PLACED);
    return building;
}

Building* Building::createBarracks(int id, int gridX, int gridY) {
    auto* building = new Building(id, Type::BARRACKS, 3, 3);
    building->setGridPosition(gridX, gridY);
    building->setState(State::PLACED);
    return building;
}