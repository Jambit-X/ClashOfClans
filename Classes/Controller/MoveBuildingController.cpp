#include "MoveBuildingController.h"
#include "Manager/BuildingManager.h"
#include "Manager/VillageDataManager.h"
#include "Sprite/BuildingSprite.h"
#include "Util/GridMapUtils.h"
#include "Layer/HUDLayer.h" 

USING_NS_CC;

MoveBuildingController::MoveBuildingController(Layer* layer, BuildingManager* buildingManager)
    : _parentLayer(layer)
    , _buildingManager(buildingManager)
    , _isMoving(false)
    , _movingBuildingId(-1)
    , _touchListener(nullptr)
    , _isDragging(false)
    , _touchStartPos(Vec2::ZERO)
    , _touchDownTime(0.0f)        //  初始化
    , _isLongPressTriggered(false)   //  初始化
    , _touchedBuildingId(-1) {          //  初始化
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

  // 检查建筑状态
  auto dataManager = VillageDataManager::getInstance();
  auto building = dataManager->getBuildingById(buildingId);

  if (!building) {
    CCLOG("MoveBuildingController: Building %d not found", buildingId);
    return;
  }

  // 只有 PLACING 状态不允许通过长按移动（因为正在首次放置中）
  if (building->state == BuildingInstance::State::PLACING) {
    CCLOG("MoveBuildingController: Cannot move building in PLACING state");
    return;
  }

  // 保存原始位置
  saveOriginalPosition(buildingId);

  _isMoving = true;
  _movingBuildingId = buildingId;

  auto sprite = _buildingManager->getBuildingSprite(buildingId);
  if (sprite) {
    // 根据建筑状态决定拖动模式的显示
    if (building->state == BuildingInstance::State::CONSTRUCTING) {
      // 建造中的建筑：保持暗灰色，不改变透明度
      CCLOG("MoveBuildingController: Moving CONSTRUCTING building, keeping dark color");
      // 不调用 setDraggingMode()，因为会覆盖建造动画的颜色
      // 只增加一点缩放效果表示正在拖动
      sprite->setScale(1.05f);
    } else {
      // 已建造完成的建筑：正常拖动模式
      sprite->setDraggingMode(true);
    }
  }

  CCLOG("MoveBuildingController: Started moving building ID=%d (state=%d)",
        buildingId, (int)building->state);
}

void MoveBuildingController::cancelMoving() {
  if (!_isMoving) {
    return;
  }

  CCLOG("MoveBuildingController: Cancelling move for building %d", _movingBuildingId);

  // 检查建筑状态
  auto dataManager = VillageDataManager::getInstance();
  auto building = dataManager->getBuildingById(_movingBuildingId);
  bool isConstructing = (building && building->state == BuildingInstance::State::CONSTRUCTING);

  // 恢复原始位置
  if (_originalPositions.find(_movingBuildingId) != _originalPositions.end()) {
    Vec2 originalGridPos = _originalPositions[_movingBuildingId];

    auto sprite = _buildingManager->getBuildingSprite(_movingBuildingId);
    if (sprite) {
      sprite->setGridPos(originalGridPos);

      Vec2 visualPos = GridMapUtils::getVisualPosition(
        (int)originalGridPos.x,
        (int)originalGridPos.y,
        sprite->getVisualOffset()
      );
      sprite->setPosition(visualPos);

      // 根据状态恢复显示
      if (isConstructing) {
        // 建造中的建筑：保持建造动画状态
        sprite->setScale(1.0f);
        sprite->setOpacity(255);
        sprite->setColor(Color3B(80, 80, 80));  // 保持暗灰色
      } else {
        // 已完成的建筑：完全恢复
        sprite->setDraggingMode(false);
        sprite->setPlacementPreview(false);
        sprite->setColor(Color3B::WHITE);
        sprite->setOpacity(255);
      }
    }

    _originalPositions.erase(_movingBuildingId);
  }

  _isMoving = false;
  _movingBuildingId = -1;
}

#pragma endregion

