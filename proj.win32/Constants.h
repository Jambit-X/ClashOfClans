#ifndef _CONSTANT_H_
#define _CONSTANT_H_
#include "cocos2d.h"

#define COCOS2D_DEBUG 1

const float VILLAGE_IMAGE_WIDTH = 3705.0;
const float VILLAGE_IMAGE_HEIGHT = 2545.0;

const cocos2d::Vec2 GRID_LEFT_POINT = cocos2d::Vec2(660.0f, 1371.0f); // 网格左顶点坐标
const cocos2d::Vec2 GRID_RIGHT_POINT = cocos2d::Vec2(3128.0f, 1371.0f); // 网格右顶点坐标
const cocos2d::Vec2 GRID_TOP_POINT = cocos2d::Vec2(1893.0f, 2293.0f); // 网格上顶点坐标
const cocos2d::Vec2 GRID_BOTTOM_POINT = cocos2d::Vec2(1893.0f, 445.0f); // 网格下顶点坐标

const float VERT_DIAGONAL_LENGTH = (2293.0f - 445.0f) / 44.0f; // 菱形竖直对角线
const float HORIZ_DIAGONAL_LENGTH = (3128.0f - 660.0f) / 44.0f; // 菱形横向对角线

const cocos2d::Vec2 VEC_1_NORMALIZED = (GRID_TOP_POINT - GRID_LEFT_POINT) / 44.0f; // 网格右上方向一格向量
const cocos2d::Vec2 VEC_2_NORMALIZED = (GRID_BOTTOM_POINT - GRID_LEFT_POINT) / 44.0f; // 网格右下方向一格向量

const float GRID_LENGTH = VEC_1_NORMALIZED.getLength(); // 单格网格边长

#endif // !_CONSTANT_H_
