// VillageDataManager.h
// 村庄数据管理器头文件，负责管理游戏核心数据和状态

#pragma once
#include "../Model/VillageData.h"
#include "../Model/BattleMapData.h"
#include <functional>
#include <ctime>
#include "../Model/TroopConfig.h"

class VillageDataManager {
public:
  static VillageDataManager* getInstance();
  static void destroyInstance();

  // 资源接口
  int getGold() const;
  int getElixir() const;
  void addGold(int amount);
  void addElixir(int amount);
  bool spendGold(int amount);
  bool spendElixir(int amount);
  int getGem() const;
  void addGem(int amount);
  bool spendGem(int amount);

  using ResourceCallback = std::function<void(int gold, int elixir)>;
  void setResourceCallback(ResourceCallback callback);

  void checkAndFinishConstructions();

  // 建筑接口
  const std::vector<BuildingInstance>& getAllBuildings() const;
  BuildingInstance* getBuildingById(int id);
  BuildingInstance* getBuildingAtGrid(int gridX, int gridY);

  int addBuilding(int type, int level, int gridX, int gridY,
                  BuildingInstance::State state,
                  long long finishTime = 0,
                  bool isInitialConstruction = true);

  void upgradeBuilding(int id, int newLevel, long long finishTime);
  void setBuildingPosition(int id, int gridX, int gridY);
  void setBuildingState(int id, BuildingInstance::State state, long long finishTime = 0);

  bool startUpgradeBuilding(int id);
  void finishNewBuildingConstruction(int id);
  void finishUpgradeBuilding(int id);
  bool startConstructionAfterPlacement(int buildingId);
  void removeBuilding(int buildingId);

  // 网格占用查询
  bool isAreaOccupied(int startX, int startY, int width, int height, int ignoreBuildingId = -1) const;
  void updateGridOccupancy();

  // 存档/读档
  void loadFromFile(const std::string& filename);
  void saveToFile(const std::string& filename);

  // 军队与兵营接口
  int getTownHallLevel() const;
  int getArmyCampCount() const;
  int calculateTotalHousingSpace() const;
  int getCurrentHousingSpace() const;

  int getTroopCount(int troopId) const;
  void addTroop(int troopId, int count);
  bool removeTroop(int troopId, int count);

  const std::map<int, int>& getAllTroops() const { return _data.troops; }

  // 资源存储容量接口
  int getGoldStorageCapacity() const;
  int getElixirStorageCapacity() const;

  // 工人系统
  int getTotalWorkers() const;
  int getBusyWorkerCount() const;
  bool hasIdleWorker() const;
  int getIdleWorkerCount() const;

  // 实验室与兵种升级
  int getLaboratoryLevel() const;
  int getTroopLevel(int troopId) const;
  bool canUpgradeTroop(int troopId) const;
  bool startTroopUpgrade(int troopId);
  void finishTroopUpgrade();
  void checkAndFinishResearch();
  int getResearchingTroopId() const;
  long long getResearchFinishTime() const;
  bool canUpgradeLaboratory() const;
  bool isResearching() const;
  void finishResearchImmediately();

  // 钻石接口别名
  int getGems() const { return getGem(); }
  void addGems(int amount) { addGem(amount); }

  // 战斗地图
  void setBattleMapData(const BattleMapData& data);
  const BattleMapData& getBattleMapData() const;
  void generateRandomBattleMap(int difficulty = 0);
  bool hasBattleMapData() const;
  
  // 战斗模式切换
  void setInBattleMode(bool inBattle);
  bool isInBattleMode() const;
  void updateBattleGridOccupancy();

  // 回放相关方法
  void clearBattleMap();
  void addBattleBuildingFromReplay(const BuildingInstance& building);

  // 场景管理
  int getCurrentThemeId() const;
  void setCurrentTheme(int themeId);
  bool isThemePurchased(int themeId) const;
  void purchaseTheme(int themeId);

private:
  VillageDataManager();
  ~VillageDataManager();

  void notifyResourceChanged();

  static VillageDataManager* _instance;
  VillageData _data;
  int _nextBuildingId;

  std::vector<std::vector<int>> _gridOccupancy;
  std::vector<std::vector<int>> _battleGridOccupancy;

  ResourceCallback _resourceCallback;
  
  BattleMapData _battleMapData;
  bool _inBattleMode = false;

  int _currentThemeId;
  std::set<int> _purchasedThemes;
};
