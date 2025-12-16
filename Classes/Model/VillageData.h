#pragma once
#include <string>
#include <vector>
#include <map> 

struct BuildingInstance {
  int id;
  int type;
  int level;
  int gridX;
  int gridY;

  enum class State {
    PLACING,        // 放置中
    CONSTRUCTING,   // 建造/升级中
    BUILT           // 已完成
  };
  State state;

  long long finishTime;

  // 新增：是否为首次建造（用于区分新建筑和升级）
  bool isInitialConstruction;
  
  // ========== 战斗系统：运行时血量 ==========
  int currentHP;  // 当前血量（战斗时使用，初始值=配置的hitPoints）
  bool isDestroyed; // 新增：是否已被摧毁（用于渲染）
  // =========================================
};

struct VillageData {
  int gold;
  int elixir;
  int gem;
  std::vector<BuildingInstance> buildings;

  // 已训练的军队数据 <兵种ID, 数量>
  std::map<int, int> troops;

  // ========== 实验室研究系统 ==========
  // 兵种研究等级 <兵种ID, 等级>（默认1级）
  std::map<int, int> troopLevels;

  // 正在研究的兵种（-1 表示无）
  int researchingTroopId = -1;

  // 研究完成时间戳
  long long researchFinishTime = 0;
  // ====================================
};
