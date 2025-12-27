// BuildingPlacementController.h
// 建筑放置控制器声明，管理建筑的放置流程和位置验证

#pragma once
#include "cocos2d.h"
#include <functional>

class BuildingSprite;

// 建筑放置控制器类
class BuildingPlacementController {
public:
  BuildingPlacementController();
  ~BuildingPlacementController();

  // 开始放置建筑
  void startPlacement(int buildingId);

  // 取消放置
  void cancelPlacement();

  // 确认放置
  bool confirmPlacement();

  // 检查当前位置是否可放置
  bool canPlaceAtCurrentPosition() const;

  // 放置回调函数类型
  using PlacementCallback = std::function<void(bool success, int buildingId)>;
  
  // 设置放置完成回调
  void setPlacementCallback(PlacementCallback callback);

  // 获取当前放置的建筑ID
  int getCurrentBuildingId() const { return _currentBuildingId; }

  // 是否正在放置中
  bool isPlacing() const { return _isPlacing; }

private:
  int _currentBuildingId;       // 当前放置的建筑ID
  bool _isPlacing;              // 是否处于放置状态
  PlacementCallback _placementCallback;  // 放置完成回调
};
