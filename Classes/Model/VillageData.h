// VillageData.h
// 村庄数据结构定义，包含建筑和资源信息

#pragma once
#include <string>
#include <vector>
#include <map>

// 建筑实例数据
struct BuildingInstance {
  int id;
  int type;
  int level;
  int gridX;
  int gridY;

  // 建筑状态枚举
  enum class State {
    PLACING,        // 放置中
    CONSTRUCTING,   // 建造/升级中
    BUILT           // 已完成
  };
  State state;

  long long finishTime;

  // 区分新建筑和升级
  bool isInitialConstruction;

  // 战斗系统运行时数据
  int currentHP;        // 当前生命值
  bool isDestroyed;     // 是否已被摧毁

  // 防御建筑锁定目标
  mutable void* lockedTarget = nullptr;  // 锁定的兵种指针（BattleUnitSprite*）

  // 攻击冷却系统
  float attackCooldown = 0.0f;  // 当前冷却时间（秒）
};

// 村庄数据
struct VillageData {
  int gold;
  int elixir;
  int gem;
  std::vector<BuildingInstance> buildings;

  // 已训练的军队 <兵种ID, 数量>
  std::map<int, int> troops;

  // 实验室研究系统
  std::map<int, int> troopLevels;    // 兵种等级 <兵种ID, 等级>，默认1级
  int researchingTroopId = -1;       // 正在研究的兵种，-1表示无
  long long researchFinishTime = 0;  // 研究完成时间戳
};