#pragma region 触摸事件处理
bool MoveBuildingController::onTouchBegan(Touch* touch, Event* event) {
    // 保存触摸起始位置和时间
    _touchStartPos = touch->getLocation();
    _isDragging = false;
    _isLongPressTriggered = false;
    _touchDownTime = Director::getInstance()->getTotalFrames() / 60.0f;  // 获取当前时间（秒）
    
    // ========== 情况1: 已经在移动状态 =========="
    if (_isMoving) {
      CCLOG("MoveBuildingController: Already in move mode, continue dragging");
        return true;  // 吞噬事件，继续拖动
    }
    
    // ========== 情况2: 检查是否点击到建筑 =========
    Vec2 worldPos = _parentLayer->convertToNodeSpace(_touchStartPos);
    auto building = _buildingManager->getBuildingAtWorldPos(worldPos);
    
    if (building) {
        //  点击到建筑，记录建筑ID，但不立即进入移动模式
        _touchedBuildingId = building->getBuildingId();
        CCLOG("MoveBuildingController: Touched building ID=%d, waiting for long press", _touchedBuildingId);
        
        //  启动定时器检测长按
        _parentLayer->schedule([this](float dt) {
            if (checkLongPress()) {
      CCLOG("MoveBuildingController: Long press detected! Starting move mode");
                startMoving(_touchedBuildingId);
                _isLongPressTriggered = true;
      
    // 停止定时器
            _parentLayer->unschedule("long_press_check");
    }
        }, 0.05f, "long_press_check");  // 每0.05秒检查一次
        
    return true;  //  吞噬事件（防止地图移动）
    }
    
    // ========== 情況3: 没有点击到建筑 =========
  _touchedBuildingId = -1;
    return false;  // 不吞噬事件，让 MoveMapController 处理地图拖动
}

void MoveBuildingController::onTouchMoved(Touch* touch, Event* event) {
    Vec2 currentPos = touch->getLocation();
    float distance = _touchStartPos.distance(currentPos);
    
const float DRAG_THRESHOLD = 5.0f;  // 5像素阈值
    
    // ========== 情况1: 在移动状态，更新拖动 =========
    if (_isMoving && _movingBuildingId >= 0) {
        if (distance > DRAG_THRESHOLD) {
      _isDragging = true;
      CCLOG("MoveBuildingController::onTouchMoved - isDragging set to TRUE");
        }
        
        if (_isDragging) {
            Vec2 worldPos = _parentLayer->convertToNodeSpace(currentPos);
       updatePreviewPosition(worldPos);
        }
        return;
    }
  
    // ========== 情况2: 还没触发长按，但手指移动了 =========
    if (_touchedBuildingId >= 0 && !_isLongPressTriggered) {
  if (distance > DRAG_THRESHOLD) {
            //  手指移动超过阈值，取消长按检测
    CCLOG("MoveBuildingController: Finger moved, cancelling long press check");
            _parentLayer->unschedule("long_press_check");
            _touchedBuildingId = -1;
        }
    }
}

void MoveBuildingController::onTouchEnded(Touch* touch, Event* event) {
    // 停止长按检测定时器
    _parentLayer->unschedule("long_press_check");
    
    // ========== 情况1: 在移动状态，完成移动 =========
    if (_isMoving && _movingBuildingId >= 0) {
 CCLOG("MoveBuildingController: Touch ended - isDragging=%s", _isDragging ? "true" : "false");
        
        if (_isDragging) {
            // 真的拖动过，完成移动
     Vec2 touchPos = touch->getLocation();
   Vec2 worldPos = _parentLayer->convertToNodeSpace(touchPos);
   
   bool success = completeMove(worldPos);
CCLOG("MoveBuildingController: Building %d move %s", 
_movingBuildingId, success ? "succeeded" : "failed");
        } else {
        // 没有拖动，取消移动（恢复原位）
     CCLOG("MoveBuildingController: No drag detected, cancelling move");
   cancelMoving();
        }
        
    // 重置状态
        _isMoving = false;
        _movingBuildingId = -1;
        _isDragging = false;
        return;
    }
    
    // ========== 情况2: 点击建筑但没有长按（短按）==========
    if (_touchedBuildingId >= 0 && !_isLongPressTriggered) {
        CCLOG("MoveBuildingController: Short tap on building ID=%d, triggering callback", _touchedBuildingId);
  
        //  触发短按回调
   if (_onBuildingTapped) {
       _onBuildingTapped(_touchedBuildingId);
     }
    }
    
    // 重置状态
    _touchedBuildingId = -1;
    _isLongPressTriggered = false;
}
#pragma endregion

