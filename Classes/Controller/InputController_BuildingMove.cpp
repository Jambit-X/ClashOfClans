// 这个文件包含建筑移动功能的实现
// 将以下代码合并到 InputController.cpp 的末尾

#pragma region 建筑移动功能
void InputController::startBuildingMove(int buildingId) {
  if (_currentState == InputState::BUILDING_MOVING) {
    CCLOG("InputController: Already in BUILDING_MOVING state");
    return;
  }
  
  _movingBuildingId = buildingId;
  changeState(InputState::BUILDING_MOVING);
  
  CCLOG("InputController: Started moving building ID=%d", buildingId);
}

void InputController::cancelBuildingMove() {
  if (_currentState != InputState::BUILDING_MOVING) {
    return;
  }
  
  CCLOG("InputController: Cancelled moving building ID=%d", _movingBuildingId);
  
  _movingBuildingId = -1;
  changeState(InputState::MAP_DRAG);
}

void InputController::handleBuildingMoving(Touch* touch) {
  if (_movingBuildingId < 0) {
    CCLOG("InputController: No building to move!");
    return;
  }
  
  // 获取触摸点的世界坐标
  Vec2 touchPos = touch->getLocation();
  
  // 将屏幕坐标转换为 Layer 内的世界坐标
  Vec2 worldPos = _villageLayer->convertToNodeSpace(touchPos);
  
  // 如果有回调，调用它（让 BuildingManager 处理网格对齐和预览）
  if (_onBuildingMoveUpdate) {
    // 这里传入世界坐标，由 BuildingManager 转换为网格坐标
    _onBuildingMoveUpdate(_movingBuildingId, worldPos, Vec2::ZERO);
  }
}

void InputController::completeBuildingMove(const Vec2& dropPosition) {
  if (_movingBuildingId < 0) {
    return;
  }
  
  // 将屏幕坐标转换为 Layer 内的世界坐标
  Vec2 worldPos = _villageLayer->convertToNodeSpace(dropPosition);
  
  CCLOG("InputController: Completing building move at world(%.0f, %.0f)", 
        worldPos.x, worldPos.y);
  
  // 如果有回调，调用它（让 BuildingManager 检查合法性并完成放置）
  if (_onBuildingMoveComplete) {
    // BuildingManager 会负责：
    // 1. 转换为网格坐标
    // 2. 检查是否可以放置
    // 3. 更新建筑位置或恢复原位
    _onBuildingMoveComplete(_movingBuildingId, worldPos, true);
  }
  
  // 重置状态
  _movingBuildingId = -1;
  changeState(InputState::MAP_DRAG);
}
#pragma endregion
