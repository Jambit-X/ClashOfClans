// ReplayData.cpp
// 战斗回放数据序列化实现

#include "ReplayData.h"
#include "BuildingConfig.h"

USING_NS_CC;

// TroopDeployEvent 序列化
ValueMap TroopDeployEvent::toValueMap() const {
    ValueMap map;
    map["timestamp"] = timestamp;
    map["troopId"] = troopId;
    map["gridX"] = gridX;
    map["gridY"] = gridY;
    return map;
}

TroopDeployEvent TroopDeployEvent::fromValueMap(const ValueMap& map) {
    TroopDeployEvent event;
    event.timestamp = map.at("timestamp").asFloat();
    event.troopId = map.at("troopId").asInt();
    event.gridX = map.at("gridX").asInt();
    event.gridY = map.at("gridY").asInt();
    return event;
}

// BattleReplayData 序列化
ValueMap BattleReplayData::toValueMap() const {
    ValueMap map;

    // 元数据
    map["replayId"] = replayId;
    map["timestamp"] = (int)timestamp;
    map["defenderName"] = defenderName;
    map["battleDuration"] = battleDuration;

    // 战斗结果
    map["finalStars"] = finalStars;
    map["destructionPercentage"] = destructionPercentage;
    map["lootedGold"] = lootedGold;
    map["lootedElixir"] = lootedElixir;

    // 兵种消耗
    ValueMap usedTroopsMap;
    for (const auto& pair : usedTroops) {
        usedTroopsMap[std::to_string(pair.first)] = pair.second;
    }
    map["usedTroops"] = usedTroopsMap;

    // 兵种等级
    ValueMap troopLevelsMap;
    for (const auto& pair : troopLevels) {
        troopLevelsMap[std::to_string(pair.first)] = pair.second;
    }
    map["troopLevels"] = troopLevelsMap;

    // 地图快照
    map["battleMapSeed"] = battleMapSeed;
    ValueVector buildingsVec;
    for (const auto& building : initialBuildings) {
        ValueMap buildingMap;
        buildingMap["id"] = building.id;
        buildingMap["type"] = building.type;
        buildingMap["level"] = building.level;
        buildingMap["gridX"] = building.gridX;
        buildingMap["gridY"] = building.gridY;
        buildingMap["currentHP"] = building.currentHP;

        // 从BuildingConfig读取maxHP
        auto config = BuildingConfig::getInstance()->getConfig(building.type);
        if (config) {
            buildingMap["maxHP"] = config->hitPoints;
        } else {
            buildingMap["maxHP"] = building.currentHP;
        }

        buildingsVec.push_back(Value(buildingMap));
    }
    map["initialBuildings"] = buildingsVec;

    // 兵种部署序列
    ValueVector eventsVec;
    for (const auto& event : troopEvents) {
        eventsVec.push_back(Value(event.toValueMap()));
    }
    map["troopEvents"] = eventsVec;

    return map;
}

BattleReplayData BattleReplayData::fromValueMap(const ValueMap& map) {
    BattleReplayData data;

    // 元数据
    data.replayId = map.at("replayId").asInt();
    data.timestamp = (time_t)map.at("timestamp").asInt();
    data.defenderName = map.at("defenderName").asString();
    data.battleDuration = map.at("battleDuration").asFloat();

    // 战斗结果
    data.finalStars = map.at("finalStars").asInt();
    data.destructionPercentage = map.at("destructionPercentage").asInt();
    data.lootedGold = map.at("lootedGold").asInt();
    data.lootedElixir = map.at("lootedElixir").asInt();

    // 兵种消耗
    if (map.find("usedTroops") != map.end()) {
        ValueMap usedTroopsMap = map.at("usedTroops").asValueMap();
        for (const auto& pair : usedTroopsMap) {
            int troopId = std::stoi(pair.first);
            int count = pair.second.asInt();
            data.usedTroops[troopId] = count;
        }
    }

    // 兵种等级
    if (map.find("troopLevels") != map.end()) {
        ValueMap troopLevelsMap = map.at("troopLevels").asValueMap();
        for (const auto& pair : troopLevelsMap) {
            int troopId = std::stoi(pair.first);
            int level = pair.second.asInt();
            data.troopLevels[troopId] = level;
        }
    }

    // 地图快照
    data.battleMapSeed = map.at("battleMapSeed").asInt();
    if (map.find("initialBuildings") != map.end()) {
        ValueVector buildingsVec = map.at("initialBuildings").asValueVector();
        for (const auto& buildingValue : buildingsVec) {
            ValueMap buildingMap = buildingValue.asValueMap();
            BuildingInstance building;
            building.id = buildingMap.at("id").asInt();
            building.type = buildingMap.at("type").asInt();
            building.level = buildingMap.at("level").asInt();
            building.gridX = buildingMap.at("gridX").asInt();
            building.gridY = buildingMap.at("gridY").asInt();
            building.currentHP = buildingMap.at("currentHP").asInt();

            // 使用BUILT状态
            building.state = BuildingInstance::State::BUILT;
            building.isDestroyed = false;
            building.finishTime = 0;
            building.isInitialConstruction = false;

            data.initialBuildings.push_back(building);
        }
    }

    // 兵种部署序列
    if (map.find("troopEvents") != map.end()) {
        ValueVector eventsVec = map.at("troopEvents").asValueVector();
        for (const auto& eventValue : eventsVec) {
            TroopDeployEvent event = TroopDeployEvent::fromValueMap(eventValue.asValueMap());
            data.troopEvents.push_back(event);
        }
    }

    return data;
}

// ReplayMetadata 序列化
ValueMap ReplayMetadata::toValueMap() const {
    ValueMap map;
    map["replayId"] = replayId;
    map["timestamp"] = (int)timestamp;
    map["defenderName"] = defenderName;
    map["finalStars"] = finalStars;
    map["destructionPercentage"] = destructionPercentage;
    map["lootedGold"] = lootedGold;
    map["lootedElixir"] = lootedElixir;
    map["battleDuration"] = battleDuration;

    // 兵种消耗
    ValueMap usedTroopsMap;
    for (const auto& pair : usedTroops) {
        usedTroopsMap[std::to_string(pair.first)] = pair.second;
    }
    map["usedTroops"] = usedTroopsMap;

    return map;
}

ReplayMetadata ReplayMetadata::fromValueMap(const ValueMap& map) {
    ReplayMetadata meta;
    meta.replayId = map.at("replayId").asInt();
    meta.timestamp = (time_t)map.at("timestamp").asInt();
    meta.defenderName = map.at("defenderName").asString();
    meta.finalStars = map.at("finalStars").asInt();
    meta.destructionPercentage = map.at("destructionPercentage").asInt();
    meta.lootedGold = map.at("lootedGold").asInt();
    meta.lootedElixir = map.at("lootedElixir").asInt();
    meta.battleDuration = map.at("battleDuration").asFloat();

    // 兵种消耗
    if (map.find("usedTroops") != map.end()) {
        ValueMap usedTroopsMap = map.at("usedTroops").asValueMap();
        for (const auto& pair : usedTroopsMap) {
            int troopId = std::stoi(pair.first);
            int count = pair.second.asInt();
            meta.usedTroops[troopId] = count;
        }
    }

    return meta;
}
