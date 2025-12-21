// Manager/ReplayManager.h
#ifndef __REPLAY_MANAGER_H__
#define __REPLAY_MANAGER_H__

#include "cocos2d.h"
#include "Model/ReplayData.h"
#include <vector>
#include <string>

class ReplayManager {
public:
    static ReplayManager* getInstance();
    static void destroyInstance();

    // ========== 回放操作 ==========
    void saveReplay(const BattleReplayData& data);          // 保存回放
    BattleReplayData loadReplay(int replayId);              // 加载完整回放数据
    std::vector<ReplayMetadata> getReplayList();            // 获取回放列表（元数据）
    void deleteReplay(int replayId);                        // 删除回放

private:
    ReplayManager();
    ~ReplayManager();

    static ReplayManager* _instance;

    // ========== 文件路径管理 ==========
    std::string getReplayDirectory();                       // 获取回放目录
    std::string getReplayFilePath(int replayId);           // 获取回放文件路径
    std::string getMetadataFilePath();                      // 获取元数据文件路径

    // ========== 元数据管理 ==========
    void loadMetadata();                                    // 加载元数据
    void saveMetadata();                                    // 保存元数据
    void addMetadata(const ReplayMetadata& meta);          // 添加元数据
    void removeMetadata(int replayId);                     // 移除元数据
    void enforceReplayLimit();                              // 强制执行10场限制

    std::vector<ReplayMetadata> _metadataList;             // 缓存的元数据列表
    int _nextReplayId;                                      // 下一个回放ID

    const int MAX_REPLAYS = 10;                             // 最多保存10场
};

#endif // __REPLAY_MANAGER_H__
