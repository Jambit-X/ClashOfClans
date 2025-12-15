#include "VillageDataManager.h"
#include "../Util/GridMapUtils.h"
#include "../Model/BuildingConfig.h"
#include "../Model/BuildingRequirements.h"
#include <algorithm>
#include "cocos2d.h"
#include "json/document.h"
#include "json/writer.h"
#include "json/stringbuffer.h"

USING_NS_CC;

VillageDataManager* VillageDataManager::_instance = nullptr;

VillageDataManager::VillageDataManager()
  : _nextBuildingId(1) {

  _gridOccupancy.resize(GridMapUtils::GRID_WIDTH);
  for (auto& row : _gridOccupancy) {
    row.resize(GridMapUtils::GRID_HEIGHT, 0);
  }

  _data.gold = 100000;
  _data.elixir = 100000;
  _data.gem = 1000;

  CCLOG("VillageDataManager: Initialized");
}

VillageDataManager::~VillageDataManager() {
  _data.buildings.clear();
  _gridOccupancy.clear();
}

VillageDataManager* VillageDataManager::getInstance() {
  if (!_instance) {
    _instance = new VillageDataManager();
  }
  return _instance;
}

void VillageDataManager::destroyInstance() {
  if (_instance) {
    delete _instance;
    _instance = nullptr;
  }
}

// ========== 资源接口（只保留基础增减）==========

int VillageDataManager::getGold() const {
  return _data.gold;
}

int VillageDataManager::getElixir() const {
  return _data.elixir;
}

void VillageDataManager::addGold(int amount) {
  if (amount <= 0) return;

  int maxCapacity = getGoldStorageCapacity();
  int newAmount = _data.gold + amount;

  if (newAmount > maxCapacity) {
    _data.gold = maxCapacity;  // 限制在上限
    CCLOG("VillageDataManager: Gold capacity reached! Max: %d", maxCapacity);

    // 触发溢出事件，通知UI显示提示
    EventCustom event("EVENT_GOLD_OVERFLOW");
    Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
  } else {
    _data.gold = newAmount;
  }

  notifyResourceChanged();
}

void VillageDataManager::addElixir(int amount) {
  if (amount <= 0) return;

  int maxCapacity = getElixirStorageCapacity();
  int newAmount = _data.elixir + amount;

  if (newAmount > maxCapacity) {
    _data.elixir = maxCapacity;  // 限制在上限
    CCLOG("VillageDataManager: Elixir capacity reached! Max: %d", maxCapacity);

    // 触发溢出事件，通知UI显示提示
    EventCustom event("EVENT_ELIXIR_OVERFLOW");
    Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
  } else {
    _data.elixir = newAmount;
  }

  notifyResourceChanged();
}

bool VillageDataManager::spendGold(int amount) {
  if (_data.gold >= amount) {
    _data.gold -= amount;
    notifyResourceChanged();
    return true;
  }
  return false;
}

bool VillageDataManager::spendElixir(int amount) {
  if (_data.elixir >= amount) {
    _data.elixir -= amount;
    notifyResourceChanged();
    return true;
  }
  return false;
}

// 宝石接口实现
int VillageDataManager::getGem() const {
  return _data.gem;
}

void VillageDataManager::addGem(int amount) {
  _data.gem += amount;
  notifyResourceChanged();
}

bool VillageDataManager::spendGem(int amount) {
  if (_data.gem >= amount) {
    _data.gem -= amount;
    notifyResourceChanged();
    return true;
  }
  return false;
}

void VillageDataManager::setResourceCallback(ResourceCallback callback) {
  _resourceCallback = callback;
}

void VillageDataManager::notifyResourceChanged() {
  if (_resourceCallback) {
    _resourceCallback(_data.gold, _data.elixir);
  }
  Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_RESOURCE_CHANGED");
}


// ========== 军队与兵营接口实现 ==========

int VillageDataManager::getTownHallLevel() const {
    for (const auto& building : _data.buildings) {
        if (building.type == 1) { // 假设大本营 ID 为 1
            return building.level;
        }
    }
    return 1; // 默认1级
}

int VillageDataManager::getArmyCampCount() const {
    int count = 0;
    for (const auto& building : _data.buildings) {
        // 排除放置中的兵营，只计算已存在的
        if (building.type == 101 && building.state != BuildingInstance::State::PLACING) {
            count++;
        }
    }
    return count;
}