#pragma region 位置更新和完成
void MoveBuildingController::updatePreviewPosition(const Vec2& worldPos) {
  auto sprite = _buildingManager->getBuildingSprite(_movingBuildingId);
  if (!sprite) {
    CCLOG("MoveBuildingController: Building sprite not found: %d", _movingBuildingId);
    return;
  }

  // 检查建筑状态
  auto dataManager = VillageDataManager::getInstance();
  auto building = dataManager->getBuildingById(_movingBuildingId);
  if (!building) return;

  bool isConstructing = (building->state == BuildingInstance::State::CONSTRUCTING);

  // 统一计算位置信息
  BuildingPositionInfo posInfo = calculatePositionInfo(worldPos, _movingBuildingId);

  // 更新精灵位置（平滑移动）
  Vec2 currentPos = sprite->getPosition();
  Vec2 targetPos = posInfo.worldPos;
  float distance = currentPos.distance(targetPos);

  if (distance > 1.0f) {
    Vec2 smoothPos = currentPos.lerp(targetPos, 0.3f);
    sprite->setPosition(smoothPos);
  } else {
    sprite->setPosition(targetPos);
  }

  // 实时更新网格坐标（用于碰撞检测）
  sprite->setGridPos(posInfo.gridPos);

  // 显示视觉反馈（根据状态）
  if (isConstructing) {
    // 建造中的建筑：不改变颜色，只改变透明度表示合法性
    if (posInfo.isValid) {
      sprite->setOpacity(255);  // 可放置：保持原透明度
    } else {
      sprite->setOpacity(180);  // 不可放置：稍微变透明
    }
    // 保持暗灰色（建造动画的颜色）
    sprite->setColor(Color3B(80, 80, 80));
  } else {
    // 已完成的建筑：正常的绿/红预览
    sprite->setPlacementPreview(posInfo.isValid);
  }

  CCLOG("MoveBuildingController: Preview at grid(%.0f, %.0f) - %s (constructing=%s)",
        posInfo.gridPos.x, posInfo.gridPos.y,
        posInfo.isValid ? "VALID" : "INVALID",
        isConstructing ? "YES" : "NO");
}

bool MoveBuildingController::completeMove(const Vec2& worldPos) {
  auto sprite = _buildingManager->getBuildingSprite(_movingBuildingId);
  if (!sprite) {
    return false;
  }

  // 检查建筑状态
  auto dataManager = VillageDataManager::getInstance();
  auto building = dataManager->getBuildingById(_movingBuildingId);
  if (!building) return false;

  bool isConstructing = (building->state == BuildingInstance::State::CONSTRUCTING);

  // 统一计算位置信息
  BuildingPositionInfo posInfo = calculatePositionInfo(worldPos, _movingBuildingId);

  // 检查是否可以放置
  if (!posInfo.isValid) {
    CCLOG("MoveBuildingController: Invalid position, cancelling move");
    cancelMoving();
    return false;
  }

  // 1. 更新数据层
  dataManager->setBuildingPosition(_movingBuildingId, (int)posInfo.gridPos.x, (int)posInfo.gridPos.y);

  // 2. 更新精灵的网格坐标
  sprite->setGridPos(posInfo.gridPos);
  sprite->setPosition(posInfo.worldPos);

  // 3. 根据状态恢复显示
  if (isConstructing) {
    // 建造中的建筑：恢复建造动画状态
    sprite->setScale(1.0f);  // 恢复缩放
    sprite->setOpacity(255);  // 恢复透明度
    sprite->setColor(Color3B(80, 80, 80));  // 保持暗灰色
    CCLOG("MoveBuildingController: Constructing building moved, keeping dark color");
  } else {
    // 已完成的建筑：完全恢复正常
    sprite->setDraggingMode(false);
    sprite->setPlacementPreview(false);
    sprite->setColor(Color3B::WHITE);
    sprite->setOpacity(255);
    CCLOG("MoveBuildingController: Built building moved, restored to normal");
  }

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
 
    // 修复：使用 Scene Graph Priority（遵循渲染树层级）
    // 绑定到 _parentLayer，让 Cocos 自然决定事件优先级
    // UI 在上层 → 优先接收事件
    // 建筑在下层 → UI 不处理才轮到建筑
    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(_touchListener, _parentLayer);
    
    CCLOG("MoveBuildingController: Touch listener set up with Scene Graph Priority (respects UI layers)");
}

bool MoveBuildingController::checkLongPress() {
    if (_touchedBuildingId < 0 || _isLongPressTriggered) {
    return false;
    }
    
    float currentTime = Director::getInstance()->getTotalFrames() / 60.0f;
    float pressDuration = currentTime - _touchDownTime;
    
    return pressDuration >= LONG_PRESS_DURATION;
}
