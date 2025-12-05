#ifndef __GAME_SAVE_DATA_H__
#define __GAME_SAVE_DATA_H__

#include <vector>
#include <string>

// 建筑存档数据
struct BuildingSaveData {
  int buildingId;
  int buildingType;
  int level;
  int gridX, gridY;
  bool isUpgrading;
  long long upgradeStartTime;
  long long lastCollectTime;  // 资源建筑专用
  int storedResource;         // 已存储资源
};

// 游戏存档数据
struct GameSaveData {
  // 玩家信息
  int playerLevel;
  std::string playerName;
  long long lastSaveTime;

  // 资源
  int gold;
  int elixir;
  int darkElixir;
  int gems;

  // 建筑列表
  std::vector<BuildingSaveData> buildings;
};

#endif