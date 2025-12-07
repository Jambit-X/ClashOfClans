#include "MoveBuildingController.h"
#include "Manager/BuildingManager.h"
#include "Manager/VillageDataManager.h"
#include "Sprite/BuildingSprite.h"
#include "Util/GridMapUtils.h"

USING_NS_CC;

MoveBuildingController::MoveBuildingController(Layer* layer, BuildingManager* buildingManager)
    : _parentLayer(layer)
    , _buildingManager(buildingManager)
    , _isMoving(false)
    , _movingBuildingId(-1)
    , _touchListener(nullptr)
    , _isDragging(false)
    , _touchStartPos(Vec2::ZERO) {
    CCLOG("MoveBuildingController: Initialized");
}

MoveBuildingController::~MoveBuildingController() {
    // 清理触摸监听器
    if (_touchListener) {
        Director::getInstance()->getEventDispatcher()->removeEventListener(_touchListener);
        _touchListener = nullptr;
    }
    CCLOG("MoveBuildingController: Destroyed");
}

#pragma region 移动流程控制
void MoveBuildingController::startMoving(int buildingId) {
    if (_isMoving) {
        CCLOG("MoveBuildingController: Already moving building %d", _movingBuildingId);
        return;
    }
    
    // 保存原始位置
    saveOriginalPosition(buildingId);
    
    _isMoving = true;
    _movingBuildingId = buildingId;
    
    // 使用 BuildingSprite 的拖动模式
    auto sprite = _buildingManager->getBuildingSprite(buildingId);
    if (sprite) {
        sprite->setDraggingMode(true);
    }
    
    CCLOG("MoveBuildingController: Started moving building ID=%d", buildingId);
}

void MoveBuildingController::cancelMoving() {
    if (!_isMoving) {
        return;
    }
    
    CCLOG("MoveBuildingController: Cancelling move for building %d", _movingBuildingId);
    
    // 恢复原始位置
    if (_originalPositions.find(_movingBuildingId) != _originalPositions.end()) {
        Vec2 originalGridPos = _originalPositions[_movingBuildingId];
        
        auto sprite = _buildingManager->getBuildingSprite(_movingBuildingId);
        if (sprite) {
            // ? 恢复网格坐标（确保数据一致）
            sprite->setGridPos(originalGridPos);
            
            // ? 使用统一函数计算视觉位置
            Vec2 visualPos = GridMapUtils::getVisualPosition(
                (int)originalGridPos.x, 
                (int)originalGridPos.y, 
                sprite->getVisualOffset()
            );
            sprite->setPosition(visualPos);
            
            // 恢复正常显示模式
            sprite->setDraggingMode(false);
            sprite->setPlacementPreview(false);
            sprite->setColor(Color3B::WHITE);
            sprite->setOpacity(255);  // 确保完全不透明
        }
        
        _originalPositions.erase(_movingBuildingId);
    }
    
    _isMoving = false;
    _movingBuildingId = -1;
}

#pragma endregion

#pragma region 触摸事件处理
bool MoveBuildingController::onTouchBegan(Touch* touch, Event* event) {
    // 保存触摸起始位置
    _touchStartPos = touch->getLocation();
    _isDragging = false;
    
    // 如果已经在移动状态，吞噬事件继续拖动
    if (_isMoving) {
        CCLOG("MoveBuildingController: Touch began at (%.0f, %.0f) - Ready to drag", 
              _touchStartPos.x, _touchStartPos.y);
        return true;  // 吞噬事件
    }
    
    // 如果不在移动状态，检查是否点击到建筑
    Vec2 worldPos = _parentLayer->convertToNodeSpace(_touchStartPos);
    auto building = _buildingManager->getBuildingAtWorldPos(worldPos);
    
    if (building) {
        // 点击到建筑，立即进入移动模式
        CCLOG("MoveBuildingController: Touch on building ID=%d, entering move mode", building->getBuildingId());
        startMoving(building->getBuildingId());
        return true;  // 吞噬事件
    }
    
    // 没点击到建筑，不吞噬事件，让 InputController 处理
    return false;
}

