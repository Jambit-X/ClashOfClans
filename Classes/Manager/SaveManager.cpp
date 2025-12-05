#include "SaveManager.h"
#include "ResourceManager.h"
#include "cocos2d.h"
#include <ctime>

USING_NS_CC;

SaveManager* SaveManager::_instance = nullptr;

SaveManager* SaveManager::getInstance() {
    if (!_instance) {
        _instance = new SaveManager();
    }
    return _instance;
}

void SaveManager::destroyInstance() {
    if (_instance) {
        delete _instance;
        _instance = nullptr;
    }
}

SaveManager::SaveManager() : _autoSaveEnabled(false) {
    _saveFilePath = FileUtils::getInstance()->getWritablePath() + "savegame.dat";
}

SaveManager::~SaveManager() {
    disableAutoSave();
}

bool SaveManager::saveGame() {
    GameSaveData saveData = getCurrentSaveData();
    std::string jsonStr = serializeToJSON(saveData);
    
    if (FileUtils::getInstance()->writeStringToFile(jsonStr, _saveFilePath)) {
        CCLOG("Game saved successfully to %s", _saveFilePath.c_str());
        return true;
    }
    
    CCLOG("Failed to save game to %s", _saveFilePath.c_str());
    return false;
}

bool SaveManager::loadGame() {
    if (!FileUtils::getInstance()->isFileExist(_saveFilePath)) {
        CCLOG("Save file not found: %s", _saveFilePath.c_str());
        return false;
    }
    
    std::string jsonStr = FileUtils::getInstance()->getStringFromFile(_saveFilePath);
    if (jsonStr.empty()) {
        CCLOG("Failed to read save file");
        return false;
    }
    
    GameSaveData saveData = deserializeFromJSON(jsonStr);
    
    // 加载资源数据到 ResourceManager
    ResourceManager* resMgr = ResourceManager::getInstance();
    resMgr->setGold(saveData.gold);
    resMgr->setElixir(saveData.elixir);
    
    CCLOG("Game loaded successfully");
    return true;
}

void SaveManager::enableAutoSave(float interval) {
    if (!_autoSaveEnabled) {
        _autoSaveEnabled = true;
        Director::getInstance()->getScheduler()->schedule(
            [this](float dt) { this->autoSaveCallback(dt); },
            this,
            interval,
            CC_REPEAT_FOREVER,
            0.0f,
            false,
            "AutoSave"
        );
    }
}

void SaveManager::disableAutoSave() {
    if (_autoSaveEnabled) {
        _autoSaveEnabled = false;
        Director::getInstance()->getScheduler()->unschedule("AutoSave", this);
    }
}

GameSaveData SaveManager::getCurrentSaveData() {
    GameSaveData saveData;
    
    // 玩家信息
    saveData.playerLevel = 1;
    saveData.playerName = "Player";
    saveData.lastSaveTime = (long long)time(nullptr);
    
    // 资源
    ResourceManager* resMgr = ResourceManager::getInstance();
    saveData.gold = resMgr->getGold();
    saveData.elixir = resMgr->getElixir();
    saveData.darkElixir = 0;
    saveData.gems = 0;
    
    // TODO: 收集建筑数据
    
    return saveData;
}

std::string SaveManager::serializeToJSON(const GameSaveData& data) {
    // 简单的 JSON 序列化
    std::string json = "{";
    json += "\"playerLevel\":" + std::to_string(data.playerLevel) + ",";
    json += "\"playerName\":\"" + data.playerName + "\",";
    json += "\"lastSaveTime\":" + std::to_string(data.lastSaveTime) + ",";
    json += "\"gold\":" + std::to_string(data.gold) + ",";
    json += "\"elixir\":" + std::to_string(data.elixir) + ",";
    json += "\"darkElixir\":" + std::to_string(data.darkElixir) + ",";
    json += "\"gems\":" + std::to_string(data.gems);
    json += "}";
    return json;
}

GameSaveData SaveManager::deserializeFromJSON(const std::string& json) {
    GameSaveData data;
    
    // 简单的 JSON 解析（实际项目应使用 JSON 库）
    data.playerLevel = 1;
    data.playerName = "Player";
    data.lastSaveTime = 0;
    data.gold = 0;
    data.elixir = 0;
    data.darkElixir = 0;
    data.gems = 0;
    
    // TODO: 实现完整的 JSON 解析
    
    return data;
}

void SaveManager::autoSaveCallback(float dt) {
    saveGame();
}
