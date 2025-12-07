# 建筑拖动修复方案

## ?? **问题分析**

### **当前的问题**

你发现的问题完全正确！当前的实现存在致命缺陷：

```cpp
void MoveBuildingController::onTouchEnded(Touch* touch, Event* event) {
    // ...
    _isMoving = false;  // ? 松手后立即退出移动模式
    _movingBuildingId = -1;
}
```

**问题场景**：
```
1. 用户点击建筑
   → InputController 调用 startMoving()
   → _isMoving = true

2. 用户抬起手指（第一次点击结束）
   → onTouchEnded() 触发
   → _isMoving = false  ← ? 状态被重置了！

3. 用户再次按下并拖动
   → onTouchBegan() 被调用
   → _isMoving == false  ← ? 返回 false
   → 事件传递给 InputController
   → ? 无法拖动建筑！
```

---

## ? **解决方案：添加拖动检测**

### **核心思路**

区分**"点击选中"**和**"拖动移动"**：
- 第一次点击：进入"准备移动"状态（`_isMoving = true`）
- 拖动超过阈值：进入"正在拖动"状态（`_isDragging = true`）
- 拖动后松手：完成移动，退出移动模式
- 只是点击（未拖动）：保持"准备移动"状态，等待下一次拖动

---

## ??? **代码修改**

### **1. 添加成员变量（MoveBuildingController.h）**

```cpp
private:
    cocos2d::Layer* _parentLayer;
    BuildingManager* _buildingManager;
    
    bool _isMoving;                      // 是否正在移动建筑（准备移动状态）
    bool _isDragging;                    // 是否正在拖动建筑（NEW!）
    int _movingBuildingId;
    cocos2d::Vec2 _touchStartPos;        // 触摸开始位置（NEW!）
    
    std::map<int, cocos2d::Vec2> _originalPositions;
    cocos2d::EventListenerTouchOneByOne* _touchListener;
```

---

### **2. 修改构造函数（MoveBuildingController.cpp）**

```cpp
MoveBuildingController::MoveBuildingController(Layer* layer, BuildingManager* buildingManager)
    : _parentLayer(layer)
    , _buildingManager(buildingManager)
    , _isMoving(false)
    , _movingBuildingId(-1)
    , _touchListener(nullptr)
    , _isDragging(false)              // NEW!
    , _touchStartPos(Vec2::ZERO) {    // NEW!
    CCLOG("MoveBuildingController: Initialized");
}
```

---

### **3. 重新实现触摸事件处理**

```cpp
bool MoveBuildingController::onTouchBegan(Touch* touch, Event* event) {
    // 只在移动状态时处理触摸事件
    if (!_isMoving) {
        return false;
    }
    
    // 保存触摸起始位置
    _touchStartPos = touch->getLocation();
    _isDragging = false;
    
    CCLOG("MoveBuildingController: Touch began - Ready to drag");
    return true;
}

void MoveBuildingController::onTouchMoved(Touch* touch, Event* event) {
    if (!_isMoving || _movingBuildingId < 0) {
        return;
    }
    
    // 检查是否真的在拖动（移动距离超过阈值）
    Vec2 currentPos = touch->getLocation();
    float distance = _touchStartPos.distance(currentPos);
    
    const float DRAG_THRESHOLD = 5.0f;  // 5像素阈值
    
    if (distance > DRAG_THRESHOLD) {
        _isDragging = true;
    }
    
    // 只有真正拖动时才更新位置
    if (_isDragging) {
        Vec2 worldPos = _parentLayer->convertToNodeSpace(currentPos);
        updatePreviewPosition(worldPos);
    }
}

void MoveBuildingController::onTouchEnded(Touch* touch, Event* event) {
    if (!_isMoving || _movingBuildingId < 0) {
        return;
    }
    
    CCLOG("MoveBuildingController: Touch ended - isDragging=%s", 
          _isDragging ? "true" : "false");
    
    // 只有真正拖动过才完成移动
    if (_isDragging) {
        Vec2 touchPos = touch->getLocation();
        Vec2 worldPos = _parentLayer->convertToNodeSpace(touchPos);
        
        bool success = completeMove(worldPos);
        
        if (success) {
            CCLOG("MoveBuildingController: Building %d moved successfully", _movingBuildingId);
        } else {
            CCLOG("MoveBuildingController: Building %d move failed, restored", _movingBuildingId);
        }
        
        // ? 移动完成后退出移动模式
        _isMoving = false;
        _movingBuildingId = -1;
        _isDragging = false;
    } else {
        // ? 只是点击，不是拖动，保持移动状态
        CCLOG("MoveBuildingController: Just a tap, keeping move mode active");
        _isDragging = false;
    }
}
```