int VillageDataManager::calculateTotalHousingSpace() const {
    int totalSpace = 0;
    for (const auto& building : _data.buildings) {
        // 兵营 ID = 101, 且不是放置中状态
        if (building.type == 101 && building.state != BuildingInstance::State::PLACING) {
            // 规则：1级=20, 2级=30, 3级=40
            // 公式：10 + 等级 * 10
            int space = 10 + (building.level * 10);
            totalSpace += space;
        }
    }
    return totalSpace;
}

int VillageDataManager::getCurrentHousingSpace() const {
    int usedSpace = 0;
    auto troopConfig = TroopConfig::getInstance();

    for (const auto& pair : _data.troops) {
        int troopId = pair.first;
        int count = pair.second;

        TroopInfo info = troopConfig->getTroopById(troopId);
        usedSpace += info.housingSpace * count;
    }
    return usedSpace;
}

int VillageDataManager::getTroopCount(int troopId) const {
    auto it = _data.troops.find(troopId);
    if (it != _data.troops.end()) {
        return it->second;
    }
    return 0;
}

void VillageDataManager::addTroop(int troopId, int count) {
    if (count <= 0) return;
    _data.troops[troopId] += count;
    saveToFile("village.json"); // 立即保存
}

bool VillageDataManager::removeTroop(int troopId, int count) {
    if (count <= 0) return false;

    auto it = _data.troops.find(troopId);
    if (it == _data.troops.end() || it->second < count) {
        return false;
    }

    it->second -= count;
    if (it->second <= 0) {
        _data.troops.erase(it);
    }

    saveToFile("village.json"); // 立即保存
    return true;
}

// ========== 建筑接口 ==========

const std::vector<BuildingInstance>& VillageDataManager::getAllBuildings() const {
  return _data.buildings;
}

BuildingInstance* VillageDataManager::getBuildingById(int id) {
  for (auto& building : _data.buildings) {
    if (building.id == id) {
      return &building;
    }
  }
  return nullptr;
}

int VillageDataManager::addBuilding(int type, int level, int gridX, int gridY,
                                    BuildingInstance::State state,
                                    long long finishTime,
                                    bool isInitialConstruction) {
  BuildingInstance building;
  building.id = _nextBuildingId++;
  building.type = type;
  building.level = level;
  building.gridX = gridX;
  building.gridY = gridY;
  building.state = state;
  building.finishTime = finishTime;
  building.isInitialConstruction = isInitialConstruction;  // 设置标志

  _data.buildings.push_back(building);

  if (state != BuildingInstance::State::PLACING) {
    updateGridOccupancy();
  }

  CCLOG("VillageDataManager: Added building ID=%d, type=%d, level=%d, initial=%s",
        building.id, type, level, isInitialConstruction ? "YES" : "NO");

  return building.id;
}

void VillageDataManager::upgradeBuilding(int id, int newLevel, long long finishTime) {
  auto* building = getBuildingById(id);
  if (building) {
    building->level = newLevel;
    building->state = BuildingInstance::State::CONSTRUCTING;
    building->finishTime = finishTime;

    CCLOG("VillageDataManager: Building ID=%d upgraded to level %d", id, newLevel);
  }
}

void VillageDataManager::setBuildingPosition(int id, int gridX, int gridY) {
  auto* building = getBuildingById(id);
  if (building) {
    building->gridX = gridX;
    building->gridY = gridY;
    updateGridOccupancy();

    CCLOG("VillageDataManager: Building ID=%d moved to grid(%d, %d)", id, gridX, gridY);
    
    saveToFile("village.json");  // 立即保存位置变更
  }
}

void VillageDataManager::setBuildingState(int id, BuildingInstance::State state, long long finishTime) {
  auto* building = getBuildingById(id);
  if (building) {
    building->state = state;
    building->finishTime = finishTime;
    updateGridOccupancy();

    CCLOG("VillageDataManager: Building ID=%d state changed to %d", id, (int)state);
  }
}

