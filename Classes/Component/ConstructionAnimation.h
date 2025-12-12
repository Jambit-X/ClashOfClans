#pragma once
#include "cocos2d.h"

class BuildingSprite;

class ConstructionAnimation {
public:
  ConstructionAnimation(BuildingSprite* sprite);
  ~ConstructionAnimation();

  void start();
  void stop();
  void updateProgress(float progress);

private:
  BuildingSprite* _buildingSprite;  // 统一使用 _buildingSprite
  bool _isRunning;
  cocos2d::Label* _progressLabel;
};