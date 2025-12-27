// ConstructionAnimation.h
// 建筑建造动画组件声明，管理建造过程中的视觉反馈

#pragma once
#include "cocos2d.h"

class BuildingSprite;

// 建筑建造动画管理类
class ConstructionAnimation {
public:
  ConstructionAnimation(BuildingSprite* sprite);
  ~ConstructionAnimation();

  // 启动建造动画
  void start();
  
  // 停止建造动画
  void stop();
  
  // 更新建造进度 (0.0-1.0)
  void updateProgress(float progress);

private:
  BuildingSprite* _buildingSprite;  // 关联的建筑精灵
  bool _isRunning;                  // 动画是否正在运行
  cocos2d::Label* _progressLabel;   // 进度显示标签
};
