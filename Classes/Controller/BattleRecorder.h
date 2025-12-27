// BattleRecorder.h
// 战斗回放录制和播放管理器声明，处理战斗过程的记录与重放

#ifndef __BATTLE_RECORDER_H__
#define __BATTLE_RECORDER_H__

#include "Model/ReplayData.h"
#include "cocos2d.h"
#include <functional>

class BattleMapLayer;
class BattleTroopLayer;
class BattleHUDLayer;
class BattleProgressUI;

// 战斗回放管理器类
// 职责：录制战斗事件、保存回放数据、播放回放
class BattleRecorder {
public:
    BattleRecorder();
    ~BattleRecorder() = default;

    // 开始录制
    void startRecording();
    
    // 保存当前地图状态（在用户确定目标后调用）
    void saveCurrentMap();
    
    // 停止录制并保存结果
    void stopRecording(int lootedGold, int lootedElixir,
                       const std::map<int, int>& usedTroops,
                       const std::map<int, int>& troopLevels);
    
    // 记录兵种部署事件
    void recordTroopDeployment(int troopId, int gridX, int gridY);
    
    // 是否正在录制
    bool isRecording() const { return _isRecording; }

    // 初始化回放模式
    void initReplayMode(const BattleReplayData& replayData);
    
    // 开始播放回放
    void startReplay(BattleHUDLayer* hudLayer, std::function<void()> onSwitchToFighting);
    
    // 更新回放进度
    void updateReplay(float dt, BattleTroopLayer* troopLayer,
                      std::function<void()> onReplayFinished);
    
    // 是否为回放模式
    bool isReplayMode() const { return _isReplayMode; }

    // 获取回放数据
    const BattleReplayData& getReplayData() const { return _replayData; }
    BattleReplayData& getReplayDataMutable() { return _replayData; }

    // 加载回放地图
    void loadReplayMap(BattleMapLayer* mapLayer, BattleHUDLayer* hudLayer);

private:
    // 检查并部署下一个兵种
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
