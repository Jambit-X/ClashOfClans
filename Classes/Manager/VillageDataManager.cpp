#include "VillageDataManager.h"
#include "../Util/GridMapUtils.h"
#include "../Model/BuildingConfig.h"
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
  _data.gem = 100;

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
  _data.gold += amount;
  notifyResourceChanged();
}

void VillageDataManager::addElixir(int amount) {
  _data.elixir += amount;
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

  // 检查是否已达到最大等级（3级）
  if (building->level >= 3) {
    CCLOG("VillageDataManager: Building %d already at max level (3)", id);
    return false;
  }

  if (!config->canUpgrade(building->type, building->level)) {
    CCLOG("VillageDataManager: Building %d already max level", id);
    return false;
  }

  int cost = config->getUpgradeCost(building->type, building->level);

  bool success = false;
  if (configData->costType == "金币") {
    success = spendGold(cost);
  } else if (configData->costType == "圣水") {
    success = spendElixir(cost);
  }

  if (!success) {
    CCLOG("VillageDataManager: Not enough resources to upgrade");
    return false;
  }

  long long currentTime = time(nullptr);
  long long finishTime = currentTime + configData->buildTimeSeconds;
  building->state = BuildingInstance::State::CONSTRUCTING;
  building->finishTime = finishTime;
  building->isInitialConstruction = false;  // 升级不是首次建造

  CCLOG("VillageDataManager: Started upgrade for building %d, finish at %lld", id, finishTime);

  saveToFile("village.json");
  return true;
}

// 新增：新建筑建造完成
void VillageDataManager::finishNewBuildingConstruction(int id) {
  auto* building = getBuildingById(id);
  if (!building) return;

  building->state = BuildingInstance::State::BUILT;
  building->finishTime = 0;
  building->isInitialConstruction = false;

  CCLOG("VillageDataManager: New building %d construction complete (level=%d)", id, building->level);

  saveToFile("village.json");

  Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(
    "EVENT_BUILDING_CONSTRUCTED", &id);
}

void VillageDataManager::finishUpgradeBuilding(int id) {
  auto* building = getBuildingById(id);
  if (!building) return;

  building->level++;  // 升级才+1
  building->state = BuildingInstance::State::BUILT;
  building->finishTime = 0;

  CCLOG("VillageDataManager: Building %d upgraded to level %d", id, building->level);

  saveToFile("village.json");

  Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(
    "EVENT_BUILDING_UPGRADED", &id);
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
    CCLOG("VillageDataManager: Save file not found, using defaults");
    return;
  }

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
    _data.gem = doc["gem"].GetInt();  // 新增
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