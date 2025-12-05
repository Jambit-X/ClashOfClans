#ifndef __SAVE_MANAGER_H__
#define __SAVE_MANAGER_H__

#include "cocos2d.h"
#include "../Model/GameSaveData.h"

class SaveManager {
public:
  static SaveManager* getInstance();
  static void destroyInstance();

  // 保存/加载
  bool saveGame();
  bool loadGame();

  // 自动保存
  void enableAutoSave(float interval = 60.0f);
  void disableAutoSave();

  // 获取存档数据
  GameSaveData getCurrentSaveData();

private:
  SaveManager();
  ~SaveManager();

  static SaveManager* _instance;
  std::string _saveFilePath;

  // 序列化
  std::string serializeToJSON(const GameSaveData& data);
  GameSaveData deserializeFromJSON(const std::string& json);

  // 自动保存
  void autoSaveCallback(float dt);
  bool _autoSaveEnabled;
};

#endif