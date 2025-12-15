#include "ResourceProductionSystem.h"
#include "Manager/VillageDataManager.h"
#include "Model/BuildingConfig.h"
#include <algorithm>

USING_NS_CC;

ResourceProductionSystem* ResourceProductionSystem::_instance = nullptr;

ResourceProductionSystem::ResourceProductionSystem()
  : _pendingGold(0)
  , _pendingElixir(0)
  , _isProductionRunning(false) {}

ResourceProductionSystem::~ResourceProductionSystem() {
  stopProduction();
}

ResourceProductionSystem* ResourceProductionSystem::getInstance() {
  if (!_instance) {
    _instance = new ResourceProductionSystem();
  }
  return _instance;
}

void ResourceProductionSystem::destroyInstance() {
  if (_instance) {
    delete _instance;
    _instance = nullptr;
  }
}

void ResourceProductionSystem::startProduction() {
  if (_isProductionRunning) {
    CCLOG("ResourceProductionSystem: Already running");
    return;
  }

  _isProductionRunning = true;

  auto scheduler = Director::getInstance()->getScheduler();
  scheduler->schedule(
    [this](float dt) { this->update(dt); },
    this,
    0.5f,
    CC_REPEAT_FOREVER,
    0.0f,
    false,
    "resource_production_timer"
  );

  CCLOG("ResourceProductionSystem: Started (0.5s interval)");
}

void ResourceProductionSystem::stopProduction() {
  if (!_isProductionRunning) return;

  _isProductionRunning = false;
  auto scheduler = Director::getInstance()->getScheduler();
  scheduler->unschedule("resource_production_timer", this);

  CCLOG("ResourceProductionSystem: Stopped");
}

void ResourceProductionSystem::update(float dt) {
  int goldPerSecond = calculateGoldProductionRate();
  int elixirPerSecond = calculateElixirProductionRate();

  int goldToAdd = (goldPerSecond + 1) / 2;
  int elixirToAdd = (elixirPerSecond + 1) / 2;

  int goldCapacity = getGoldStorageCapacity();
  int elixirCapacity = getElixirStorageCapacity();

  bool resourceAdded = false;

  if (goldToAdd > 0 && _pendingGold < goldCapacity) {
    int actualGoldToAdd = std::min(goldToAdd, goldCapacity - _pendingGold);
    _pendingGold += actualGoldToAdd;
    resourceAdded = true;
  }

  if (elixirToAdd > 0 && _pendingElixir < elixirCapacity) {
    int actualElixirToAdd = std::min(elixirToAdd, elixirCapacity - _pendingElixir);
    _pendingElixir += actualElixirToAdd;
    resourceAdded = true;
  }

  if (resourceAdded) {
    notifyPendingResourceChanged();
  }
}

int ResourceProductionSystem::calculateGoldProductionRate() const {
  int totalRate = 0;
  auto config = BuildingConfig::getInstance();
  auto dataManager = VillageDataManager::getInstance();

  for (const auto& building : dataManager->getAllBuildings()) {
    if (building.state != BuildingInstance::State::BUILT) continue;
    if (building.type != 202) continue; // 金矿

    auto buildingConfig = config->getConfig(building.type);
    if (buildingConfig && buildingConfig->productionRate > 0) {
      float levelMultiplier = 1.0f + 0.2f * (building.level - 1);
      int productionRate = buildingConfig->productionRate * levelMultiplier;
      totalRate += productionRate;
    }
  }

  return totalRate / 3600;
}

int ResourceProductionSystem::calculateElixirProductionRate() const {
  int totalRate = 0;
  auto config = BuildingConfig::getInstance();
  auto dataManager = VillageDataManager::getInstance();

  for (const auto& building : dataManager->getAllBuildings()) {
    if (building.state != BuildingInstance::State::BUILT) continue;
    if (building.type != 203) continue; // 圣水收集器

    auto buildingConfig = config->getConfig(building.type);
    if (buildingConfig && buildingConfig->productionRate > 0) {
      float levelMultiplier = 1.0f + 0.2f * (building.level - 1);
      int productionRate = buildingConfig->productionRate * levelMultiplier;
      totalRate += productionRate;
    }
  }

  return totalRate / 3600;
}

