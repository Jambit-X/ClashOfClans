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
};

struct VillageData {
  int gold;
  int elixir;
  int gem;
  std::vector<BuildingInstance> buildings;

  // 已训练的军队数据 <兵种ID, 数量>
  std::map<int, int> troops;
};