bool VillageDataManager::startUpgradeBuilding(int id) {
  auto* building = getBuildingById(id);
  if (!building) {
    CCLOG("VillageDataManager: Building %d not found", id);
    return false;
  }

  auto config = BuildingConfig::getInstance();
  auto configData = config->getConfig(building->type);

  if (!configData) {
    CCLOG("VillageDataManager: Config not found for building type %d", building->type);
    return false;
  }

  // 检查最大等级
  if (building->level >= 3) {
    CCLOG("VillageDataManager: Building %d already at max level (3)", id);
    return false;
  }

  // 获取升级成本
  int cost = config->getUpgradeCost(building->type, building->level);

  // 改为英文判断
  bool success = false;
  if (configData->costType == "gold") {  // 英文
    success = spendGold(cost);
  } else if (configData->costType == "elixir") {  // 英文
    success = spendElixir(cost);
  } else if (configData->costType == "gem") {  // 新增宝石类型
    success = spendGem(cost);
  }

  if (!success) {
    CCLOG("VillageDataManager: Not enough resources to upgrade");
    return false;
  }

  // 开始升级
  long long currentTime = time(nullptr);
  long long finishTime = currentTime + configData->buildTimeSeconds;
  building->state = BuildingInstance::State::CONSTRUCTING;
  building->finishTime = finishTime;
  building->isInitialConstruction = false;  // 升级不是首次建造

  CCLOG("VillageDataManager: Started upgrade for building %d (level %d → %d), finish at %lld",
        id, building->level, building->level + 1, finishTime);

  saveToFile("village.json");

  // ========== 触发建造开始事件 ==========
  EventCustom event("EVENT_CONSTRUCTION_STARTED");
  int* data = new int(id);
  event.setUserData(data);
  Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
  delete data;

  return true;
}

// 新建筑建造完成
void VillageDataManager::finishNewBuildingConstruction(int id) {
  auto* building = getBuildingById(id);
  if (!building) {
    CCLOG("VillageDataManager: Building %d not found", id);
    return;
  }

  // 核心修复：新建筑从 0 级升到 1 级
  building->level++;
  building->state = BuildingInstance::State::BUILT;
  building->finishTime = 0;
  building->isInitialConstruction = false;

  CCLOG("VillageDataManager: New building %d construction complete (level=%d)", id, building->level);

  saveToFile("village.json");

  // 如果是存储建筑建造完成，触发资源显示更新
  if (building->type == 204 || building->type == 205) {
    CCLOG("VillageDataManager: Storage building constructed, refreshing resource display");
    notifyResourceChanged();
  }

  Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(
    "EVENT_BUILDING_CONSTRUCTED", &id);
}

void VillageDataManager::finishUpgradeBuilding(int id) {
  auto* building = getBuildingById(id);
  if (!building) {
    CCLOG("VillageDataManager: Building %d not found", id);
    return;
  }

  int oldLevel = building->level;
  building->level++;  // 升级：等级 +1
  building->state = BuildingInstance::State::BUILT;
  building->finishTime = 0;

  CCLOG("VillageDataManager: Building %d upgraded from level %d to %d", id, oldLevel, building->level);

  // 如果是大本营升级，触发特殊事件
  if (building->type == 1) {  // 大本营 ID = 1
    CCLOG("VillageDataManager: Town Hall upgraded to level %d!", building->level);
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_TOWNHALL_UPGRADED");
  }
  // 如果是存储建筑升级，触发资源显示更新
  if (building->type == 204 || building->type == 205) {
    CCLOG("VillageDataManager: Storage building upgraded, refreshing resource display");
    notifyResourceChanged();
  }

  saveToFile("village.json");

  Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(
    "EVENT_BUILDING_UPGRADED", &id);
}

// ========== 建筑放置完成后的处理 ==========
bool VillageDataManager::startConstructionAfterPlacement(int buildingId) {
  auto building = getBuildingById(buildingId);
  if (!building) {
    CCLOG("VillageDataManager: Building %d not found", buildingId);
    return false;
  }

  if (building->state != BuildingInstance::State::PLACING) {
    CCLOG("VillageDataManager: Building %d is not in PLACING state", buildingId);
    return false;
  }

  // ========== 特殊处理：建筑工人小屋瞬间完成 ==========
  if (building->type == 201) {
    CCLOG("VillageDataManager: Builder hut completing instantly after placement");

    building->state = BuildingInstance::State::BUILT;
    building->finishTime = 0;
    saveToFile("village.json");

    // 触发完成事件
    EventCustom event("EVENT_BUILDING_UPGRADED");
    int* data = new int(buildingId);
    event.setUserData(data);
    Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
    delete data;

    return true;
  }
  // ====================================================

  // 其他建筑正常建造流程
  auto config = BuildingConfig::getInstance()->getConfig(building->type);
  if (!config) {
    CCLOG("VillageDataManager: Config not found for building type %d", building->type);
    return false;
  }

  building->state = BuildingInstance::State::CONSTRUCTING;
  building->finishTime = time(nullptr) + config->buildTimeSeconds;

  saveToFile("village.json");

  // ========== 触发建造开始事件 ==========
  EventCustom event("EVENT_CONSTRUCTION_STARTED");
  int* data = new int(buildingId);
  event.setUserData(data);
  Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
  delete data;

  CCLOG("VillageDataManager: Construction started for building %d, duration=%d seconds",
        buildingId, config->buildTimeSeconds);
  return true;
}

