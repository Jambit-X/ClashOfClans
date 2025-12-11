#pragma once
#include "cocos2d.h"

class BuildingSprite;

// 建造动画组件 - 显示"建造中..."动画
class ConstructionAnimation {
public:
  ConstructionAnimation(BuildingSprite* targetSprite);
  ~ConstructionAnimation();

  // 开始/停止动画
  void start();
  void stop();

  // 更新进度（0.0 - 1.0）
  void updateProgress(float progress);

private:
  void createAnimationLabel();
  void updateDots(float dt);

  BuildingSprite* _targetSprite;
  cocos2d::Label* _progressLabel;
  int _dotCount;
  bool _isRunning;
};