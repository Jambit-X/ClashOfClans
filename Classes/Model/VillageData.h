#pragma once
#include <vector>
#include <string>

// 建筑实例
struct BuildingInstance {
  int id;             // 唯一ID
  int type;           // 建筑类型（如1=大本营，2=金矿，3=圣水瓶...）
  int level;          // 等级
  int gridX, gridY;   // 网格坐标（左下角）
  enum State {
    PLACING,        // 待放置状态
    CONSTRUCTING,   // 建造中
    BUILT           // 已建造完成
  } state;
  long long finishTime; // 建造/升级完成时间戳（秒），已建好为0
};

// 村庄数据
struct VillageData {
  int gold;
  int elixir;
  std::vector<BuildingInstance> buildings;
};