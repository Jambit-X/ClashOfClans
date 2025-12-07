# InputController 建筑移动功能使用指南

## ?? 功能概述

建筑移动功能允许用户拖动已放置的建筑到新位置。

## ?? 完整流程

```
用户长按建筑
    ↓
InputController::startBuildingMove(buildingId) 被调用
    ↓
进入 BUILDING_MOVING 状态
    ↓
用户拖动（触发 handleBuildingMoving）
    ↓
实时回调 _onBuildingMoveUpdate
    ↓
BuildingManager 更新建筑预览位置
    ↓
用户松手（触发 completeBuildingMove）
    ↓
回调 _onBuildingMoveComplete
    ↓
BuildingManager 检查合法性并完成放置
    ↓
返回 MAP_DRAG 状态
```

## ?? 使用示例

### 1. 在 VillageLayer 中设置回调

```cpp
void VillageLayer::init() {
    // ...existing code...
    
    // 设置建筑移动更新回调
    _inputController->setOnBuildingMoveUpdateCallback(
        [this](int buildingId, const Vec2& worldPos, const Vec2& gridPos) {
            // 转换为网格坐标
            Vec2 grid = GridMapUtils::pixelToGrid(worldPos);
            
            // 更新建筑精灵位置（实时预览）
            _buildingManager->updateBuildingPreviewPosition(buildingId, grid);
        }
    );
    
    // 设置建筑移动完成回调
    _inputController->setOnBuildingMoveCompleteCallback(
        [this](int buildingId, const Vec2& worldPos, bool isValid) {
            // 转换为网格坐标
            Vec2 grid = GridMapUtils::pixelToGrid(worldPos);
            
            // 检查是否可以放置
            bool canPlace = _buildingManager->canPlaceBuildingAt(buildingId, grid);
            
            if (canPlace) {
                // 完成移动
                _buildingManager->finalizeBuildingMove(buildingId, grid);
                CCLOG("Building moved successfully to grid(%.0f, %.0f)", grid.x, grid.y);
            } else {
                // 恢复原位
                _buildingManager->cancelBuildingMove(buildingId);
                CCLOG("Invalid position, building returned to original location");
            }
        }
    );
}
```

### 2. 触发建筑移动

```cpp
// 方法 A：从建筑选中菜单触发
void VillageLayer::onBuildingMenuMoveClicked(int buildingId) {
    _inputController->startBuildingMove(buildingId);
}

// 方法 B：长按建筑触发
void VillageLayer::onBuildingLongPress(int buildingId) {
    _inputController->startBuildingMove(buildingId);
}
```

### 3. 在 BuildingManager 中实现辅助方法

```cpp
// BuildingManager.h
class BuildingManager {
public:
    // 更新建筑预览位置（拖动时）
    void updateBuildingPreviewPosition(int buildingId, const Vec2& gridPos);
    
    // 检查建筑是否可以放置在指定位置
    bool canPlaceBuildingAt(int buildingId, const Vec2& gridPos);
    
    // 完成建筑移动
    void finalizeBuildingMove(int buildingId, const Vec2& gridPos);
    
    // 取消建筑移动（恢复原位）
    void cancelBuildingMove(int buildingId);
    
private:
    std::map<int, Vec2> _originalBuildingPositions;  // 存储原始位置
};

// BuildingManager.cpp
void BuildingManager::updateBuildingPreviewPosition(int buildingId, const Vec2& gridPos) {
    auto sprite = getBuildingSprite(buildingId);
    if (!sprite) return;
    
    // 转换为世界坐标
    Vec2 worldPos = GridMapUtils::gridToPixel((int)gridPos.x, (int)gridPos.y);
    sprite->setPosition(worldPos);
    
    // 检查合法性，改变颜色
    bool isValid = canPlaceBuildingAt(buildingId, gridPos);
    sprite->setPlacementPreview(isValid);
}

bool BuildingManager::canPlaceBuildingAt(int buildingId, const Vec2& gridPos) {
    auto sprite = getBuildingSprite(buildingId);
    if (!sprite) return false;
    
    Size gridSize = sprite->getGridSize();
    
    // 检查边界
    if (!GridMapUtils::isBuildingInBounds(
        (int)gridPos.x, (int)gridPos.y, 
        (int)gridSize.width, (int)gridSize.height)) {
        return false;
    }
    
    // 检查是否与其他建筑重叠
    // TODO: 实现碰撞检测
    
    return true;
}

void BuildingManager::finalizeBuildingMove(int buildingId, const Vec2& gridPos) {
    // 更新数据层
    auto dataManager = VillageDataManager::getInstance();
    dataManager->setBuildingPosition(buildingId, (int)gridPos.x, (int)gridPos.y);
    
    // 更新精灵
    auto sprite = getBuildingSprite(buildingId);
    if (sprite) {
        Vec2 worldPos = GridMapUtils::gridToPixel((int)gridPos.x, (int)gridPos.y);
        sprite->setPosition(worldPos);
        sprite->setPlacementPreview(false);  // 恢复正常显示
    }
    
    CCLOG("Building %d moved to grid(%d, %d)", 
          buildingId, (int)gridPos.x, (int)gridPos.y);
}

void BuildingManager::cancelBuildingMove(int buildingId) {
    // 恢复到原始位置
    if (_originalBuildingPositions.find(buildingId) != _originalBuildingPositions.end()) {
        Vec2 originalGrid = _originalBuildingPositions[buildingId];
        
        auto sprite = getBuildingSprite(buildingId);
        if (sprite) {
            Vec2 worldPos = GridMapUtils::gridToPixel(
                (int)originalGrid.x, (int)originalGrid.y);
            sprite->setPosition(worldPos);
            sprite->setPlacementPreview(false);
        }
        
        _originalBuildingPositions.erase(buildingId);
    }
}
```

## ?? 关键点

1. **坐标转换**：InputController 传递的是世界坐标，需要转换为网格坐标
2. **实时预览**：拖动时通过 `setPlacementPreview()` 改变建筑颜色提示合法性
3. **合法性检查**：检查边界和碰撞
4. **原位恢复**：移动失败时恢复到原始位置

## ?? 状态转换图

```
MAP_DRAG ──startBuildingMove()──> BUILDING_MOVING
    ↑                                    │
    │                                    │
    └────completeBuildingMove()──────────┘
           or cancelBuildingMove()
```

## ?? 注意事项

1. 在 `BUILDING_MOVING` 状态下，地图拖动被禁用
2. 需要保存建筑原始位置，以便取消时恢复
3. 拖动过程中实时检查合法性并给予视觉反馈
4. 使用 `convertToNodeSpace()` 将屏幕坐标转换为 Layer 坐标

## ?? 视觉反馈建议

- 拖动时建筑半透明
- 可放置：绿色边框
- 不可放置：红色边框
- 显示网格对齐辅助线

## ?? 相关回调

- `_onBuildingMoveUpdate` - 拖动中（每帧）
- `_onBuildingMoveComplete` - 移动完成（松手时）
- `_onStateChanged` - 状态改变
