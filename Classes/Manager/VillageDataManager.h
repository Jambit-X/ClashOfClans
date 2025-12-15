#pragma once
#include "../Model/VillageData.h"
#include <functional>
#include <ctime>
#include "../Model/TroopConfig.h"

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
  int getGem() const;
  void addGem(int amount);
  bool spendGem(int amount);

  using ResourceCallback = std::function<void(int gold, int elixir)>;
  void setResourceCallback(ResourceCallback callback);

  // 检查并完成所有到期的建造任务
  void checkAndFinishConstructions();

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
  // 新增：建筑放置完成后的处理
  bool startConstructionAfterPlacement(int buildingId);
  void removeBuilding(int buildingId);

  // ========== 网格占用查询 ==========
  bool isAreaOccupied(int startX, int startY, int width, int height, int ignoreBuildingId = -1) const;
  void updateGridOccupancy();

  // ========== 存档/读档 ==========
  void loadFromFile(const std::string& filename);
  void saveToFile(const std::string& filename);

  // ========== 军队与兵营接口 ==========

  // 获取大本营等级
  int getTownHallLevel() const;

  // 获取兵营数量
  int getArmyCampCount() const;

  // 计算总人口容量 (根据兵营等级 20/30/40)
  int calculateTotalHousingSpace() const;

  // 计算当前已占用人口
  int getCurrentHousingSpace() const;

  // 兵种增删查
  int getTroopCount(int troopId) const;
  void addTroop(int troopId, int count);
  bool removeTroop(int troopId, int count);

  // 获取所有兵种数据(用于UI显示)
  const std::map<int, int>& getAllTroops() const { return _data.troops; }

  // ========== 资源存储容量接口 ==========

/**
 * @brief 获取金币存储上限
 * @return 基础容量(1000) + 所有储金罐的容量总和
 */
  int getGoldStorageCapacity() const;

  /**
   * @brief 获取圣水存储上限
   * @return 基础容量(1000) + 所有圣水瓶的容量总和
   */
  int getElixirStorageCapacity() const;

  // ========== 工人系统 ==========

/**
 * @brief 获取当前拥有的总工人数量
 * @return 已建造完成的建筑工人小屋数量
 */
  int getTotalWorkers() const;

  /**
   * @brief 获取当前正在工作的工人数量
   * @return 所有处于 CONSTRUCTING 状态的建筑数量
   */
  int getBusyWorkerCount() const;

  /**
   * @brief 检查是否有空闲工人
   */
  bool hasIdleWorker() const;

  /**
   * @brief 获取空闲工人数量
   */
  int getIdleWorkerCount() const;

private:
  VillageDataManager();
  ~VillageDataManager();

  void notifyResourceChanged();

  static VillageDataManager* _instance;
  VillageData _data;
  int _nextBuildingId;

  std::vector<std::vector<int>> _gridOccupancy;

  ResourceCallback _resourceCallback;
};