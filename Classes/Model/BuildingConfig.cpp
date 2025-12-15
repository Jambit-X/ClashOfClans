// 【必须加在第一行】强制使用 UTF-8 编码，解决中文乱码
#pragma execution_character_set("utf-8")

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
  // ========== 1: 大本营 (Town Hall) ==========
  BuildingConfigData townHall;
  townHall.type = 1;
  townHall.name = "大本营";
  townHall.category = "核心";
  townHall.spritePathTemplate = "buildings/Town_Hall/Town_Hall{level}.png";
  townHall.gridWidth = 4;
  townHall.gridHeight = 4;
  townHall.anchorOffset = Vec2(0, -60);
  townHall.maxLevel = 3;
  townHall.initialCost = 0;
  townHall.costType = "gold";  // 改为英文
  townHall.buildTimeSeconds = 60;  // 改为60秒
  townHall.hitPoints = 1500;
  townHall.damagePerSecond = 0;
  townHall.attackRange = 0;
  townHall.resourceCapacity = 0;
  townHall.productionRate = 0;
  _configs[1] = townHall;

  // ========== 军队建筑 ==========
  // 101: 兵营
  BuildingConfigData armyCamp;
  armyCamp.type = 101;
  armyCamp.name = "兵营";
  armyCamp.category = "军队";
  armyCamp.spritePathTemplate = "buildings/military_architecture/army_camp/Army_Camp{level}.png";
  armyCamp.gridWidth = 3;
  armyCamp.gridHeight = 3;
  armyCamp.anchorOffset = Vec2(0, -30);
  armyCamp.maxLevel = 3;
  armyCamp.initialCost = 250;
  armyCamp.costType = "elixir";  // 改为英文
  armyCamp.buildTimeSeconds = 30;
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
  barracks.category = "军队";
  barracks.spritePathTemplate = "buildings/military_architecture/barracks/Barracks{level}.png";
  barracks.gridWidth = 3;
  barracks.gridHeight = 3;
  barracks.anchorOffset = Vec2(0, -30);
  barracks.maxLevel = 3;
  barracks.initialCost = 200;
  barracks.costType = "elixir";  // 改为英文
  barracks.buildTimeSeconds = 20;
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
  laboratory.category = "军队";
  laboratory.spritePathTemplate = "buildings/military_architecture/laboratory/Laboratory{level}.png";
  laboratory.gridWidth = 3;
  laboratory.gridHeight = 3;
  laboratory.anchorOffset = Vec2(0, -35);
  laboratory.maxLevel = 3;
  laboratory.initialCost = 500;
  laboratory.costType = "elixir";  // 改为英文
  laboratory.buildTimeSeconds = 60;
  laboratory.hitPoints = 0;
  laboratory.damagePerSecond = 0;
  laboratory.attackRange = 0;
  laboratory.resourceCapacity = 0;
  laboratory.productionRate = 0;
  _configs[103] = laboratory;

  // ========== 资源建筑 ==========
  // 201: 建筑工人小屋 
  BuildingConfigData builderHut;
  builderHut.type = 201;
  builderHut.name = "建筑工人小屋";
  builderHut.category = "资源";
  builderHut.spritePathTemplate = "buildings/resource_architecture/builders_hut/Builders_Hut{level}.png";
  builderHut.gridWidth = 2;           // 2x2 占地
  builderHut.gridHeight = 2;
  builderHut.anchorOffset = Vec2(0, -25);  //  2x2建筑标准偏移
  builderHut.maxLevel = 1;            // 不可升级
  builderHut.initialCost = 50;        // 固定50宝石
  builderHut.costType = "gem";        // 宝石购买
  builderHut.buildTimeSeconds = 0;    // 瞬间完成
  builderHut.hitPoints = 250;
  builderHut.damagePerSecond = 0;
  builderHut.attackRange = 0;
  builderHut.resourceCapacity = 0;
  builderHut.productionRate = 0;
  _configs[201] = builderHut;

  // 202: 金矿
  BuildingConfigData goldMine;
  goldMine.type = 202;
  goldMine.name = "金矿";
  goldMine.category = "资源";
  goldMine.spritePathTemplate = "buildings/resource_architecture/gold_mine/Gold_Mine{level}.png";
  goldMine.gridWidth = 3;
  goldMine.gridHeight = 3;
  goldMine.anchorOffset = Vec2(0, -35);
  goldMine.maxLevel = 3;
  goldMine.initialCost = 150;
  goldMine.costType = "elixir";  // 改为英文
  goldMine.buildTimeSeconds = 20;
  goldMine.hitPoints = 400;
  goldMine.damagePerSecond = 0;
  goldMine.attackRange = 0;
  goldMine.resourceCapacity = 500;
  goldMine.productionRate = 7200;
  _configs[202] = goldMine;

  // 203: 圣水采集器
  BuildingConfigData elixirCollector;
  elixirCollector.type = 203;
  elixirCollector.name = "圣水采集器";
  elixirCollector.category = "资源";
  elixirCollector.spritePathTemplate = "buildings/resource_architecture/elixir_collector/Elixir_Collector{level}.png";
  elixirCollector.gridWidth = 3;
  elixirCollector.gridHeight = 3;
  elixirCollector.anchorOffset = Vec2(0, -35);
  elixirCollector.maxLevel = 3;
  elixirCollector.initialCost = 150;
  elixirCollector.costType = "gold";  // 改为英文
  elixirCollector.buildTimeSeconds = 20;
  elixirCollector.hitPoints = 400;
  elixirCollector.damagePerSecond = 0;
  elixirCollector.attackRange = 0;
  elixirCollector.resourceCapacity = 500;
  elixirCollector.productionRate = 7200;
  _configs[203] = elixirCollector;

  // 204: 储金罐
  BuildingConfigData goldStorage;
  goldStorage.type = 204;
  goldStorage.name = "储金罐";
  goldStorage.category = "资源";
  goldStorage.spritePathTemplate = "buildings/resource_architecture/gold_storage/Gold_Storage{level}.png";
  goldStorage.gridWidth = 3;
  goldStorage.gridHeight = 3;
  goldStorage.anchorOffset = Vec2(0, -40);
  goldStorage.maxLevel = 3;
  goldStorage.initialCost = 300;
  goldStorage.costType = "elixir";  // 改为英文
  goldStorage.buildTimeSeconds = 30;
  goldStorage.hitPoints = 600;
  goldStorage.damagePerSecond = 0;
  goldStorage.attackRange = 0;
  goldStorage.resourceCapacity = 5000;
  goldStorage.productionRate = 0;
  _configs[204] = goldStorage;

  // 205: 圣水瓶
  BuildingConfigData elixirStorage;
  elixirStorage.type = 205;
  elixirStorage.name = "圣水瓶";
  elixirStorage.category = "资源";
  elixirStorage.spritePathTemplate = "buildings/resource_architecture/elixir_storage/Elixir_Storage{level}.png";
  elixirStorage.gridWidth = 3;
  elixirStorage.gridHeight = 3;
  elixirStorage.anchorOffset = Vec2(0, -40);
  elixirStorage.maxLevel = 3;
  elixirStorage.initialCost = 300;
  elixirStorage.costType = "gold";  // 改为英文
  elixirStorage.buildTimeSeconds = 30;
  elixirStorage.hitPoints = 600;
  elixirStorage.damagePerSecond = 0;
  elixirStorage.attackRange = 0;
  elixirStorage.resourceCapacity = 5000;
  elixirStorage.productionRate = 0;
  _configs[205] = elixirStorage;

  // ========== 防御建筑 ==========
  // 301: 加农炮
  BuildingConfigData cannon;
  cannon.type = 301;
  cannon.name = "加农炮";
  cannon.category = "防御";
  cannon.spritePathTemplate = "buildings/defence_architecture/cannon/Cannon_lvl{level}.png";
  cannon.gridWidth = 3;
  cannon.gridHeight = 3;
  cannon.anchorOffset = Vec2(0, -30);
  cannon.maxLevel = 3;
  cannon.initialCost = 250;
  cannon.costType = "gold";  // 改为英文
  cannon.buildTimeSeconds = 15;
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
  archerTower.anchorOffset = Vec2(0, -35);
  archerTower.maxLevel = 3;
  archerTower.initialCost = 1000;
  archerTower.costType = "gold";  // 改为英文
  archerTower.buildTimeSeconds = 30;
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
  wall.anchorOffset = Vec2(0, -15);
  wall.maxLevel = 3;
  wall.initialCost = 50;
  wall.costType = "gold";  // 改为英文
  wall.buildTimeSeconds = 0;
  wall.hitPoints = 300;
  wall.damagePerSecond = 0;
  wall.attackRange = 0;
  wall.resourceCapacity = 0;
  wall.productionRate = 0;
  _configs[303] = wall;

  // ========== 陷阱 ==========
  // 401: 炸弹
  BuildingConfigData bomb;
  bomb.type = 401;
  bomb.name = "炸弹";
  bomb.category = "陷阱";
  bomb.spritePathTemplate = "buildings/trap/bomb/Bomb{level}.png";
  bomb.gridWidth = 1;
  bomb.gridHeight = 1;
  bomb.anchorOffset = Vec2(0, -15);
  bomb.maxLevel = 3;
  bomb.initialCost = 400;
  bomb.costType = "gold";  // 改为英文
  bomb.buildTimeSeconds = 0;
  bomb.hitPoints = 1;
  bomb.damagePerSecond = 100;
  bomb.attackRange = 1;
  bomb.resourceCapacity = 0;
  bomb.productionRate = 0;
  _configs[401] = bomb;

  // 402: 隐形弹簧
  BuildingConfigData springTrap;
  springTrap.type = 402;
  springTrap.name = "隐形弹簧";
  springTrap.category = "陷阱";
  springTrap.spritePathTemplate = "buildings/trap/spring/Spring_Trap{level}.png";
  springTrap.gridWidth = 1;
  springTrap.gridHeight = 1;
  springTrap.anchorOffset = Vec2(0, -15);
  springTrap.maxLevel = 3;
  springTrap.initialCost = 2000;
  springTrap.costType = "gold";  // 改为英文
  springTrap.buildTimeSeconds = 0;
  springTrap.hitPoints = 1;
  springTrap.damagePerSecond = 0;
  springTrap.attackRange = 1;
  springTrap.resourceCapacity = 0;
  springTrap.productionRate = 0;
  _configs[402] = springTrap;

  // 404: 巨型炸弹
  BuildingConfigData giantBomb;
  giantBomb.type = 404;
  giantBomb.name = "巨型炸弹";
  giantBomb.category = "陷阱";
  giantBomb.spritePathTemplate = "buildings/trap/giant_bomb/Giant_Bomb{level}.png";
  giantBomb.gridWidth = 2;
  giantBomb.gridHeight = 2;
  giantBomb.anchorOffset = Vec2(0, -25);
  giantBomb.maxLevel = 3;
  giantBomb.initialCost = 12500;
  giantBomb.costType = "gold";  // 改为英文
  giantBomb.buildTimeSeconds = 0;
  giantBomb.hitPoints = 1;
  giantBomb.damagePerSecond = 400;
  giantBomb.attackRange = 2;
  giantBomb.resourceCapacity = 0;
  giantBomb.productionRate = 0;
  _configs[404] = giantBomb;

  CCLOG("BuildingConfig: Initialized %lu building configs", _configs.size());
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

  // 核心修复：0 级建筑显示 1 级图片
  int displayLevel = (level == 0) ? 1 : level;

  // 替换 {level} 占位符
  std::string path = config->spritePathTemplate;
  size_t pos = path.find("{level}");
  if (pos != std::string::npos) {
    path.replace(pos, 7, std::to_string(displayLevel));
  }

  CCLOG("BuildingConfig: Generated sprite path for type=%d, level=%d (display=%d): %s",
        buildingType, level, displayLevel, path.c_str());
  return path;
}

