# Battle Troop Control Refactoring Summary

## 概述
将 `BattleTroopLayer` 中的控制逻辑移动到 `BattleProcessController`，实现更清晰的职责分离。

## 修改内容

### 1. 新建 `BattleProcessController` 控制器

**文件**: `Classes/Controller/BattleProcessController.h` 和 `.cpp`

**职责**:
- 控制单位的AI行为（寻路、攻击、目标选择）
- 管理战斗逻辑（伤害计算、战斗状态等）
- 分离控制逻辑与显示层

**核心方法**:
```cpp
// 启动单位AI（寻路+攻击）
void startUnitAI(BattleUnitSprite* unit, BattleTroopLayer* troopLayer);

// 查找离单位最近的建筑目标
const BuildingInstance* findNearestBuilding(const Vec2& unitWorldPos);
```

### 2. 简化 `BattleTroopLayer`

**移除的方法**:
- `startUnitAI()` - 移动到 `BattleProcessController`
- `findNearestBuilding()` - 移动到 `BattleProcessController`

**保留的方法** (纯显示层):
- `spawnUnit()` - 生成单位
- `spawnUnitsGrid()` - 批量生成单位
- `removeAllUnits()` - 移除所有单位
- `getAllUnits()` - 获取所有单位

**移除的依赖**:
```cpp
// 不再需要这些头文件
#include "../Manager/VillageDataManager.h"
#include "../Model/BuildingConfig.h"
#include "../Util/FindPathUtil.h"
```

### 3. 更新 `BattleScene`

**修改点**: `onTouchEnded()` 方法

**之前**:
```cpp
auto unit = troopLayer->spawnUnit(name, gx, gy);
if (unit) {
    troopLayer->startUnitAI(unit);  // 直接调用层的方法
}
```

**之后**:
```cpp
auto unit = troopLayer->spawnUnit(name, gx, gy);
if (unit) {
    // 使用控制器启动AI
    auto controller = BattleProcessController::getInstance();
    controller->startUnitAI(unit, troopLayer);
}
```

**新增依赖**:
```cpp
#include "Controller/BattleProcessController.h"
```

## 架构优势

### 修改前
```
BattleScene
    ↓
BattleTroopLayer (显示 + 控制混合)
    ↓
BattleUnitSprite
```

### 修改后
```
BattleScene
    ↓
BattleProcessController (纯控制逻辑)
    ↓
BattleTroopLayer (纯显示层)
    ↓
BattleUnitSprite
```

## 职责分离

### BattleTroopLayer (显示层)
- ? 管理单位精灵的生命周期
- ? 处理单位的显示和布局
- ? 提供单位访问接口
- ? **不再**处理AI逻辑
- ? **不再**处理战斗逻辑

### BattleProcessController (控制层)
- ? 管理单位AI行为
- ? 处理目标选择算法
- ? 控制寻路和攻击流程
- ? 未来扩展：伤害计算、战斗奖励等

## 编译状态
? 所有文件编译成功，无错误

## 后续优化建议
1. 考虑将 `BattleProcessController` 改为非单例，由 `BattleScene` 持有实例
2. 添加战斗事件回调机制（单位死亡、建筑摧毁等）
3. 实现循环攻击逻辑和伤害计算
4. 添加单位状态机（Idle, Moving, Attacking, Dead）
