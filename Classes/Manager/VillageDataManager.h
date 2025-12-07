#pragma once
#include "../Model/VillageData.h"

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

	// 建筑接口
	const std::vector<BuildingInstance>& getAllBuildings() const;
	BuildingInstance* getBuildingById(int id);
	int addBuilding(int type, int level, int gridX, int gridY, BuildingInstance::State state, long long finishTime = 0);
	void upgradeBuilding(int id, int newLevel, long long finishTime);
	void setBuildingPosition(int id, int gridX, int gridY);
	void setBuildingState(int id, BuildingInstance::State state, long long finishTime = 0);

	// 网格占用查询（性能优化：O(1) 查询）
	bool isAreaOccupied(int startX, int startY, int width, int height, int ignoreBuildingId = -1) const;
	void updateGridOccupancy();  // 重建网格占用表
	// 存档/读档
	void loadFromFile(const std::string& filename);
	void saveToFile(const std::string& filename);

private:
	VillageDataManager();
	~VillageDataManager();

	// 新增：通知资源变化
	void notifyResourceChanged();

	static VillageDataManager* _instance;
	VillageData _data;
	int _nextBuildingId;

	// 网格占用表：存储每个格子被哪个建筑ID占用（0表示空闲）
	// 尺寸：44x44（与 GridMapUtils::GRID_WIDTH/HEIGHT 一致）
	std::vector<std::vector<int>> _gridOccupancy;
};
