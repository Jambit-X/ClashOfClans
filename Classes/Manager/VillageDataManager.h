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
	
	// 新增：获取收集上限
	int getGoldStorageCapacity() const;
	int getElixirStorageCapacity() const;
	
	using PendingResourceCallback = std::function<void(int pendingGold, int pendingElixir)>;
	void setPendingResourceCallback(PendingResourceCallback callback);

	// ========== 建筑接口 ==========
	const std::vector<BuildingInstance>& getAllBuildings() const;
	BuildingInstance* getBuildingById(int id);
	int addBuilding(int type, int level, int gridX, int gridY, BuildingInstance::State state, long long finishTime = 0);
	void upgradeBuilding(int id, int newLevel, long long finishTime);
	void setBuildingPosition(int id, int gridX, int gridY);
	void setBuildingState(int id, BuildingInstance::State state, long long finishTime = 0);

	bool startUpgradeBuilding(int id);
	void finishUpgradeBuilding(int id);

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
	
	// 新增：处理离线时间
	void processOfflineTime();

	// 在 VillageDataManager 类中添加
	void removeBuilding(int buildingId);

private:
	VillageDataManager();
	~VillageDataManager();

	void notifyResourceChanged();
	void notifyPendingResourceChanged();
	
	// 新增：计算总收集容量
	int calculateTotalGoldStorageCapacity() const;
	int calculateTotalElixirStorageCapacity() const;

	static VillageDataManager* _instance;
	VillageData _data;
	int _nextBuildingId;

	std::vector<std::vector<int>> _gridOccupancy;

	ResourceCallback _resourceCallback;
	
	// 待收集资源
	int _pendingGold;
	int _pendingElixir;
	PendingResourceCallback _pendingResourceCallback;
	
	// 新增：离线时间记录
	long long _lastOnlineTime;  // 上次在线时间戳（秒）
	
	bool _isProductionRunning;
};