// ========== 网格占用查询 ==========

bool VillageDataManager::isAreaOccupied(int startX, int startY, int width, int height, int ignoreBuildingId) const {
  if (startX < 0 || startY < 0 ||
      startX + width > GridMapUtils::GRID_WIDTH ||
      startY + height > GridMapUtils::GRID_HEIGHT) {
    return true;
  }

  for (int x = startX; x < startX + width; ++x) {
    for (int y = startY; y < startY + height; ++y) {
      int occupyingId = _gridOccupancy[x][y];

      if (occupyingId != 0 && occupyingId != ignoreBuildingId) {
        return true;
      }
    }
  }

  return false;
}

void VillageDataManager::updateGridOccupancy() {
  for (auto& row : _gridOccupancy) {
    std::fill(row.begin(), row.end(), 0);
  }

  for (const auto& building : _data.buildings) {
    if (building.state == BuildingInstance::State::PLACING) {
      continue;
    }

    auto config = BuildingConfig::getInstance()->getConfig(building.type);
    if (!config) continue;

    for (int x = building.gridX; x < building.gridX + config->gridWidth; ++x) {
      for (int y = building.gridY; y < building.gridY + config->gridHeight; ++y) {
        if (x >= 0 && x < GridMapUtils::GRID_WIDTH &&
            y >= 0 && y < GridMapUtils::GRID_HEIGHT) {
          _gridOccupancy[x][y] = building.id;
        }
      }
    }
  }

  CCLOG("VillageDataManager: Grid occupancy table updated");
}

void VillageDataManager::removeBuilding(int buildingId) {
  auto it = std::find_if(_data.buildings.begin(), _data.buildings.end(),
                         [buildingId](const BuildingInstance& b) {
    return b.id == buildingId;
  });

  if (it != _data.buildings.end()) {
    CCLOG("VillageDataManager: Removing building ID=%d", buildingId);
    _data.buildings.erase(it);
    updateGridOccupancy();  // 更新网格占用
  } else {
    CCLOG("VillageDataManager: Building ID=%d not found", buildingId);
  }
}

// 新增方法：检查并完成所有到期的建造
void VillageDataManager::checkAndFinishConstructions() {
  long long currentTime = time(nullptr);
  std::vector<int> finishedBuildings;  // 收集需要完成的建筑 ID

  // 第一遍：找出所有完成的建筑
  for (const auto& building : _data.buildings) {
    if (building.state == BuildingInstance::State::CONSTRUCTING &&
        building.finishTime > 0 &&
        currentTime >= building.finishTime) {
      finishedBuildings.push_back(building.id);
    }
  }

  // 第二遍：完成建造（避免迭代中修改容器）
  for (int buildingId : finishedBuildings) {
    auto* building = getBuildingById(buildingId);
    if (!building) continue;

    if (building->isInitialConstruction) {
      CCLOG("VillageDataManager: Auto-completing NEW building construction ID=%d", buildingId);
      finishNewBuildingConstruction(buildingId);
    } else {
      CCLOG("VillageDataManager: Auto-completing building UPGRADE ID=%d", buildingId);
      finishUpgradeBuilding(buildingId);
    }
  }
}



