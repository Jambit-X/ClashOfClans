#include "BuildingConfig.h"
#include <cmath>

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
  // ========== 军队建筑 ==========

   // 101: 兵营
  _configs[101] = {
    /* .type = */ 101,
    /* .name = */ "兵营",
    /* .category = */ "军队",
    /* .spritePathTemplate = */ "UI/Shop/military_architecture/Army_Camp{level}.png",
    /* .gridWidth = */ 3,
    /* .gridHeight = */ 3,
    /* .anchorOffset = */ Vec2(0, 0),
    /* .maxLevel = */ 10,
    /* .initialCost = */ 250,
    /* .costType = */ "圣水",
    /* .buildTimeSeconds = */ 300,
    /* .hitPoints = */ 0,
    /* .damagePerSecond = */ 0,
    /* .attackRange = */ 0,
    /* .resourceCapacity = */ 0,
    /* .productionRate = */ 0
  };

  // 102: 训练营
  _configs[102] = {
    /* .type = */ 102,
    /* .name = */ "训练营",
    /* .category = */ "军队",
    /* .spritePathTemplate = */ "UI/Shop/military_architecture/Barracks{level}.png",
    /* .gridWidth = */ 3,
    /* .gridHeight = */ 3,
    /* .anchorOffset = */ Vec2(0, 0),
    /* .maxLevel = */ 13,
    /* .initialCost = */ 200,
    /* .costType = */ "圣水",
    /* .buildTimeSeconds = */ 60,
    /* .hitPoints = */ 0,
    /* .damagePerSecond = */ 0,
    /* .attackRange = */ 0,
    /* .resourceCapacity = */ 0,
    /* .productionRate = */ 0
  };

  // 103: 实验室
  _configs[103] = {
      /*.type = */103,
      /*.name = */"实验室",
      /*.category = */"军队",
      /*.spritePathTemplate = */"UI/Shop/military_architecture/Laboratory{level}.png",
      /*.gridWidth = */3,
      /*.gridHeight = */3,
      /*.anchorOffset=*/Vec2(30,-30),
      /*.maxLevel = */10,
      /*.initialCost = */500,
      /*.costType = */"圣水",
      /*.buildTimeSeconds = */1800,  // 30分钟
      /*.hitPoints = */0,
      /*.damagePerSecond = */0,
      /*.attackRange = */0,
      /*.resourceCapacity = */0,
      /*.productionRate = */0
  };

  // ========== 资源建筑 ==========

  // 202: 金矿
  _configs[202] = {
      /*.type = */202,
      /*.name = */"金矿",
      /*.category = */"资源",
      /*.spritePathTemplate = */"UI/Shop/resource_architecture/Gold_Mine{level}.png",
      /*.gridWidth = */3,
      /*.gridHeight = */3,
      /*.anchorOffset = */Vec2(0, 0),
      /*.maxLevel = */14,
      /*.initialCost = */150,
      /*.costType = */"圣水",
      /*.buildTimeSeconds = */60,  // 1分钟
      /*.hitPoints = */400,
      /*.damagePerSecond = */0,
      /*.attackRange = */0,
      /*.resourceCapacity = */500,
      /*.productionRate = */20  // 每小时产20金币
  };

  // 203: 圣水收集器
  _configs[203] = {
      /*.type = */203,
      /*.name = */"圣水收集器",
      /*.category = */"资源",
      /*.spritePathTemplate = */"UI/Shop/resource_architecture/Elixir_Collector{level}.png",
      /*.gridWidth = */3,
      /*.gridHeight = */3,
      /*.anchorOffset = */Vec2(0, 0),
      /*.maxLevel = */14,
      /*.initialCost = */150,
      /*.costType = */"金币",
      /*.buildTimeSeconds = */60,
      /*.hitPoints = */400,
      /*.damagePerSecond = */0,
      /*.attackRange = */0,
      /*.resourceCapacity = */500,
      /*.productionRate = */20  // 每小时产20圣水
  };

  // ========== 防御建筑 ==========

  // 301: 加农炮
  _configs[301] = {
      /*.type = */301,
      /*.name = */"加农炮",
      /*.category = */"防御",
      /*.spritePathTemplate = */"UI/Shop/defence_architecture/Cannon_lvl{level}.png",
      /*.gridWidth = */3,
      /*.gridHeight = */3,
      /*.anchorOffset = */Vec2(0, 0),
      /*.maxLevel = */18,
      /*.initialCost = */250,
      /*.costType = */"金币",
      /*.buildTimeSeconds = */60,
      /*.hitPoints = */620,
      /*.damagePerSecond = */11,
      /*.attackRange = */9,
      /*.resourceCapacity = */0,
      /*.productionRate = */0
  };

  // 302: 箭塔
  _configs[302] = {
      /*.type = */302,
      /*.name = */"箭塔",
      /*.category = */"防御",
      /*.spritePathTemplate = */"UI/Shop/defence_architecture/Archer_Tower{level}.png",
      /*.gridWidth = */3,
      /*.gridHeight = */3,
      /*.anchorOffset = */Vec2(0, 0),
      /*.maxLevel = */18,
      /*.initialCost = */1000,
      /*.costType = */"金币",
      /*.buildTimeSeconds = */900,  // 15分钟
      /*.hitPoints = */380,
      /*.damagePerSecond = */11,
      /*.attackRange = */10,
      /*.resourceCapacity = */0,
      /*.productionRate = */0
  };

  // 303: 城墙
  _configs[303] = {
      /*.type = */303,
      /*.name = */"城墙",
      /*.category = */"防御",
      /*.spritePathTemplate = */"UI/Shop/defence_architecture/Wall{level}.png",
      /*.gridWidth = */1,
      /*.gridHeight = */1,
      /*.anchorOffset = */Vec2(0, 0),
      /*.maxLevel = */14,
      /*.initialCost = */50,
      /*.costType = */"金币",
      /*.buildTimeSeconds = */0,  // 立即建造
      /*.hitPoints = */300,
      /*.damagePerSecond = */0,
      /*.attackRange = */0,
      /*.resourceCapacity = */0,
      /*.productionRate = */0
  };

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
    return "";
  }

  // 替换 {level} 占位符
  std::string path = config->spritePathTemplate;
  size_t pos = path.find("{level}");
  if (pos != std::string::npos) {
    path.replace(pos, 7, std::to_string(level));
  }

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