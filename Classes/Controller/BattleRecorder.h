#ifndef __BATTLE_RECORDER_H__
#define __BATTLE_RECORDER_H__

#include "Model/ReplayData.h"
#include "cocos2d.h"
#include <functional>

class BattleMapLayer;
class BattleTroopLayer;
class BattleHUDLayer;
class BattleProgressUI;

/**
 * BattleRecorder - 战斗回放录制和播放管理器
 * 
 * 职责：
 * 1. 录制战斗过程中的兵种部署事件
 * 2. 保存战斗结果到回放数据
 * 3. 播放回放数据，自动部署兵种
 */
class BattleRecorder {
public:
    BattleRecorder();
    ~BattleRecorder() = default;

    // ========== 录制控制 ==========
    void startRecording();
    void stopRecording(int lootedGold, int lootedElixir,
                       const std::map<int, int>& usedTroops,
                       const std::map<int, int>& troopLevels);
    void recordTroopDeployment(int troopId, int gridX, int gridY);
    bool isRecording() const { return _isRecording; }

    // ========== 回放播放 ==========
    void initReplayMode(const BattleReplayData& replayData);
    void startReplay(BattleHUDLayer* hudLayer, std::function<void()> onSwitchToFighting);
    void updateReplay(float dt, BattleTroopLayer* troopLayer,
                      std::function<void()> onReplayFinished);
    bool isReplayMode() const { return _isReplayMode; }

    // ========== 回放数据访问 ==========
    const BattleReplayData& getReplayData() const { return _replayData; }
    BattleReplayData& getReplayDataMutable() { return _replayData; }

    // ========== 加载回放地图 ==========
    void loadReplayMap(BattleMapLayer* mapLayer, BattleHUDLayer* hudLayer);

private:
    void checkAndDeployNextTroop(float elapsedTime, BattleTroopLayer* troopLayer);

    // 录制状态
    bool _isRecording = false;
    float _battleStartTime = 0.0f;
    BattleReplayData _replayData;

    // 回放状态
    bool _isReplayMode = false;
    float _replayStartTime = 0.0f;
    size_t _currentEventIndex = 0;
    bool _isEndingScheduled = false;
};

#endif // __BATTLE_RECORDER_H__
