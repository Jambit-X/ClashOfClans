#pragma once
#include "cocos2d.h"
#include <functional>

class BuildingSprite;
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

  // 设置回调
  using PlacementCallback = std::function<void(bool success, int buildingId)>;
  void setPlacementCallback(PlacementCallback callback);

  // 获取当前放置的建筑ID
  int getCurrentBuildingId() const { return _currentBuildingId; }

  // 是否正在放置中
  bool isPlacing() const { return _isPlacing; }

private:
  int _currentBuildingId;
  bool _isPlacing;
  PlacementCallback _placementCallback;
};