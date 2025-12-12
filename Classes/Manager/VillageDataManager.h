#pragma once
#include "../Model/VillageData.h"
#include <functional>
#include <ctime>

class VillageDataManager {
public:
  static VillageDataManager* getInstance();
  static void destroyInstance();

  // ========== 资源接口 ==========
  int getGold() const;
  int getElixir() const;
  void addGold(int amount);
  void addElixir(int amount);
  bool spendGold(int amount);
  bool spendElixir(int amount);

  using ResourceCallback = std::function<void(int gold, int elixir)>;
  void setResourceCallback(ResourceCallback callback);

  // ========== 待收集资源接口 ==========
  int getPendingGold() const;
  int getPendingElixir() const;
  void collectGold();
  void collectElixir();

  int getGoldStorageCapacity() const;
  int getElixirStorageCapacity() const;

  using PendingResourceCallback = std::function<void(int pendingGold, int pendingElixir)>;
  void setPendingResourceCallback(PendingResourceCallback callback);

  // ========== 建筑接口 ==========
  const std::vector<BuildingInstance>& getAllBuildings() const;
  BuildingInstance* getBuildingById(int id);

  // 修改：添加 isInitialConstruction 参数
  int addBuilding(int type, int level, int gridX, int gridY,
                  BuildingInstance::State state,
                  long long finishTime = 0,
                  bool isInitialConstruction = true);

  void upgradeBuilding(int id, int newLevel, long long finishTime);
  void setBuildingPosition(int id, int gridX, int gridY);
  void setBuildingState(int id, BuildingInstance::State state, long long finishTime = 0);

  bool startUpgradeBuilding(int id);
  void finishNewBuildingConstruction(int id);  // 新增：新建筑建造完成
  void finishUpgradeBuilding(int id);

  void removeBuilding(int buildingId);

  // ========== 网格占用查询 ==========
  bool isAreaOccupied(int startX, int startY, int width, int height, int ignoreBuildingId = -1) const;
  void updateGridOccupancy();

  // ========== 存档/读档 ==========
  void loadFromFile(const std::string& filename);
  void saveToFile(const std::string& filename);

  // ========== 资源生产系统 ==========
  void startResourceProduction();
  void stopResourceProduction();
  void updateResourceProduction(float dt);
  int calculateGoldProductionRate() const;
  int calculateElixirProductionRate() const;
  void processOfflineTime();

private:
  VillageDataManager();
  ~VillageDataManager();

  void notifyResourceChanged();
  void notifyPendingResourceChanged();

  int calculateTotalGoldStorageCapacity() const;
  int calculateTotalElixirStorageCapacity() const;

  static VillageDataManager* _instance;
  VillageData _data;
  int _nextBuildingId;

  std::vector<std::vector<int>> _gridOccupancy;

  ResourceCallback _resourceCallback;

  int _pendingGold;
  int _pendingElixir;
  PendingResourceCallback _pendingResourceCallback;

  long long _lastOnlineTime;
  bool _isProductionRunning;
};