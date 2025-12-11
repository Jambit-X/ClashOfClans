#include "BuildingConfig.h"
#include <cmath>
#include <../proj.win32/Constants.h>

USING_NS_CC;

// 静态成员初始化
BuildingConfig* BuildingConfig::_instance = nullptr;

BuildingConfig::BuildingConfig() {
  initConfigs();
}

BuildingConfig::~BuildingConfig() {
  _configs.clear();
}

BuildingConfig* BuildingConfig::getInstance() {
  if (!_instance) {
    _instance = new BuildingConfig();
  }
  return _instance;
}

void BuildingConfig::destroyInstance() {
  if (_instance) {
    delete _instance;
    _instance = nullptr;
  }
}

void BuildingConfig::initConfigs() {
  // 1: 大本营 (Town Hall)
  BuildingConfigData townHall;
  townHall.type = 1;
  townHall.name = "大本营";
  townHall.category = "核心";
  townHall.spritePathTemplate = "buildings/Town_Hall/Town_Hall{level}.png";  // 从 .webp 改为 .png
  townHall.gridWidth = 4;
  townHall.gridHeight = 4;
  townHall.anchorOffset = Vec2(25, -50);
  townHall.maxLevel = 13;
  townHall.initialCost = 0;
  townHall.costType = "金币";
  townHall.buildTimeSeconds = 0;
  townHall.hitPoints = 1500;
  townHall.damagePerSecond = 0;
  townHall.attackRange = 0;
  townHall.resourceCapacity = 0;
  townHall.productionRate = 0;
  _configs[1] = townHall;
  
  // ========== 军事建筑 ========= =
  // 101: 兵营
  BuildingConfigData armyCamp;
  armyCamp.type = 101;
  armyCamp.name = "兵营";
  armyCamp.category = "军事";
  armyCamp.spritePathTemplate = "buildings/military_architecture/army_camp/Army_Camp{level}.png";
  armyCamp.gridWidth = 3;
  armyCamp.gridHeight = 3;
  armyCamp.anchorOffset = Vec2(44, -30);
  armyCamp.maxLevel = 10;
  armyCamp.initialCost = 250;
  armyCamp.costType = "圣水";
  armyCamp.buildTimeSeconds = 300;
  armyCamp.hitPoints = 0;
  armyCamp.damagePerSecond = 0;
  armyCamp.attackRange = 0;
  armyCamp.resourceCapacity = 0;
  armyCamp.productionRate = 0;
  _configs[101] = armyCamp;

  // 102: 训练营
  BuildingConfigData barracks;
  barracks.type = 102;
  barracks.name = "训练营";
  barracks.category = "军事";
  barracks.spritePathTemplate = "buildings/military_architecture/barracks/Barracks{level}.png";
  barracks.gridWidth = 3;
  barracks.gridHeight = 3;
  barracks.anchorOffset = Vec2(36, -30);
  barracks.maxLevel = 13;
  barracks.initialCost = 200;
  barracks.costType = "圣水";
  barracks.buildTimeSeconds = 60;
  barracks.hitPoints = 0;
  barracks.damagePerSecond = 0;
  barracks.attackRange = 0;
  barracks.resourceCapacity = 0;
  barracks.productionRate = 0;
  _configs[102] = barracks;

  // 103: 实验室
  BuildingConfigData laboratory;
  laboratory.type = 103;
  laboratory.name = "实验室";
  laboratory.category = "军事";
  laboratory.spritePathTemplate = "buildings/military_architecture/laboratory/Laboratory{level}.png";
  laboratory.gridWidth = 3;
  laboratory.gridHeight = 3;
  laboratory.anchorOffset = Vec2(3, -40);
  laboratory.maxLevel = 10;
  laboratory.initialCost = 500;
  laboratory.costType = "圣水";
  laboratory.buildTimeSeconds = 1800;
  laboratory.hitPoints = 0;
  laboratory.damagePerSecond = 0;
  laboratory.attackRange = 0;
  laboratory.resourceCapacity = 0;
  laboratory.productionRate = 0;
  _configs[103] = laboratory;

  // ========== 资源建筑 =========
  // 202: 金矿
  BuildingConfigData goldMine;
  goldMine.type = 202;
  goldMine.name = "金矿";
  goldMine.category = "资源";
  goldMine.spritePathTemplate = "buildings/resource_architecture/gold_mine/Gold_Mine{level}.png";
  goldMine.gridWidth = 3;
  goldMine.gridHeight = 3;
  goldMine.anchorOffset = Vec2(32, -50);
  goldMine.maxLevel = 14;
  goldMine.initialCost = 150;
  goldMine.costType = "圣水";
  goldMine.buildTimeSeconds = 60;
  goldMine.hitPoints = 400;
  goldMine.damagePerSecond = 0;
  goldMine.attackRange = 0;
  goldMine.resourceCapacity = 500;
  goldMine.productionRate = 7200;  // 保持 7200（2金币/秒）
  _configs[202] = goldMine;

  // 203: 圣水收集器
  BuildingConfigData elixirCollector;
  elixirCollector.type = 203;
  elixirCollector.name = "圣水收集器";
  elixirCollector.category = "资源";
  elixirCollector.spritePathTemplate = "buildings/resource_architecture/elixir_collector/Elixir_Collector{level}.png";
  elixirCollector.gridWidth = 3;
  elixirCollector.gridHeight = 3;
  elixirCollector.anchorOffset = Vec2(56*1.5f - 60, -42);
  elixirCollector.maxLevel = 14;
  elixirCollector.initialCost = 150;
  elixirCollector.costType = "金币";
  elixirCollector.buildTimeSeconds = 60;
  elixirCollector.hitPoints = 400;
  elixirCollector.damagePerSecond = 0;
  elixirCollector.attackRange = 0;
  elixirCollector.resourceCapacity = 500;
  elixirCollector.productionRate = 7200;  // 改为 7200（2圣水/秒）
  _configs[203] = elixirCollector;

  // ========== 防御建筑 =========
  // 301: 加农炮
  BuildingConfigData cannon;
  cannon.type = 301;
  cannon.name = "加农炮";
  cannon.category = "防御";
  cannon.spritePathTemplate = "buildings/defence_architecture/cannon/Cannon_lvl{level}.png";
  cannon.gridWidth = 3;
  cannon.gridHeight = 3;
  cannon.anchorOffset = Vec2(62, -6);
  cannon.maxLevel = 18;
  cannon.initialCost = 250;
  cannon.costType = "金币";
  cannon.buildTimeSeconds = 60;
  cannon.hitPoints = 620;
  cannon.damagePerSecond = 11;
  cannon.attackRange = 9;
  cannon.resourceCapacity = 0;
  cannon.productionRate = 0;
  _configs[301] = cannon;

  // 302: 箭塔
  BuildingConfigData archerTower;
  archerTower.type = 302;
  archerTower.name = "箭塔";
  archerTower.category = "防御";
  archerTower.spritePathTemplate = "buildings/defence_architecture/archer_tower/Archer_Tower{level}.png";
  archerTower.gridWidth = 3;
  archerTower.gridHeight = 3;
  archerTower.anchorOffset = Vec2(34, -30);
  archerTower.maxLevel = 18;
  archerTower.initialCost = 1000;
  archerTower.costType = "金币";
  archerTower.buildTimeSeconds = 900;
  archerTower.hitPoints = 380;
  archerTower.damagePerSecond = 11;
  archerTower.attackRange = 10;
  archerTower.resourceCapacity = 0;
  archerTower.productionRate = 0;
  _configs[302] = archerTower;

  // 303: 城墙
  BuildingConfigData wall;
  wall.type = 303;
  wall.name = "城墙";
  wall.category = "防御";
  wall.spritePathTemplate = "buildings/defence_architecture/wall/Wall{level}.png";
  wall.gridWidth = 1;
  wall.gridHeight = 1;
  wall.anchorOffset = Vec2(5, -7);
  wall.maxLevel = 14;
  wall.initialCost = 50;
  wall.costType = "金币";
  wall.buildTimeSeconds = 0;
  wall.hitPoints = 300;
  wall.damagePerSecond = 0;
  wall.attackRange = 0;
  wall.resourceCapacity = 0;
  wall.productionRate = 0;
  _configs[303] = wall;

  CCLOG("BuildingConfig: Initialized %lu building configs", _configs.size());
  CCLOG("BuildingConfig: Town Hall sprite path template: %s", townHall.spritePathTemplate.c_str());
}