void VillageDataManager::saveToFile(const std::string& filename) {
  rapidjson::Document doc;
  doc.SetObject();
  auto& allocator = doc.GetAllocator();

  doc.AddMember("gold", _data.gold, allocator);
  doc.AddMember("elixir", _data.elixir, allocator);
  doc.AddMember("gem", _data.gem, allocator);
  // --- 保存军队数据 ---
  rapidjson::Value troopsArray(rapidjson::kArrayType);
  for (const auto& pair : _data.troops) {
      rapidjson::Value troopObj(rapidjson::kObjectType);
      troopObj.AddMember("id", pair.first, allocator);
      troopObj.AddMember("count", pair.second, allocator);
      troopsArray.PushBack(troopObj, allocator);
  }
  doc.AddMember("troops", troopsArray, allocator);

  // 保存建筑数据
  rapidjson::Value buildingsArray(rapidjson::kArrayType);
  for (const auto& building : _data.buildings) {
    rapidjson::Value buildingObj(rapidjson::kObjectType);
    buildingObj.AddMember("id", building.id, allocator);
    buildingObj.AddMember("type", building.type, allocator);
    buildingObj.AddMember("level", building.level, allocator);
    buildingObj.AddMember("gridX", building.gridX, allocator);
    buildingObj.AddMember("gridY", building.gridY, allocator);
    buildingObj.AddMember("state", (int)building.state, allocator);
    buildingObj.AddMember("finishTime", building.finishTime, allocator);
    buildingObj.AddMember("isInitialConstruction", building.isInitialConstruction, allocator);
    buildingsArray.PushBack(buildingObj, allocator);
  }
  doc.AddMember("buildings", buildingsArray, allocator);

  // 写入文件
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  doc.Accept(writer);

  auto fileUtils = FileUtils::getInstance();
  std::string writablePath = fileUtils->getWritablePath();
  std::string fullPath = writablePath + filename;

  bool success = fileUtils->writeStringToFile(buffer.GetString(), fullPath);

  if (success) {
    CCLOG("VillageDataManager: Saved to %s", fullPath.c_str());
  } else {
    CCLOG("VillageDataManager: ERROR - Failed to save");
  }
}

void VillageDataManager::loadFromFile(const std::string& filename) {
  auto fileUtils = FileUtils::getInstance();
  std::string writablePath = fileUtils->getWritablePath();
  std::string fullPath = writablePath + filename;

  if (!fileUtils->isFileExist(fullPath)) {
    CCLOG("VillageDataManager: Save file not found, initializing default game");

    // ==========  初始化默认建筑 ==========
    _data.buildings.clear();

    // 1. 大本营（放在地图中心）
    // 44x44 网格，中心点为 (22, 22)
    // 大本营是 4x4 建筑，左下角应该放在 (20, 20)
    BuildingInstance townHall;
    townHall.id = _nextBuildingId++;
    townHall.type = 1;
    townHall.gridX = 20;  // 中心 (22, 22) - 宽度的一半 (2)
    townHall.gridY = 20;  // 中心 (22, 22) - 高度的一半 (2)
    townHall.level = 1;
    townHall.state = BuildingInstance::State::BUILT;
    townHall.finishTime = 0;
    townHall.isInitialConstruction = false;
    _data.buildings.push_back(townHall);

    CCLOG("VillageDataManager: Town Hall created at grid (%d, %d), center at (22, 22)",
          townHall.gridX, townHall.gridY);

    // 2. 建筑工人小屋（放在大本营左侧）
    // 工人小屋是 2x2 建筑
    BuildingInstance builderHut;
    builderHut.id = _nextBuildingId++;
    builderHut.type = 201;
    builderHut.gridX = 16;  // 大本营左侧 (20 - 2 - 2 = 16)
    builderHut.gridY = 20;  // 与大本营同一水平线
    builderHut.level = 1;
    builderHut.state = BuildingInstance::State::BUILT;
    builderHut.finishTime = 0;
    builderHut.isInitialConstruction = false;
    _data.buildings.push_back(builderHut);

    CCLOG("VillageDataManager: Builder Hut created at grid (%d, %d)",
          builderHut.gridX, builderHut.gridY);

    // 更新网格占用
    updateGridOccupancy();

    // 保存初始配置
    saveToFile(filename);

    CCLOG("VillageDataManager: Default game initialized with 2 buildings");
    CCLOG("VillageDataManager: Initial game state saved to %s", fullPath.c_str());
    // =============================================
    return;
  }

  // ========== 加载已有存档 ==========
  std::string content = fileUtils->getStringFromFile(fullPath);
  if (content.empty()) {
    CCLOG("VillageDataManager: Failed to read save file");
    return;
  }

  rapidjson::Document doc;
  doc.Parse(content.c_str());

  if (doc.HasParseError()) {
    CCLOG("VillageDataManager: JSON parse error");
    return;
  }
  // 读取资源
  if (doc.HasMember("gold") && doc["gold"].IsInt()) {
    _data.gold = doc["gold"].GetInt();
  }
  if (doc.HasMember("elixir") && doc["elixir"].IsInt()) {
    _data.elixir = doc["elixir"].GetInt();
  }
  if (doc.HasMember("gem") && doc["gem"].IsInt()) {
    _data.gem = doc["gem"].GetInt();  
  }

  // --- 读取军队数据 ---
  _data.troops.clear();
  if (doc.HasMember("troops") && doc["troops"].IsArray()) {
      const auto& troopsArray = doc["troops"];
      for (rapidjson::SizeType i = 0; i < troopsArray.Size(); i++) {
          const auto& obj = troopsArray[i];
          int id = obj["id"].GetInt();
          int count = obj["count"].GetInt();
          _data.troops[id] = count;
      }
  }
  //  读取建筑数据
  _data.buildings.clear();
  if (doc.HasMember("buildings") && doc["buildings"].IsArray()) {
    const auto& buildingsArray = doc["buildings"];
    for (rapidjson::SizeType i = 0; i < buildingsArray.Size(); i++) {
      const auto& buildingObj = buildingsArray[i];

      BuildingInstance building;
      building.id = buildingObj["id"].GetInt();
      building.type = buildingObj["type"].GetInt();
      building.level = buildingObj["level"].GetInt();
      building.gridX = buildingObj["gridX"].GetInt();
      building.gridY = buildingObj["gridY"].GetInt();
      building.state = (BuildingInstance::State)buildingObj["state"].GetInt();
      building.finishTime = buildingObj["finishTime"].GetInt64();
      
      // 加载新字段（兼容旧存档）
      if (buildingObj.HasMember("isInitialConstruction")) {
        building.isInitialConstruction = buildingObj["isInitialConstruction"].GetBool();
      } else {
        building.isInitialConstruction = false;
      }

      _data.buildings.push_back(building);

      if (building.id >= _nextBuildingId) {
        _nextBuildingId = building.id + 1;
      }
    }
  }
  // 4. 更新后续状态
  updateGridOccupancy();
  notifyResourceChanged();

  CCLOG("VillageDataManager: Loaded %lu buildings and %lu troop types",
      _data.buildings.size(), _data.troops.size());
}