void MoveBuildingController::onTouchMoved(Touch* touch, Event* event) {
    if (!_isMoving || _movingBuildingId < 0) {
        return;
    }
    
    // 检查是否真的在拖动（移动距离超过阈值）
    Vec2 currentPos = touch->getLocation();
    float distance = _touchStartPos.distance(currentPos);
    
    CCLOG("MoveBuildingController::onTouchMoved - distance=%.2f, threshold=5.0", distance);
    
    const float DRAG_THRESHOLD = 5.0f;  // 5像素阈值
    
    if (distance > DRAG_THRESHOLD) {
        _isDragging = true;
        CCLOG("MoveBuildingController::onTouchMoved - isDragging set to TRUE");
    }
    
    // 只有真正拖动时才更新位置
    if (_isDragging) {
        // 将屏幕坐标转换为 Layer 内的世界坐标
        Vec2 worldPos = _parentLayer->convertToNodeSpace(currentPos);
        
        // 更新建筑预览位置
        updatePreviewPosition(worldPos);
    }
}

void MoveBuildingController::onTouchEnded(Touch* touch, Event* event) {
    if (!_isMoving || _movingBuildingId < 0) {
        return;
    }
    
    CCLOG("MoveBuildingController: Touch ended - isDragging=%s", _isDragging ? "true" : "false");
    
    // 只有真正拖动过才完成移动
    if (_isDragging) {
        // 获取触摸点的屏幕坐标
        Vec2 touchPos = touch->getLocation();
        
        // 将屏幕坐标转换为 Layer 内的世界坐标
        Vec2 worldPos = _parentLayer->convertToNodeSpace(touchPos);
        
        // 完成建筑移动
        bool success = completeMove(worldPos);
        
        if (success) {
            CCLOG("MoveBuildingController: Building %d moved successfully", _movingBuildingId);
        } else {
            CCLOG("MoveBuildingController: Building %d move failed, restored", _movingBuildingId);
        }
        
        // 移动完成后退出移动模式
        _isMoving = false;
        _movingBuildingId = -1;
        _isDragging = false;
    } else {
        // ✅ 只是点击，没有拖动 -> 直接调用 cancelMoving()
        CCLOG("MoveBuildingController: Just a tap, cancelling move (stay in place)");
        cancelMoving();
    }
}
#pragma endregion

#pragma region 位置更新和完成
void MoveBuildingController::updatePreviewPosition(const Vec2& worldPos) {
    auto sprite = _buildingManager->getBuildingSprite(_movingBuildingId);
    if (!sprite) {
        CCLOG("MoveBuildingController: Building sprite not found: %d", _movingBuildingId);
        return;
    }
    
    // 统一计算位置信息
    BuildingPositionInfo posInfo = calculatePositionInfo(worldPos, _movingBuildingId);
    
    // 更新精灵位置
    sprite->setPosition(posInfo.worldPos);
    
    // ? 实时更新网格坐标（用于碰撞检测）
    sprite->setGridPos(posInfo.gridPos);
    
    // 显示视觉反馈
    sprite->setPlacementPreview(posInfo.isValid);
    
    CCLOG("MoveBuildingController: Preview at grid(%.0f, %.0f) - %s", 
          posInfo.gridPos.x, posInfo.gridPos.y, posInfo.isValid ? "VALID" : "INVALID");
}

bool MoveBuildingController::completeMove(const Vec2& worldPos) {
    auto sprite = _buildingManager->getBuildingSprite(_movingBuildingId);
    if (!sprite) {
        return false;
    }
    
    // 统一计算位置信息
    BuildingPositionInfo posInfo = calculatePositionInfo(worldPos, _movingBuildingId);
    
    // 检查是否可以放置
    if (!posInfo.isValid) {
        CCLOG("MoveBuildingController: Invalid position, cancelling move");
        cancelMoving();
        return false;
    }
    
    // ? 1. 更新数据层
    auto dataManager = VillageDataManager::getInstance();
    dataManager->setBuildingPosition(_movingBuildingId, (int)posInfo.gridPos.x, (int)posInfo.gridPos.y);
    
    // ? 2. 更新精灵的网格坐标（确保数据一致）
    sprite->setGridPos(posInfo.gridPos);
    
    // ? 3. 更新精灵显示 - 完全恢复正常状态
    sprite->setPosition(posInfo.worldPos);
    sprite->setDraggingMode(false);      // 退出拖动模式（会恢复 opacity 和 scale）
    sprite->setPlacementPreview(false);  // 关闭预览模式
    sprite->setColor(Color3B::WHITE);    // 恢复颜色
    sprite->setOpacity(255);             // 确保完全不透明
    
    // 清除原始位置记录
    _originalPositions.erase(_movingBuildingId);
    
    CCLOG("MoveBuildingController: Building %d moved to grid(%.0f, %.0f)", 
          _movingBuildingId, posInfo.gridPos.x, posInfo.gridPos.y);
    
    return true;
}
#pragma endregion