// 核心修复：硬编码升级成本
int BuildingConfig::getUpgradeCost(int buildingType, int currentLevel) const {
  const BuildingConfigData* config = getConfig(buildingType);
  if (!config) {
    return 0;
  }

  // ========== 大本营特殊处理 ==========
  if (buildingType == 1) {
    switch (currentLevel) {
      case 1: return 1000;   // 1→2 级：1000 金币
      case 2: return 4000;   // 2→3 级：4000 金币
      default: return 0;
    }
  }

  // ========== 其他建筑通用公式 ==========
  // 基础成本 * 2 的等级次方
  // 例如：initialCost=250, level=1 → 250*2=500
  //       initialCost=250, level=2 → 250*4=1000
  int baseCost = config->initialCost;
  if (baseCost == 0) {
    CCLOG("BuildingConfig: Warning - initialCost is 0 for type %d", buildingType);
    return 0;
  }

  return static_cast<int>(baseCost * pow(2, currentLevel));
}

bool BuildingConfig::canUpgrade(int buildingType, int currentLevel) const {
  const BuildingConfigData* config = getConfig(buildingType);
  if (!config) {
    return false;
  }

  return currentLevel < config->maxLevel;
}

// ========== 根据等级获取存储容量 ==========
int BuildingConfig::getStorageCapacityByLevel(int buildingType, int level) const {
  // 储金罐（204）和圣水瓶（205）的容量表
  static const std::map<int, int> capacityTable = {
    {1, 5000},
    {2, 15000},
    {3, 50000}
  };

  if (buildingType != 204 && buildingType != 205) {
    CCLOG("BuildingConfig: ERROR - Building type %d is not a storage building", buildingType);
    return 0;
  }

  auto it = capacityTable.find(level);
  if (it != capacityTable.end()) {
    return it->second;
  }

  CCLOG("BuildingConfig: ERROR - Invalid level %d for storage building", level);
  return 0;
}