---

## ?? **新的工作流程**

### **场景1：点击建筑并拖动**

```
1. 用户点击建筑
   → InputController 调用 startMoving()
   → _isMoving = true
   → _isDragging = false

2. 用户抬起手指（第一次点击结束）
   → onTouchEnded() 触发
   → 检查：_isDragging == false
   → ? 只是点击，保持 _isMoving = true

3. 用户再次按下并拖动
   → onTouchBegan() 被调用
   → _isMoving == true  ? 返回 true
   → 保存 _touchStartPos

4. 用户拖动手指
   → onTouchMoved() 触发
   → 检查拖动距离 > 5px
   → _isDragging = true  ? 进入拖动模式
   → 更新建筑位置

5. 用户松手
   → onTouchEnded() 触发
   → 检查：_isDragging == true
   → ? 完成移动，退出移动模式
   → _isMoving = false
```

### **场景2：多次点击（未拖动）**

```
1. 用户点击建筑
   → _isMoving = true

2. 用户抬起手指
   → _isDragging == false
   → ? 保持 _isMoving = true

3. 用户再次点击（但未拖动）
   → onTouchBegan() → onTouchEnded()
   → _isDragging == false
   → ? 保持 _isMoving = true

4. 用户第三次点击并拖动
   → 拖动距离 > 5px
   → _isDragging = true
   → ? 成功拖动建筑
```

---

## ?? **状态机图**

```
┌─────────────────┐
│  IDLE（空闲）    │
│ _isMoving=false │
└────────┬────────┘
         │
         │ 点击建筑
         ↓
┌─────────────────────┐
│  READY（准备移动）   │
│  _isMoving=true     │
│  _isDragging=false  │
└────────┬────────────┘
         │
         │ 拖动 > 5px
         ↓
┌─────────────────────┐
│  DRAGGING（拖动中）  │
│  _isMoving=true     │
│  _isDragging=true   │
└────────┬────────────┘
         │
         │ 松手
         ↓
┌─────────────────────┐
│  COMPLETE（完成）    │
│  检查位置合法性      │
└────────┬────────────┘
         │
         ↓
┌─────────────────┐
│  IDLE（空闲）    │
│ _isMoving=false │
└─────────────────┘
```

---

## ?? **关键参数**

```cpp
const float DRAG_THRESHOLD = 5.0f;  // 拖动阈值（像素）
```

**说明**：
- 小于 5 像素：视为点击
- 大于 5 像素：视为拖动

---

## ? **优点**

1. ? **支持多次点击**：用户可以先点击建筑，然后思考，再开始拖动
2. ? **防止误触**：小的抖动不会触发拖动
3. ? **状态清晰**：`_isMoving` 和 `_isDragging` 分别表示不同状态
4. ? **用户友好**：不需要一次性完成点击+拖动

---

## ?? **测试场景**

### **测试1：正常拖动**
1. 点击建筑
2. 拖动到新位置
3. 松手
4. ? 建筑移动成功

### **测试2：多次点击**
1. 点击建筑
2. 抬起手指
3. 再次点击
4. 抬起手指
5. 第三次点击并拖动
6. ? 建筑跟随拖动

### **测试3：取消移动**
1. 点击建筑
2. 拖动到非法位置
3. 松手
4. ? 建筑恢复原位

---

## ?? **总结**

**修复前**：
- ? 点击后抬起手指，`_isMoving` 立即变为 `false`
- ? 无法再次拖动

**修复后**：
- ? 点击后抬起手指，`_isMoving` 保持为 `true`
- ? 只有拖动完成后才退出移动模式
- ? 支持"点击选中 → 思考 → 拖动移动"的自然流程

**你的观察非常敏锐！这个问题会导致建筑移动功能完全失效。** ??