#pragma region 合法性检查
bool MoveBuildingController::canPlaceBuildingAt(int buildingId, const Vec2& gridPos) {
    auto sprite = _buildingManager->getBuildingSprite(buildingId);
    if (!sprite) return false;
    
    Size gridSize = sprite->getGridSize();
    
    // 检查1：边界检查
    if (!GridMapUtils::isBuildingInBounds(
        (int)gridPos.x, (int)gridPos.y, 
        (int)gridSize.width, (int)gridSize.height)) {
        CCLOG("MoveBuildingController: Out of bounds!");
        return false;
    }
    
    // ? 检查2：使用 O(1) 网格占用表检测碰撞
    auto dataManager = VillageDataManager::getInstance();
    
    // isAreaOccupied(startX, startY, width, height, ignoreBuildingId)
    // 参数5：ignoreBuildingId = buildingId（忽略自己）
    bool occupied = dataManager->isAreaOccupied(
        (int)gridPos.x, 
        (int)gridPos.y, 
        (int)gridSize.width, 
        (int)gridSize.height, 
        buildingId  // 忽略自己
    );
    
    if (occupied) {
        CCLOG("MoveBuildingController: Area occupied by other building!");
        return false;
    }
    
    return true;
}
#pragma endregion

#pragma region 辅助方法
void MoveBuildingController::saveOriginalPosition(int buildingId) {
    auto sprite = _buildingManager->getBuildingSprite(buildingId);
    if (!sprite) {
        CCLOG("MoveBuildingController: Cannot save position, sprite not found: %d", buildingId);
        return;
    }
    
    Vec2 gridPos = sprite->getGridPos();
    _originalPositions[buildingId] = gridPos;
    
    CCLOG("MoveBuildingController: Saved original position grid(%.0f, %.0f) for building %d", 
          gridPos.x, gridPos.y, buildingId);
}

BuildingPositionInfo MoveBuildingController::calculatePositionInfo(const Vec2& touchWorldPos, int buildingId) {
    BuildingPositionInfo info = { Vec2::ZERO, Vec2::ZERO, false };
    
    auto sprite = _buildingManager->getBuildingSprite(buildingId);
    if (!sprite) {
        CCLOG("MoveBuildingController: Building sprite not found: %d", buildingId);
        return info;
    }
    
    // 获取建筑的网格大小
    Size gridSize = sprite->getGridSize();
    
    // 转换为网格坐标
    Vec2 gridPos = GridMapUtils::pixelToGrid(touchWorldPos);
    
    // 将触摸点作为建筑中心，计算左下角位置
    float leftBottomGridX = gridPos.x - gridSize.width * 0.5f;
    float leftBottomGridY = gridPos.y - gridSize.height * 0.5f;
    
    // 对齐到网格整数坐标
    int gridX = (int)std::round(leftBottomGridX);
    int gridY = (int)std::round(leftBottomGridY);
    Vec2 alignedGridPos(gridX, gridY);
    
    // ? 使用统一函数计算视觉位置（网格坐标 + 视觉偏移）
    Vec2 alignedWorldPos = GridMapUtils::getVisualPosition(
        gridX, 
        gridY, 
        sprite->getVisualOffset()
    );
    
    // 检查合法性
    bool isValid = canPlaceBuildingAt(buildingId, alignedGridPos);
    
    info.gridPos = alignedGridPos;
    info.worldPos = alignedWorldPos;
    info.isValid = isValid;
    
    return info;
}
#pragma endregion

void MoveBuildingController::setupTouchListener() {
    _touchListener = EventListenerTouchOneByOne::create();
    _touchListener->setSwallowTouches(true);
    
    _touchListener->onTouchBegan = CC_CALLBACK_2(MoveBuildingController::onTouchBegan, this);
    _touchListener->onTouchMoved = CC_CALLBACK_2(MoveBuildingController::onTouchMoved, this);
    _touchListener->onTouchEnded = CC_CALLBACK_2(MoveBuildingController::onTouchEnded, this);
    
    // ✅ 修复：使用 Scene Graph Priority（遵循渲染树层级）
    // 绑定到 _parentLayer，让 Cocos 自然决定事件优先级
    // UI 在上层 → 优先接收事件
    // 建筑在下层 → UI 不处理才轮到建筑
    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(_touchListener, _parentLayer);
    
    CCLOG("MoveBuildingController: Touch listener set up with Scene Graph Priority (respects UI layers)");
}
