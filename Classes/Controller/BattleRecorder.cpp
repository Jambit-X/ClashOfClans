#pragma execution_character_set("utf-8")
#include "BattleRecorder.h"
#include "../Layer/BattleTroopLayer.h"
#include "../Layer/BattleMapLayer.h"
#include "../Layer/BattleHUDLayer.h"
#include "../Manager/VillageDataManager.h"
#include "../Manager/ReplayManager.h"
#include "../Manager/AudioManager.h"
#include "../Controller/BattleProcessController.h"
#include "../Controller/DestructionTracker.h"
#include "../UI/BattleProgressUI.h"

USING_NS_CC;

BattleRecorder::BattleRecorder()
    : _isRecording(false)
    , _battleStartTime(0.0f)
    , _isReplayMode(false)
    , _replayStartTime(0.0f)
    , _currentEventIndex(0)
    , _isEndingScheduled(false)
{
}

// ========== 录制系统实现 ==========

void BattleRecorder::startRecording() {
    _isRecording = true;
    _battleStartTime = Director::getInstance()->getTotalFrames() / 60.0f;

    // 清空数据
    _replayData = BattleReplayData();
    _replayData.troopEvents.clear();

    // 保存初始地图状态
    auto dataManager = VillageDataManager::getInstance();
    _replayData.initialBuildings = dataManager->getAllBuildings();
    _replayData.battleMapSeed = 0;

    // 保存对手名称
    _replayData.defenderName = "AI Village";
    _replayData.timestamp = time(nullptr);

    CCLOG("BattleRecorder: Recording started at time %.2f", _battleStartTime);
}

void BattleRecorder::recordTroopDeployment(int troopId, int gridX, int gridY) {
    if (!_isRecording) return;

    float currentTime = Director::getInstance()->getTotalFrames() / 60.0f;
    float timestamp = currentTime - _battleStartTime;

    TroopDeployEvent event;
    event.timestamp = timestamp;
    event.troopId = troopId;
    event.gridX = gridX;
    event.gridY = gridY;

    _replayData.troopEvents.push_back(event);

    CCLOG("BattleRecorder: Recorded troop %d at grid(%d, %d) @ %.2fs",
          troopId, gridX, gridY, timestamp);
}

void BattleRecorder::stopRecording(int lootedGold, int lootedElixir,
                                    const std::map<int, int>& usedTroops,
                                    const std::map<int, int>& troopLevels) {
    if (!_isRecording) return;

    _isRecording = false;

    // 保存战斗结果
    auto tracker = DestructionTracker::getInstance();
    _replayData.finalStars = tracker->getStars();
    _replayData.destructionPercentage = (int)tracker->getProgress();
    _replayData.lootedGold = lootedGold;
    _replayData.lootedElixir = lootedElixir;
    _replayData.usedTroops = usedTroops;
    _replayData.troopLevels = troopLevels;
    _replayData.battleDuration = Director::getInstance()->getTotalFrames() / 60.0f - _battleStartTime;

    // 保存到本地
    ReplayManager::getInstance()->saveReplay(_replayData);

    CCLOG("BattleRecorder: Recording stopped, saved replay with %zu events",
          _replayData.troopEvents.size());
}

// ========== 回放播放实现 ==========

void BattleRecorder::initReplayMode(const BattleReplayData& replayData) {
    _isReplayMode = true;
    _replayData = replayData;
    _currentEventIndex = 0;
    _isEndingScheduled = false;
    CCLOG("BattleRecorder: Initialized replay mode with %zu events",
          _replayData.troopEvents.size());
}

void BattleRecorder::startReplay(BattleHUDLayer* hudLayer, std::function<void()> onSwitchToFighting) {
    if (!_isReplayMode) return;

    CCLOG("BattleRecorder: Starting replay playback with %zu events, duration: %.2fs",
          _replayData.troopEvents.size(), _replayData.battleDuration);

    // 隐藏 HUD 中的兵种选择栏
    if (hudLayer) {
        hudLayer->hideReplayControls();
        CCLOG("BattleRecorder: HUD controls hidden for replay mode");
    }

    _replayStartTime = Director::getInstance()->getTotalFrames() / 60.0f;
    _currentEventIndex = 0;
    _isEndingScheduled = false;

    // 自动进入战斗状态
    if (onSwitchToFighting) {
        onSwitchToFighting();
    }
}

