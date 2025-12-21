// Manager/ReplayManager.cpp
#include "ReplayManager.h"
#include "json/document.h"
#include "json/writer.h"
#include "json/stringbuffer.h"

USING_NS_CC;

ReplayManager* ReplayManager::_instance = nullptr;

ReplayManager* ReplayManager::getInstance() {
    if (!_instance) {
        _instance = new ReplayManager();
    }
    return _instance;
}

void ReplayManager::destroyInstance() {
    CC_SAFE_DELETE(_instance);
}

ReplayManager::ReplayManager() : _nextReplayId(1) {
    // 创建回放目录
    auto fileUtils = FileUtils::getInstance();
    std::string replayDir = getReplayDirectory();
    if (!fileUtils->isDirectoryExist(replayDir)) {
        fileUtils->createDirectory(replayDir);
        CCLOG("ReplayManager: Created replay directory at %s", replayDir.c_str());
    }

    // 加载元数据
    loadMetadata();
}

ReplayManager::~ReplayManager() {
    saveMetadata();
}

// ========== 文件路径管理 ==========
std::string ReplayManager::getReplayDirectory() {
    return FileUtils::getInstance()->getWritablePath() + "replays/";
}

std::string ReplayManager::getReplayFilePath(int replayId) {
    return getReplayDirectory() + "replay_" + std::to_string(replayId) + ".json";
}

std::string ReplayManager::getMetadataFilePath() {
    return getReplayDirectory() + "metadata.json";
}

// ========== 保存回放 ==========
void ReplayManager::saveReplay(const BattleReplayData& data) {
    // 1. 分配回放ID
    BattleReplayData saveData = data;
    saveData.replayId = _nextReplayId++;

    // 2. 保存完整回放数据到独立文件
    ValueMap replayMap = saveData.toValueMap();
    std::string filePath = getReplayFilePath(saveData.replayId);

    if (FileUtils::getInstance()->writeValueMapToFile(replayMap, filePath)) {
        CCLOG("ReplayManager: Saved replay #%d to %s", saveData.replayId, filePath.c_str());
    } else {
        CCLOG("ReplayManager: ERROR - Failed to save replay #%d", saveData.replayId);
        return;
    }

    // 3. 添加元数据
    ReplayMetadata meta;
    meta.replayId = saveData.replayId;
    meta.timestamp = saveData.timestamp;
    meta.defenderName = saveData.defenderName;
    meta.finalStars = saveData.finalStars;
    meta.destructionPercentage = saveData.destructionPercentage;
    meta.lootedGold = saveData.lootedGold;
    meta.lootedElixir = saveData.lootedElixir;
    meta.usedTroops = saveData.usedTroops;  // ✅ 兵种消耗
    meta.battleDuration = saveData.battleDuration;

    addMetadata(meta);

    // 4. 强制执行10场限制
    enforceReplayLimit();

    // 5. 保存元数据
    saveMetadata();
}

// ========== 加载回放 ==========
BattleReplayData ReplayManager::loadReplay(int replayId) {
    std::string filePath = getReplayFilePath(replayId);
    ValueMap replayMap = FileUtils::getInstance()->getValueMapFromFile(filePath);

    if (replayMap.empty()) {
        CCLOG("ReplayManager: ERROR - Failed to load replay #%d", replayId);
        return BattleReplayData();
    }

    return BattleReplayData::fromValueMap(replayMap);
}

// ========== 获取回放列表 ==========
std::vector<ReplayMetadata> ReplayManager::getReplayList() {
    return _metadataList;
}

// ========== 删除回放 ==========
void ReplayManager::deleteReplay(int replayId) {
    // 1. 删除文件
    std::string filePath = getReplayFilePath(replayId);
    if (FileUtils::getInstance()->removeFile(filePath)) {
        CCLOG("ReplayManager: Deleted replay file #%d", replayId);
    }

    // 2. 移除元数据
    removeMetadata(replayId);

    // 3. 保存元数据
    saveMetadata();
}

// ========== 元数据管理 ==========
void ReplayManager::loadMetadata() {
    std::string metaPath = getMetadataFilePath();
    if (!FileUtils::getInstance()->isFileExist(metaPath)) {
        CCLOG("ReplayManager: No metadata file found, starting fresh");
        return;
    }

    ValueMap metaMap = FileUtils::getInstance()->getValueMapFromFile(metaPath);
    if (metaMap.empty()) return;

    // 读取 nextReplayId
    if (metaMap.find("nextReplayId") != metaMap.end()) {
        _nextReplayId = metaMap["nextReplayId"].asInt();
    }

    // 读取 replays 数组
    if (metaMap.find("replays") != metaMap.end()) {
        ValueVector replaysVec = metaMap["replays"].asValueVector();
        for (const auto& replayValue : replaysVec) {
            ReplayMetadata meta = ReplayMetadata::fromValueMap(replayValue.asValueMap());
            _metadataList.push_back(meta);
        }
    }

    CCLOG("ReplayManager: Loaded %zu replays, nextId=%d", _metadataList.size(), _nextReplayId);
}

void ReplayManager::saveMetadata() {
    ValueMap metaMap;
    metaMap["nextReplayId"] = _nextReplayId;

    ValueVector replaysVec;
    for (const auto& meta : _metadataList) {
        replaysVec.push_back(Value(meta.toValueMap()));
    }
    metaMap["replays"] = replaysVec;

    std::string metaPath = getMetadataFilePath();
    if (FileUtils::getInstance()->writeValueMapToFile(metaMap, metaPath)) {
        CCLOG("ReplayManager: Saved metadata (%zu replays)", _metadataList.size());
    } else {
        CCLOG("ReplayManager: ERROR - Failed to save metadata");
    }
}

void ReplayManager::addMetadata(const ReplayMetadata& meta) {
    _metadataList.push_back(meta);
}

void ReplayManager::removeMetadata(int replayId) {
    auto it = std::remove_if(_metadataList.begin(), _metadataList.end(),
                             [replayId](const ReplayMetadata& meta) {
        return meta.replayId == replayId;
    });
    _metadataList.erase(it, _metadataList.end());
}

void ReplayManager::enforceReplayLimit() {
    while (_metadataList.size() > MAX_REPLAYS) {
        // 找到最旧的回放（时间戳最小）
        auto oldestIt = std::min_element(_metadataList.begin(), _metadataList.end(),
                                         [](const ReplayMetadata& a, const ReplayMetadata& b) {
            return a.timestamp < b.timestamp;
        });

        if (oldestIt != _metadataList.end()) {
            int oldestId = oldestIt->replayId;
            CCLOG("ReplayManager: Deleting oldest replay #%d to maintain limit", oldestId);
            deleteReplay(oldestId);
        }
    }
}
