#ifndef __JSON_UTILS_H__
#define __JSON_UTILS_H__

#include "cocos2d.h"
#include "json/document.h"
#include "../Model/GameData/GameSaveData.h"

class JsonUtils {
public:
  // 序列化
  static std::string serializeGameData(const GameSaveData& data);
  static std::string serializeBuildingData(const BuildingSaveData& data);

  // 反序列化
  static GameSaveData deserializeGameData(const std::string& json);
  static BuildingSaveData deserializeBuildingData(const rapidjson::Value& json);
};

#endif