void BattleRecorder::updateReplay(float dt, BattleTroopLayer* troopLayer,
                                   std::function<void()> onReplayFinished) {
    if (!_isReplayMode) return;

    float currentTime = Director::getInstance()->getTotalFrames() / 60.0f;
    float elapsedTime = currentTime - _replayStartTime;

    // 检查是否有兵种需要部署
    checkAndDeployNextTroop(elapsedTime, troopLayer);

    // 当已经过了完整的战斗持续时间后触发结束回调
    // 这样回放会完整播放整个战斗过程，而不是兵种部署完就结束
    if (elapsedTime >= _replayData.battleDuration && !_isEndingScheduled) {
        _isEndingScheduled = true;
        CCLOG("BattleRecorder: Battle duration (%.2fs) reached, triggering finish callback...", 
              _replayData.battleDuration);
        
        if (onReplayFinished) {
            onReplayFinished();
        }
    }
}

void BattleRecorder::checkAndDeployNextTroop(float elapsedTime, BattleTroopLayer* troopLayer) {
    if (!troopLayer) return;

    while (_currentEventIndex < _replayData.troopEvents.size()) {
        const auto& event = _replayData.troopEvents[_currentEventIndex];

        // 如果时间未到，跳出循环
        if (elapsedTime < event.timestamp) {
            break;
        }

        // 兵种ID映射
        std::string name = "Barbarian";
        if (event.troopId == 1001) name = "Barbarian";
        else if (event.troopId == 1002) name = "Archer";
        else if (event.troopId == 1003) name = "Goblin";
        else if (event.troopId == 1004) name = "Giant";
        else if (event.troopId == 1005) name = "Wall_Breaker";
        else if (event.troopId == 1006) name = "Balloon";

        auto unit = troopLayer->spawnUnit(name, event.gridX, event.gridY);
        if (unit) {
            // 播放部署音效
            auto audioManager = AudioManager::getInstance();
            if (event.troopId == 1001) {
                audioManager->playEffect("Audios/barbarian_deploy.mp3", 0.8f);
            } else if (event.troopId == 1002) {
                audioManager->playEffect("Audios/archer_deploy.mp3", 0.8f);
            } else if (event.troopId == 1003) {
                audioManager->playEffect("Audios/goblin_deploy.mp3", 0.8f);
            } else if (event.troopId == 1004) {
                audioManager->playEffect("Audios/giant_deploy.mp3", 0.8f);
            } else if (event.troopId == 1005) {
                audioManager->playEffect("Audios/wall_breaker_deploy.mp3", 0.8f);
            } else if (event.troopId == 1006) {
                audioManager->playEffect("Audios/balloon_deploy.mp3", 0.8f);
            }

            BattleProcessController::getInstance()->startUnitAI(unit, troopLayer);
            CCLOG("BattleRecorder: [REPLAY] Auto-deployed %s at grid(%d, %d)",
                  name.c_str(), event.gridX, event.gridY);
        }

        _currentEventIndex++;
    }
}

// ========== 加载回放地图 ==========

void BattleRecorder::loadReplayMap(BattleMapLayer* mapLayer, BattleHUDLayer* hudLayer) {
    if (!mapLayer) return;

    CCLOG("BattleRecorder: Loading replay map with %zu buildings",
          _replayData.initialBuildings.size());

    // 将回放的建筑数据加载到 VillageDataManager
    auto dataManager = VillageDataManager::getInstance();

    // 清空当前战斗地图数据
    dataManager->clearBattleMap();

    // 加载回放的建筑
    for (const auto& building : _replayData.initialBuildings) {
        dataManager->addBattleBuildingFromReplay(building);
    }

    // 刷新地图层显示
    mapLayer->reloadMapFromData();

    // 通知 HUD 更新资源显示
    if (hudLayer) {
        hudLayer->initLootDisplay(_replayData.lootedGold, _replayData.lootedElixir);
    }

    CCLOG("BattleRecorder: Replay map loaded successfully");
}