int ResourceProductionSystem::calculateTotalGoldStorageCapacity() const {
  int totalCapacity = 0;
  auto config = BuildingConfig::getInstance();
  auto dataManager = VillageDataManager::getInstance();

  for (const auto& building : dataManager->getAllBuildings()) {
    if (building.state != BuildingInstance::State::BUILT) continue;
    if (building.type != 202) continue;

    auto buildingConfig = config->getConfig(building.type);
    if (buildingConfig && buildingConfig->resourceCapacity > 0) {
      float levelMultiplier = 1.0f + 0.5f * (building.level - 1);
      int capacity = buildingConfig->resourceCapacity * levelMultiplier;
      totalCapacity += capacity;
    }
  }

  return std::max(500, totalCapacity);
}

int ResourceProductionSystem::calculateTotalElixirStorageCapacity() const {
  int totalCapacity = 0;
  auto config = BuildingConfig::getInstance();
  auto dataManager = VillageDataManager::getInstance();

  for (const auto& building : dataManager->getAllBuildings()) {
    if (building.state != BuildingInstance::State::BUILT) continue;
    if (building.type != 203) continue;

    auto buildingConfig = config->getConfig(building.type);
    if (buildingConfig && buildingConfig->resourceCapacity > 0) {
      float levelMultiplier = 1.0f + 0.5f * (building.level - 1);
      int capacity = buildingConfig->resourceCapacity * levelMultiplier;
      totalCapacity += capacity;
    }
  }

  return std::max(500, totalCapacity);
}

int ResourceProductionSystem::getGoldStorageCapacity() const {
  return calculateTotalGoldStorageCapacity();
}

int ResourceProductionSystem::getElixirStorageCapacity() const {
  return calculateTotalElixirStorageCapacity();
}

void ResourceProductionSystem::collectGold() {
  if (_pendingGold > 0) {
    auto dataManager = VillageDataManager::getInstance();
    dataManager->addGold(_pendingGold);
    CCLOG("ResourceProductionSystem: Collected %d gold", _pendingGold);
    _pendingGold = 0;
    notifyPendingResourceChanged();
  }
}

void ResourceProductionSystem::collectElixir() {
  if (_pendingElixir > 0) {
    auto dataManager = VillageDataManager::getInstance();
    dataManager->addElixir(_pendingElixir);
    CCLOG("ResourceProductionSystem: Collected %d elixir", _pendingElixir);
    _pendingElixir = 0;
    notifyPendingResourceChanged();
  }
}

void ResourceProductionSystem::setPendingResourceCallback(PendingResourceCallback callback) {
  _pendingResourceCallback = callback;
}

void ResourceProductionSystem::notifyPendingResourceChanged() {
  if (_pendingResourceCallback) {
    _pendingResourceCallback(_pendingGold, _pendingElixir);
  }
}

void ResourceProductionSystem::processOfflineTime(long long lastOnlineTime) {
  long long currentTime = time(nullptr);

  if (lastOnlineTime == 0) {
    CCLOG("ResourceProductionSystem: First launch");
    return;
  }

  long long offlineSeconds = currentTime - lastOnlineTime;
  if (offlineSeconds < 1) return;

  CCLOG("ResourceProductionSystem: Processing %lld seconds offline", offlineSeconds);

  int goldPerSecond = calculateGoldProductionRate();
  int elixirPerSecond = calculateElixirProductionRate();

  int offlineGold = goldPerSecond * offlineSeconds;
  int offlineElixir = elixirPerSecond * offlineSeconds;

  int goldCapacity = getGoldStorageCapacity();
  int elixirCapacity = getElixirStorageCapacity();

  int goldToAdd = std::min(offlineGold, goldCapacity - _pendingGold);
  int elixirToAdd = std::min(offlineElixir, elixirCapacity - _pendingElixir);

  goldToAdd = std::max(0, goldToAdd);
  elixirToAdd = std::max(0, elixirToAdd);

  if (goldToAdd > 0 || elixirToAdd > 0) {
    _pendingGold += goldToAdd;
    _pendingElixir += elixirToAdd;

    CCLOG("ResourceProductionSystem: Added %d gold, %d elixir", goldToAdd, elixirToAdd);
    notifyPendingResourceChanged();
  }
}