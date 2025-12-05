#ifndef __BUILDING_DATA_MANAGER_H__
#define __BUILDING_DATA_MANAGER_H__

#include "cocos2d.h"
#include "../Model/Building.h"
#include <map>
#include <vector>

class BuildingDataManager {
public:
  static BuildingDataManager* getInstance();
  static void destroyInstance();

  // 建筑管理
  void addBuilding(Building* building);
  void removeBuilding(int buildingId);
  Building* getBuildingById(int buildingId);
  std::vector<Building*> getAllBuildings();
  std::vector<Building*> getResourceProducers();

  // 数据持久化
  //std::vector<BuildingSaveData> getSaveData();
  //void loadFromSaveData(const std::vector<BuildingSaveData>& data);

  // 离线计算
  void calculateOfflineProduction(long long offlineSeconds);

private:
  BuildingDataManager();
  ~BuildingDataManager();

  static BuildingDataManager* _instance;
  std::map<int, Building*> _buildings;
  int _nextBuildingId;
};

#endif