// ========== 工人系统实现 ==========

int VillageDataManager::getTotalWorkers() const {
  int workerCount = 0;

  // 统计所有已建成的建筑工人小屋（ID=201, State=BUILT）
  for (const auto& building : _data.buildings) {
    if (building.type == 201 && building.state == BuildingInstance::State::BUILT) {
      workerCount++;
    }
  }

  // 直接返回工人小屋数量（不额外加1）
  return workerCount;
}

int VillageDataManager::getBusyWorkerCount() const {
  int busyCount = 0;

  // 统计所有正在建造/升级的建筑
  for (const auto& building : _data.buildings) {
    if (building.state == BuildingInstance::State::CONSTRUCTING) {
      busyCount++;
    }
  }

  return busyCount;
}

bool VillageDataManager::hasIdleWorker() const {
  return getIdleWorkerCount() > 0;
}

int VillageDataManager::getIdleWorkerCount() const {
  int total = getTotalWorkers();
  int busy = getBusyWorkerCount();
  int idle = total - busy;

  // 防止负数
  return (idle < 0) ? 0 : idle;
}

// ========== 资源存储容量实现 ==========

int VillageDataManager::getGoldStorageCapacity() const {
  const int BASE_CAPACITY = 100000;  // 基础容量
  int totalCapacity = BASE_CAPACITY;

  auto config = BuildingConfig::getInstance();

  // 统计所有已建成的储金罐（ID=204）
  for (const auto& building : _data.buildings) {
    if (building.type == 204 && building.state == BuildingInstance::State::BUILT) {
      int capacity = config->getStorageCapacityByLevel(204, building.level);
      totalCapacity += capacity;
      CCLOG("VillageDataManager: Gold Storage (level %d) adds %d capacity",
            building.level, capacity);
    }
  }

  CCLOG("VillageDataManager: Total gold capacity = %d", totalCapacity);
  return totalCapacity;
}

int VillageDataManager::getElixirStorageCapacity() const {
  const int BASE_CAPACITY = 100000;  // 基础容量
  int totalCapacity = BASE_CAPACITY;

  auto config = BuildingConfig::getInstance();

  // 统计所有已建成的圣水瓶（ID=205）
  for (const auto& building : _data.buildings) {
    if (building.type == 205 && building.state == BuildingInstance::State::BUILT) {
      int capacity = config->getStorageCapacityByLevel(205, building.level);
      totalCapacity += capacity;
      CCLOG("VillageDataManager: Elixir Storage (level %d) adds %d capacity",
            building.level, capacity);
    }
  }

  CCLOG("VillageDataManager: Total elixir capacity = %d", totalCapacity);
  return totalCapacity;
}