const BuildingConfigData* BuildingConfig::getConfig(int buildingType) const {
  auto it = _configs.find(buildingType);
  if (it != _configs.end()) {
    return &(it->second);
  }

  CCLOG("BuildingConfig: Warning - Config not found for type %d", buildingType);
  return nullptr;
}

std::string BuildingConfig::getSpritePath(int buildingType, int level) const {
  const BuildingConfigData* config = getConfig(buildingType);
  if (!config) {
    CCLOG("BuildingConfig: ERROR - No config for building type %d", buildingType);
    return "";
  }

  // 替换 {level} 占位符
  std::string path = config->spritePathTemplate;
  size_t pos = path.find("{level}");
  if (pos != std::string::npos) {
    path.replace(pos, 7, std::to_string(level));
  }

  CCLOG("BuildingConfig: Generated sprite path for type=%d, level=%d: %s", 
        buildingType, level, path.c_str());
  return path;
}

int BuildingConfig::getUpgradeCost(int buildingType, int currentLevel) const {
  const BuildingConfigData* config = getConfig(buildingType);
  if (!config) {
    return 0;
  }

  // 简单的升级费用公式：初始费用 * (1.5 ^ currentLevel)
  return static_cast<int>(config->initialCost * pow(1.5, currentLevel));
}

bool BuildingConfig::canUpgrade(int buildingType, int currentLevel) const {
  const BuildingConfigData* config = getConfig(buildingType);
  if (!config) {
    return false;
  }

  return currentLevel < config->maxLevel;
}