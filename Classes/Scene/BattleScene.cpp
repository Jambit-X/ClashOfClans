// BattleScene.cpp
// 战斗场景实现，包含战斗逻辑、兵种部署和资源掠夺

#pragma execution_character_set("utf-8")
#include "BattleScene.h"
#include "Layer/BattleMapLayer.h"
#include "Layer/BattleHUDLayer.h"
#include "Layer/BattleResultLayer.h"
#include "Scene/VillageScene.h"
#include "Layer/BattleTroopLayer.h"
#include "Controller/BattleProcessController.h"
#include "Controller/TrapSystem.h"
#include "Controller/DefenseSystem.h"
#include "Controller/DestructionTracker.h"
#include "Manager/VillageDataManager.h"
#include "Manager/BuildingManager.h"
#include "Manager/AudioManager.h"
#include "Model/BuildingConfig.h"
#include "UI/BattleProgressUI.h"
#include "Util/FindPathUtil.h"
#include "Util/GridMapUtils.h"
#include "Util/RandomBattleMapGenerator.h"
#include "Component/DefenseBuildingAnimation.h"
#include <iostream>

USING_NS_CC;

Scene* BattleScene::createScene() {
    return BattleScene::create();
}

// Scene/BattleScene.cpp

bool BattleScene::init() {
    if (!Scene::init()) return false;

    _stateTimer = 0.0f;
    _resultLayer = nullptr;
    _currentState = BattleState::PREPARE;
    _isSearching = false;

    // 初始化音频ID
    _combatPlanningMusicID = -1;
    _combatMusicID = -1;

    // 初始化战斗兵种数据
    initBattleTroops();

    // 地图层
    _mapLayer = BattleMapLayer::create();
    if (_mapLayer) {
        this->addChild(_mapLayer, 0);

        // 将TroopLayer作为MapLayer的子节点，Tag设为999
        auto troopLayer = BattleTroopLayer::create();
        troopLayer->setTag(999);
        _mapLayer->addChild(troopLayer, 10);
    }

    // UI层（Z=10）
    _hudLayer = BattleHUDLayer::create();
    if (_hudLayer) this->addChild(_hudLayer, 10);

    // 初始化云层遮罩
    _cloudSprite = Sprite::create("UI/battle/battle-prepare/cloud.jpg");
    if (_cloudSprite) {
        auto visibleSize = Director::getInstance()->getVisibleSize();
        _cloudSprite->setPosition(visibleSize.width / 2, visibleSize.height / 2);

        float scaleX = visibleSize.width / _cloudSprite->getContentSize().width;
        float scaleY = visibleSize.height / _cloudSprite->getContentSize().height;
        float finalScale = std::max(scaleX, scaleY) * 1.1f;
        _cloudSprite->setScale(finalScale);

        _cloudSprite->setOpacity(255);
        _cloudSprite->setVisible(true);

        this->addChild(_cloudSprite, 100);
    }

    // 进入战斗模式
    VillageDataManager::getInstance()->setInBattleMode(true);
    CCLOG("BattleScene: Entered battle mode");

    // 更新寻路地图
    FindPathUtil::getInstance()->updatePathfindingMap();
    CCLOG("BattleScene: Pathfinding map updated for battle");

    switchState(BattleState::PREPARE);

    // 启动时执行进入动画
    float randomStartDelay = cocos2d::RandomHelper::random_real(0.5f, 1.0f);
    this->scheduleOnce([this](float) {
        performCloudTransition(nullptr, true);
    }, randomStartDelay, "start_anim_delay");

    this->scheduleUpdate();

    // 开启触摸监听
    setupTouchListener();

    // 设置建筑摧毁事件监听
    setupBuildingDestroyedListener();

    // 创建战斗进度UI
    _battleProgressUI = BattleProgressUI::create();
    if (_battleProgressUI) {
        this->addChild(_battleProgressUI, 200);
        CCLOG("BattleScene: Battle progress UI initialized");
    }

    // 注册事件监听
    auto dispatcher = Director::getInstance()->getEventDispatcher();

    _progressListener = dispatcher->addCustomEventListener(
        "EVENT_DESTRUCTION_PROGRESS_UPDATED",
        CC_CALLBACK_1(BattleScene::onProgressUpdated, this)
    );

    _starListener = dispatcher->addCustomEventListener(
        "EVENT_STAR_AWARDED",
        CC_CALLBACK_1(BattleScene::onStarAwarded, this)
    );

    return true;
}

//在 onEnter 中完成初始化
void BattleScene::onEnter() {
    Scene::onEnter();

    // 只执行一次
    if (_isInitialized) return;
    _isInitialized = true;

    CCLOG("BattleScene::onEnter - Completing initialization");

    // 根据模式加载不同的地图数据
    if (_recorder.isReplayMode()) {
        CCLOG("BattleScene: Initializing in REPLAY mode");

        // 回放模式：加载回放地图（不生成新地图）
        loadReplayMap();

    } else {
        CCLOG("BattleScene: Initializing in NORMAL mode");

        // 正常模式：生成随机地图
        loadEnemyVillage();
        setupTouchListener();
        setupBuildingDestroyedListener();

        // 初始化摧毁追踪（只在正常模式）
        DestructionTracker::getInstance()->initTracking();

        startRecording();
    }
}

void BattleScene::update(float dt) {
    if (_currentState == BattleState::PREPARE || _currentState == BattleState::FIGHTING) {
        _stateTimer -= dt;
        if (_hudLayer) _hudLayer->updateTimer((int)_stateTimer);

        if (_stateTimer <= 0) {
            if (_currentState == BattleState::PREPARE) {
                // 准备时间结束后自动进入战斗状态
                CCLOG("BattleScene: Preparation time expired, auto-starting battle");
                switchState(BattleState::FIGHTING);
            } else if (_currentState == BattleState::FIGHTING) {
                onEndBattleClicked();
            }
        }
        
        // 建筑防御系统和陷阱系统自动更新
        if (_currentState == BattleState::FIGHTING) {
            auto troopLayer = dynamic_cast<BattleTroopLayer*>(_mapLayer->getChildByTag(999));
            if (troopLayer) {
                DefenseSystem::getInstance()->updateBuildingDefense(troopLayer);
                TrapSystem::getInstance()->updateTrapDetection(troopLayer);
            }
        }
    }
    
    // 回放播放逻辑
    if (_recorder.isReplayMode() && _currentState == BattleState::FIGHTING) {
        updateReplay(dt);
    }
}

void BattleScene::switchState(BattleState newState) {
    CCLOG("##############################################");
    CCLOG("BattleScene::switchState");
    CCLOG("  From: %s", 
          _currentState == BattleState::PREPARE ? "PREPARE" : 
          _currentState == BattleState::FIGHTING ? "FIGHTING" : "RESULT");
    CCLOG("  To: %s", 
          newState == BattleState::PREPARE ? "PREPARE" : 
          newState == BattleState::FIGHTING ? "FIGHTING" : "RESULT");
    CCLOG("##############################################");
    
    _currentState = newState;
    if (_hudLayer) _hudLayer->updatePhase(newState);

    auto audioManager = AudioManager::getInstance();

    switch (_currentState) {
        case BattleState::PREPARE:
            CCLOG(">>> Entering PREPARE state");
            _stateTimer = 30.0f;
            
            // 播放准备战斗音乐（循环）
            CCLOG(">>> Attempting to play combat planning music...");
            _combatPlanningMusicID = audioManager->playBackgroundMusic(
                "Audios/combat_planning_music.mp3", 
                0.8f,
                true
            );
            
            if (_combatPlanningMusicID == -1) {
                CCLOG(">>> ERROR: Failed to start combat planning music!");
            } else {
                CCLOG(">>> Combat planning music started (ID: %d)", _combatPlanningMusicID);
            }
            break;
            
        case BattleState::FIGHTING:
            CCLOG(">>> Entering FIGHTING state");
            _stateTimer = 180.0f;
            
            // 保存当前地图状态到回放数据
            if (!_recorder.isReplayMode()) {
                _recorder.saveCurrentMap();
            }
            
            // 停止准备音乐，播放战斗音乐（循环）
            CCLOG(">>> Stopping combat planning music (ID: %d)", _combatPlanningMusicID);
            if (_combatPlanningMusicID != -1) {
                audioManager->stopAudio(_combatPlanningMusicID);
                _combatPlanningMusicID = -1;
                CCLOG(">>> Combat planning music stopped");
            } else {
                CCLOG(">>> No combat planning music to stop");
            }
            
            CCLOG(">>> Attempting to play combat music...");
            _combatMusicID = audioManager->playBackgroundMusic(
                "Audios/combat_music.mp3", 
                0.8f,
                true
            );
            
            if (_combatMusicID == -1) {
                CCLOG(">>> ERROR: Failed to start combat music!");
            } else {
                CCLOG(">>> Combat music started (ID: %d)", _combatMusicID);
            }
            
            // 进入战斗状态时显示进度UI
            if (_battleProgressUI) {
                _battleProgressUI->show();
                CCLOG(">>> Battle progress UI shown");
            }
            break;
            
        case BattleState::RESULT:
            CCLOG(">>> Entering RESULT state");
            _stateTimer = 0;
            
            // 停止所有战斗音乐
            CCLOG(">>> Stopping all combat music");
            CCLOG("    Planning music ID: %d", _combatPlanningMusicID);
            CCLOG("    Combat music ID: %d", _combatMusicID);
            
            if (_combatPlanningMusicID != -1) {
                audioManager->stopAudio(_combatPlanningMusicID);
                _combatPlanningMusicID = -1;
            }
            if (_combatMusicID != -1) {
                audioManager->stopAudio(_combatMusicID);
                _combatMusicID = -1;
            }
            
            CCLOG(">>> All music stopped");
            
            // 播放进度UI的结算动画
            if (_battleProgressUI) {
                _battleProgressUI->playResultAnimation([this]() {
                    CCLOG(">>> Progress UI animation completed");
                });
            }
            
            if (!_resultLayer) {
                // 传递消耗数据和掠夺资源给结算页面
                _resultLayer = BattleResultLayer::createWithData(
                    _usedTroops, _troopLevels, _lootedGold, _lootedElixir);
                this->addChild(_resultLayer, 100);
            }
            break;
    }
    
    CCLOG("##############################################");
    CCLOG("BattleScene::switchState - END");
    CCLOG("##############################################");
}

void BattleScene::loadEnemyVillage() {
    if (_mapLayer) {
        // 刷新地图
        _mapLayer->reloadMap();
        
        // 初始化资源掠夺数据
        _lootedGold = 0;
        _lootedElixir = 0;
        
        // 从VillageDataManager获取战斗地图数据
        auto dataManager = VillageDataManager::getInstance();
        const auto& battleMap = dataManager->getBattleMapData();
        
        _totalLootableGold = battleMap.lootableGold;
        _totalLootableElixir = battleMap.lootableElixir;
        
        // 计算每个储存建筑的资源
        if (battleMap.goldStorageCount > 0) {
            _goldPerStorage = _totalLootableGold / battleMap.goldStorageCount;
        } else {
            _goldPerStorage = 0;
        }
        
        if (battleMap.elixirStorageCount > 0) {
            _elixirPerStorage = _totalLootableElixir / battleMap.elixirStorageCount;
        } else {
            _elixirPerStorage = 0;
        }
        
        CCLOG("BattleScene: Loot initialized - Gold: %d (%d per storage), Elixir: %d (%d per storage)",
              _totalLootableGold, _goldPerStorage, _totalLootableElixir, _elixirPerStorage);
        
        // 通知HUD更新资源显示
        if (_hudLayer) {
            _hudLayer->initLootDisplay(_totalLootableGold, _totalLootableElixir);
        }
    }
}

void BattleScene::onNextOpponentClicked() {
    if (_isSearching) return;

    // 执行云层过渡动画
    performCloudTransition([this]() {
        // 云层遮住屏幕时刷新地图
        this->loadEnemyVillage();

        // 重置侦查倒计时
        _stateTimer = 30.0f;
    }, false);
}

void BattleScene::onEndBattleClicked() {
    CCLOG("========================================");
    CCLOG("BattleScene::onEndBattleClicked - START");
    CCLOG("========================================");

    // 停止所有战斗音乐
    auto audioManager = AudioManager::getInstance();
    if (_combatPlanningMusicID != -1) {
        audioManager->stopAudio(_combatPlanningMusicID);
        _combatPlanningMusicID = -1;
        CCLOG("BattleScene: Stopped combat planning music");
    }
    if (_combatMusicID != -1) {
        audioManager->stopAudio(_combatMusicID);
        _combatMusicID = -1;
        CCLOG("BattleScene: Stopped combat music");
    }

    // 保存回放数据（只在正常战斗模式下）
    if (!_recorder.isReplayMode()) {
        CCLOG("----------------------------------------");
        CCLOG("BattleScene: Normal battle mode detected");
        CCLOG("BattleScene: Stopping recording and saving replay...");
        CCLOG("  Current state:");
        CCLOG("    - Looted Gold: %d", _lootedGold);
        CCLOG("    - Looted Elixir: %d", _lootedElixir);
        CCLOG("    - Used Troops: %zu types", _usedTroops.size());

        for (const auto& pair : _usedTroops) {
            if (pair.second > 0) {
                CCLOG("      * Troop %d: %d used", pair.first, pair.second);
            }
        }

        // 调用保存回放（委托给_recorder）
        stopRecording();

        CCLOG("BattleScene: ✅ Replay saved successfully!");
        CCLOG("----------------------------------------");
    } else {
        CCLOG("BattleScene: In replay mode, skipping recording");
    }

    // 从村庄数据中扣除消耗的兵种（回放模式下不扣除）
    auto dataManager = VillageDataManager::getInstance();
    if (!_recorder.isReplayMode()) {
        for (const auto& pair : _usedTroops) {
            int troopId = pair.first;
            int usedCount = pair.second;
            if (usedCount > 0) {
                dataManager->removeTroop(troopId, usedCount);
                CCLOG("BattleScene: Deducted %d of troop %d from village data", usedCount, troopId);
            }
        }
    }

    // 战斗结束时让兵种停止AI但保持在原地
    auto troopLayer = dynamic_cast<BattleTroopLayer*>(_mapLayer->getChildByTag(999));
    if (troopLayer) {
        auto allUnits = troopLayer->getAllUnits();
        for (auto unit : allUnits) {
            if (unit && !unit->isDead()) {
                unit->stopAllActions();
                unit->playIdleAnimation();
                CCLOG("BattleScene: Unit stopped and set to idle");
            }
        }
    }

    switchState(BattleState::RESULT);

    CCLOG("========================================");
    CCLOG("BattleScene::onEndBattleClicked - END");
    CCLOG("========================================");
}

void BattleScene::onReturnHomeClicked() {
    CCLOG("BattleScene: Returning home, resetting building states");

    // 停止所有战斗音乐
    auto audioManager = AudioManager::getInstance();
    if (_combatPlanningMusicID != -1) {
        audioManager->stopAudio(_combatPlanningMusicID);
        _combatPlanningMusicID = -1;
    }
    if (_combatMusicID != -1) {
        audioManager->stopAudio(_combatMusicID);
        _combatMusicID = -1;
    }
    
    // 停止所有音频（包括结算音乐等可能遗漏的音频）
    audioManager->stopAllAudio();
    CCLOG("BattleScene: All audio stopped");
    
    // 清除资源回调，防止回调访问即将销毁的HUDLayer
    auto dataManager = VillageDataManager::getInstance();
    dataManager->setResourceCallback(nullptr);
    CCLOG("BattleScene: Resource callback cleared before adding loot");
    
    // 将掠夺的资源添加到玩家账户
    if (_lootedGold > 0) {
        dataManager->addGold(_lootedGold);
        CCLOG("BattleScene: Added %d gold to player", _lootedGold);
    }
    if (_lootedElixir > 0) {
        dataManager->addElixir(_lootedElixir);
        CCLOG("BattleScene: Added %d elixir to player", _lootedElixir);
    }
    
    // 退出战斗模式，切换回村庄数据源
    dataManager->setInBattleMode(false);
    
    // 只在返回村庄时才重置建筑状态
    BattleProcessController::getInstance()->resetBattleState();
    CCLOG("BattleScene: Building states reset when returning home");
    
    // 清理建筑摧毁事件监听器
    if (_buildingDestroyedListener) {
        _eventDispatcher->removeEventListener(_buildingDestroyedListener);
        _buildingDestroyedListener = nullptr;
    }
    
    // 重置战斗进度UI
    if (_battleProgressUI) {
        _battleProgressUI->reset();
        CCLOG("BattleScene: Battle progress UI reset");
    }
  
    auto homeScene = VillageScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, homeScene));
}

// --- 核心：云层过渡逻辑 ---
void BattleScene::performCloudTransition(std::function<void()> onMapReloadCallback, bool isInitialEntry) {
    if (!_cloudSprite) {
        // 如果没有云层图片，直接执行回调，防止卡死
        if (onMapReloadCallback) onMapReloadCallback();
        return;
    }

    _isSearching = true;
    if (_hudLayer) _hudLayer->setButtonsEnabled(false); // 禁用按钮防止乱点

    // 随机等待时间 (0.1s - 2.0s)
    float randomWait = cocos2d::RandomHelper::random_real(0.1f, 2.0f);

    if (isInitialEntry) {
        // 情况A：刚进入场景，云已经在了，只需要执行淡出
        _cloudSprite->setOpacity(255);
        _cloudSprite->setVisible(true);

        _cloudSprite->runAction(Sequence::create(
            FadeOut::create(0.5f),   // 淡出
            CallFunc::create([this]() {
            _cloudSprite->setVisible(false);
            _isSearching = false;
            if (_hudLayer) _hudLayer->setButtonsEnabled(true);
        }),
            nullptr
        ));
    } else {
        // 情况B：点击"下一个"，需要 淡入 -> 随机等待 -> 回调(刷新地图) -> 淡出
        _cloudSprite->setVisible(true);
        _cloudSprite->setOpacity(0);

        _cloudSprite->runAction(Sequence::create(
            FadeIn::create(0.2f), // 1. 快速淡入变白
            CallFunc::create([=]() {
            // 云层完全遮挡时，刷新地图数据，玩家看不见突变
            if (onMapReloadCallback) onMapReloadCallback();
        }),
            DelayTime::create(randomWait), // 2. 模拟搜索的随机等待 (0.1s - 2.0s)
            FadeOut::create(0.4f),         // 3. 淡出显示新地图
            CallFunc::create([this]() {
            _cloudSprite->setVisible(false);
            _isSearching = false;
            if (_hudLayer) _hudLayer->setButtonsEnabled(true);
        }),
            nullptr
        ));
    }
}
void BattleScene::setupTouchListener() {
    // 如果已经有监听器，先清理
    cleanupTouchListener();

    _touchListener = EventListenerTouchOneByOne::create();

    // 关键修改：不要全局吞噬，让 UI 层可以接收事件
    _touchListener->setSwallowTouches(false);

    _touchListener->onTouchBegan = CC_CALLBACK_2(BattleScene::onTouchBegan, this);
    _touchListener->onTouchMoved = CC_CALLBACK_2(BattleScene::onTouchMoved, this);
    _touchListener->onTouchEnded = CC_CALLBACK_2(BattleScene::onTouchEnded, this);

    // 使用固定优先级（数字越小优先级越高），确保比 MoveMapController 更早处理
    _eventDispatcher->addEventListenerWithFixedPriority(_touchListener, -1);

    CCLOG("BattleScene: Touch listener setup with fixed priority -1");

    // 初始化触摸状态
    _touchStartPos = Vec2::ZERO;
    _isTouchMoving = false;
}

void BattleScene::cleanupTouchListener() {
    if (_touchListener) {
        _eventDispatcher->removeEventListener(_touchListener);
        _touchListener = nullptr;
        CCLOG("BattleScene: Touch listener cleaned up");
    }
}

bool BattleScene::onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event) {
    _touchStartPos = touch->getLocation();
    _currentTouchPos = _touchStartPos;
    _isTouchMoving = false;
    _isLongPressDeploying = false;

    // 1. 检查有没有选中兵种
    if (!_hudLayer || _hudLayer->getSelectedTroopId() == -1) {
        CCLOG("BattleScene: No troop selected, passing event to map controller");
        return false; // 没选兵，把事件留给地图拖动和 UI
    }

    // 2. 如果选中了兵种，检查是否点击在 UI 区域上
    if (isTouchOnUI(touch->getLocation())) {
        CCLOG("BattleScene: Touch on UI, passing event");
        return false; // 让 UI 处理（比如点击兵种卡片取消选择）
    }

    CCLOG("BattleScene: Troop selected (ID=%d), handling touch event", _hudLayer->getSelectedTroopId());
    
    //立即尝试放置一次
    if (tryDeployTroopAt(_touchStartPos)) {
        _isLongPressDeploying = true;
        
        // 启动长按定时器：延迟0.3秒后开始，每0.15秒放置一次
        this->schedule([this](float) {
            if (_isLongPressDeploying && !_isTouchMoving) {
                tryDeployTroopAt(_currentTouchPos);
            }
        }, 0.15f, CC_REPEAT_FOREVER, 0.3f, "long_press_deploy");
    }
    
    return true; // 开始跟踪触摸
}

void BattleScene::onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event) {
    Vec2 currentPos = touch->getLocation();
    _currentTouchPos = currentPos;  // 【新增】更新当前触摸位置
    
    float distance = _touchStartPos.distance(currentPos);

    // 如果移动距离超过阈值，判定为拖动地图
    const float DRAG_THRESHOLD = 15.0f;
    if (distance > DRAG_THRESHOLD) {
        _isTouchMoving = true;
        // 【新增】移动时停止长按放置
        stopLongPressDeploy();
    }
}

void BattleScene::onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event) {
    //停止长按放置
    stopLongPressDeploy();
    
    // 如果没有选中兵种，不处理
    if (!_hudLayer || _hudLayer->getSelectedTroopId() == -1) {
        return;
    }

    // 如果是拖动操作，取消兵种选择
    if (_isTouchMoving) {
        CCLOG("BattleScene: Drag detected, deselecting troop");
        _hudLayer->clearTroopSelection();
        return;
    }
    
    // 放置逻辑已在 onTouchBegan 中完成，这里不需要再放置
    CCLOG("BattleScene: Touch ended, deployment already handled in onTouchBegan");
}

// ==========停止长按放置 ==========
void BattleScene::stopLongPressDeploy() {
    if (_isLongPressDeploying) {
        _isLongPressDeploying = false;
        this->unschedule("long_press_deploy");
        CCLOG("BattleScene: Long press deploy stopped");
    }
}

// ==========尝试在指定位置放置兵种 ==========
bool BattleScene::tryDeployTroopAt(const cocos2d::Vec2& touchPos) {
    // 检查是否在 UI 区域
    if (isTouchOnUI(touchPos)) {
        return false;
    }
    
    // 检查是否选中兵种
    if (!_hudLayer || _hudLayer->getSelectedTroopId() == -1) {
        return false;
    }
    
    int troopId = _hudLayer->getSelectedTroopId();
    
    // 检查兵种剩余数量
    if (getRemainingTroopCount(troopId) <= 0) {
        showPlacementTip("该兵种已用完", touchPos);
        stopLongPressDeploy();  // 兵种用完时停止长按
        return false;
    }

    // 转换坐标
    Vec2 worldPos = _mapLayer->convertToNodeSpace(touchPos);
    Vec2 gridPos = GridMapUtils::pixelToGrid(worldPos);
    int gx = (int)std::round(gridPos.x);
    int gy = (int)std::round(gridPos.y);

    // 边界检查
    if (!GridMapUtils::isValidGridPosition(gx, gy)) {
        return false;
    }

    // 检查点击位置是否在建筑禁区内
    auto dataManager = VillageDataManager::getInstance();
    const auto& buildings = dataManager->getAllBuildings();
    
    for (const auto& building : buildings) {
        if (building.state == BuildingInstance::State::PLACING) continue;
        if (building.type >= 400 && building.type < 500) continue;  // 跳过陷阱

        auto config = BuildingConfig::getInstance()->getConfig(building.type);
        if (!config) continue;

        int bx = building.gridX;
        int by = building.gridY;
        int bw = config->gridWidth;
        int bh = config->gridHeight;

        int minX = std::max(0, bx - 1);
        int maxX = std::min(49, bx + bw);
        int minY = std::max(0, by - 1);
        int maxY = std::min(49, by + bh);

        if (gx >= minX && gx <= maxX && gy >= minY && gy <= maxY) {
            // 在建筑禁区内，静默返回（长按时不显示提示）
            return false;
        }
    }

    // 生成士兵
    auto troopLayer = dynamic_cast<BattleTroopLayer*>(_mapLayer->getChildByTag(999));
    if (!troopLayer) {
        CCLOG("BattleScene: TroopLayer not found!");
        return false;
    }

    // 兵种ID映射
    std::string name = "Barbarian";
    if (troopId == 1001) name = "Barbarian";
    else if (troopId == 1002) name = "Archer";
    else if (troopId == 1003) name = "Goblin";
    else if (troopId == 1004) name = "Giant";
    else if (troopId == 1005) name = "Wall_Breaker";
    else if (troopId == 1006) name = "Balloon";

    auto unit = troopLayer->spawnUnit(name, gx, gy);
    if (!unit) {
        return false;
    }

    // 播放部署音效
    if (troopId == 1001) AudioManager::getInstance()->playEffect("Audios/barbarian_deploy.mp3", 0.8f);
    else if (troopId == 1002) AudioManager::getInstance()->playEffect("Audios/archer_deploy.mp3", 0.8f);
    else if (troopId == 1003) AudioManager::getInstance()->playEffect("Audios/goblin_deploy.mp3", 0.8f);
    else if (troopId == 1004) AudioManager::getInstance()->playEffect("Audios/giant_deploy.mp3", 0.8f);
    else if (troopId == 1005) AudioManager::getInstance()->playEffect("Audios/wall_breaker_deploy.mp3", 0.8f);
    else if (troopId == 1006) AudioManager::getInstance()->playEffect("Audios/balloon_deploy.mp3", 0.8f);

    // 记录并启动AI
    recordTroopDeployment(troopId, gx, gy);
    BattleProcessController::getInstance()->startUnitAI(unit, troopLayer);

    // 更新数量统计
    _remainingTroops[troopId]--;
    _usedTroops[troopId]++;
    CCLOG("BattleScene: Troop %d deployed at (%d,%d). Remaining: %d", 
          troopId, gx, gy, _remainingTroops[troopId]);

    // 更新 HUD
    if (_hudLayer) {
        _hudLayer->updateTroopCount(troopId, _remainingTroops[troopId]);
    }

    // 自动切换到战斗状态
    if (_currentState == BattleState::PREPARE) {
        switchState(BattleState::FIGHTING);
    }

    return true;
}

bool BattleScene::isTouchOnUI(const Vec2& touchPos) {
    if (!_hudLayer) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 定义 UI 区域（根据你的布局调整）
    // 1. 顶部区域（倒计时等）
    const float TOP_UI_HEIGHT = 100.0f;
    if (touchPos.y > visibleSize.height - TOP_UI_HEIGHT) {
        return true;
    }

    // 2. 底部区域（兵种栏和按钮）
    const float BOTTOM_UI_HEIGHT = 150.0f;
    if (touchPos.y < BOTTOM_UI_HEIGHT) {
        return true;
    }

    // 3. 右下角按钮区域（"下一个"按钮）
    const float RIGHT_BUTTON_WIDTH = 100.0f;
    const float RIGHT_BUTTON_HEIGHT = 100.0f;
    if (touchPos.x > visibleSize.width - RIGHT_BUTTON_WIDTH &&
        touchPos.y < RIGHT_BUTTON_HEIGHT) {
        return true;
    }

    return false;
}

void BattleScene::showPlacementTip(const std::string& message, const cocos2d::Vec2& pos) {
    // 创建提示标签
    auto label = Label::createWithSystemFont(message, "Arial", 28);
    if (!label) return;
    
    // 设置样式（红色字体更醒目）
    label->setTextColor(Color4B::RED);
    label->enableOutline(Color4B::BLACK, 2);
    
    // 设置位置（在触摸点上方显示）
    label->setPosition(pos.x, pos.y + 50);
    
    // 添加到场景
    this->addChild(label, 200);  // 高层级确保可见
    
    // 动画：向上漂浮并淡出
    label->runAction(Sequence::create(
        Spawn::create(
            MoveBy::create(1.5f, Vec2(0, 30)),  // 向上移动
            FadeOut::create(1.5f),              // 淡出
            nullptr
        ),
        RemoveSelf::create(),  // 自动移除
        nullptr
    ));
}

// ========== 兵种追踪系统实现 ==========

void BattleScene::initBattleTroops() {
    // 清空之前的数据
    _remainingTroops.clear();
    _usedTroops.clear();
    _troopLevels.clear();
    
    // 从村庄数据复制兵种数量
    auto dataManager = VillageDataManager::getInstance();
    const auto& troops = dataManager->getAllTroops();
    
    for (const auto& pair : troops) {
        int troopId = pair.first;
        int count = pair.second;
        
        if (count > 0) {
            _remainingTroops[troopId] = count;
            _usedTroops[troopId] = 0;
            _troopLevels[troopId] = dataManager->getTroopLevel(troopId);
            
            CCLOG("BattleScene::initBattleTroops - Troop %d: count=%d, level=%d", 
                  troopId, count, _troopLevels[troopId]);
        }
    }
    
    CCLOG("BattleScene: Battle troops initialized with %zu types", _remainingTroops.size());
}

int BattleScene::getRemainingTroopCount(int troopId) const {
    auto it = _remainingTroops.find(troopId);
    if (it != _remainingTroops.end()) {
        return it->second;
    }
    return 0;
}

// ========== 资源掠夺系统实现 ==========

void BattleScene::addLootedGold(int amount) {
    _lootedGold += amount;
    if (_lootedGold > _totalLootableGold) {
        _lootedGold = _totalLootableGold;
    }
    CCLOG("BattleScene: Looted gold: %d/%d", _lootedGold, _totalLootableGold);
    
    // 更新 HUD 显示
    if (_hudLayer) {
        _hudLayer->updateLootDisplay(_lootedGold, _lootedElixir, 
                                      _totalLootableGold, _totalLootableElixir);
    }
}

void BattleScene::addLootedElixir(int amount) {
    _lootedElixir += amount;
    if (_lootedElixir > _totalLootableElixir) {
        _lootedElixir = _totalLootableElixir;
    }
    CCLOG("BattleScene: Looted elixir: %d/%d", _lootedElixir, _totalLootableElixir);
    
    // 更新 HUD 显示
    if (_hudLayer) {
        _hudLayer->updateLootDisplay(_lootedGold, _lootedElixir, 
                                      _totalLootableGold, _totalLootableElixir);
    }
}

void BattleScene::setupBuildingDestroyedListener() {
    _buildingDestroyedListener = EventListenerCustom::create(
        "EVENT_BUILDING_DESTROYED",
        CC_CALLBACK_1(BattleScene::onBuildingDestroyed, this)
    );
    _eventDispatcher->addEventListenerWithSceneGraphPriority(_buildingDestroyedListener, this);
    CCLOG("BattleScene: Building destroyed listener setup");
}

void BattleScene::onBuildingDestroyed(cocos2d::EventCustom* event) {
    BuildingInstance* building = static_cast<BuildingInstance*>(event->getUserData());
    if (!building) return;
    
    CCLOG("BattleScene: Building destroyed - ID=%d, Type=%d", building->id, building->type);
    
    // 检查是否是储存建筑
    if (building->type == 204) {  // 储金罐
        addLootedGold(_goldPerStorage);
        CCLOG("BattleScene: Gold storage destroyed, looted %d gold", _goldPerStorage);
    }
    else if (building->type == 205) {  // 圣水瓶
        addLootedElixir(_elixirPerStorage);
        CCLOG("BattleScene: Elixir storage destroyed, looted %d elixir", _elixirPerStorage);
    }
    
    //检查是否所有建筑都已被摧毁
    if (_currentState == BattleState::FIGHTING) {
        checkAllBuildingsDestroyed();
    }
}

void BattleScene::checkAllBuildingsDestroyed() {
    auto dataManager = VillageDataManager::getInstance();
    const auto& buildings = dataManager->getAllBuildings();
    
    // 遍历所有建筑，检查是否还有未摧毁的建筑（不包括城墙）
    bool allDestroyed = true;
    int totalBuildings = 0;
    int destroyedBuildings = 0;
    
    for (const auto& building : buildings) {
        // 跳过放置中的建筑和城墙（城墙类型为303）
        if (building.state == BuildingInstance::State::PLACING) {
            continue;
        }
        if (building.type == 303 || building.type == 401 || building.type == 404) {  // 城墙和陷阱不计入判定
            continue;
        }
        
        totalBuildings++;
        
        if (building.isDestroyed || building.currentHP <= 0) {
            destroyedBuildings++;
        } else {
            allDestroyed = false;
        }
    }
    
    CCLOG("BattleScene: Building check - %d/%d destroyed", destroyedBuildings, totalBuildings);
    
    // 如果所有建筑都已被摧毁，自动进入结算页面
    if (allDestroyed && totalBuildings > 0) {
        CCLOG("BattleScene: All buildings destroyed! Automatically ending battle.");
        
        // 使用延迟调度器，让玩家有时间看到最后一个建筑被摧毁的效果
        this->scheduleOnce([this](float) {
            onEndBattleClicked();
        }, 1.5f, "auto_end_battle");
    }
}

void BattleScene::onProgressUpdated(EventCustom* event) {
    auto data = static_cast<DestructionProgressEventData*>(event->getUserData());
    if (!data) return;

    //更新战斗进度UI
    if (_battleProgressUI) {
        _battleProgressUI->updateProgress(data->progress);
    }

    CCLOG("BattleScene: Progress updated to %.1f%%, Stars: %d",
          data->progress, data->stars);
}

void BattleScene::onStarAwarded(EventCustom* event) {
    auto data = static_cast<StarAwardedEventData*>(event->getUserData());
    if (!data) return;

    int starIndex = data->starIndex;

    CCLOG("BattleScene: Star #%d awarded! Reason: %s",
          starIndex + 1, data->reason.c_str());

    //更新战斗进度UI的星星
    if (_battleProgressUI) {
        _battleProgressUI->updateStars(starIndex + 1);
    }
}

// ========== 清理资源（合并版本）==========
void BattleScene::onExit() {
    CCLOG("BattleScene::onExit - Cleaning up resources");

    cleanupTouchListener();

    if (_progressListener) {
        Director::getInstance()->getEventDispatcher()->removeEventListener(_progressListener);
        _progressListener = nullptr;
    }

    if (_starListener) {
        Director::getInstance()->getEventDispatcher()->removeEventListener(_starListener);
        _starListener = nullptr;
    }

    Scene::onExit();
}

// ========== 录制系统实现（委托给 _recorder）==========

void BattleScene::startRecording() {
    _recorder.startRecording();
    CCLOG("BattleScene: Recording started (delegated to BattleRecorder)");
}

void BattleScene::recordTroopDeployment(int troopId, int gridX, int gridY) {
    _recorder.recordTroopDeployment(troopId, gridX, gridY);
}

void BattleScene::stopRecording() {
    _recorder.stopRecording(_lootedGold, _lootedElixir, _usedTroops, _troopLevels);
    CCLOG("BattleScene: Recording stopped (delegated to BattleRecorder)");
}

BattleScene* BattleScene::createReplayScene(const BattleReplayData& replayData) {
    auto scene = BattleScene::create();
    scene->_recorder.initReplayMode(replayData);
    return scene;
}

void BattleScene::startReplay() {
    if (!_recorder.isReplayMode()) return;

    const auto& replayData = _recorder.getReplayData();
    CCLOG("BattleScene: Starting replay playback with %zu events",
          replayData.troopEvents.size());

    _recorder.startReplay(_hudLayer, [this]() {
        switchState(BattleState::FIGHTING);
    });
}

void BattleScene::updateReplay(float dt) {
    auto troopLayer = dynamic_cast<BattleTroopLayer*>(_mapLayer->getChildByTag(999));
    if (!troopLayer) return;

    _recorder.updateReplay(dt, troopLayer, [this]() {
        CCLOG("BattleScene: All replay events deployed, preparing to show results...");

        this->scheduleOnce([this](float) {
            CCLOG("BattleScene: Showing actual battle results from replay data");

            // 回放结束时让兵种停止AI但不移除
            auto troopLayer = dynamic_cast<BattleTroopLayer*>(_mapLayer->getChildByTag(999));
            if (troopLayer) {
                auto allUnits = troopLayer->getAllUnits();
                for (auto unit : allUnits) {
                    if (unit && !unit->isDead()) {
                        unit->stopAllActions();
                        unit->playIdleAnimation();
                    }
                }
            }

            // 使用录制时保存的真实数据
            const auto& replayData = _recorder.getReplayData();
            if (_battleProgressUI) {
                _battleProgressUI->updateProgress((float)replayData.destructionPercentage);
                _battleProgressUI->updateStars(replayData.finalStars);
            }

            // 设置真实的战斗结果数据
            _lootedGold = replayData.lootedGold;
            _lootedElixir = replayData.lootedElixir;
            _usedTroops = replayData.usedTroops;
            _troopLevels = replayData.troopLevels;

            CCLOG("BattleScene: Replay results - Stars: %d, Destruction: %d%%, Gold: %d, Elixir: %d",
                  replayData.finalStars, replayData.destructionPercentage,
                  replayData.lootedGold, replayData.lootedElixir);

            switchState(BattleState::RESULT);

        }, 1.0f, "show_replay_result");
    });
}

void BattleScene::loadReplayMap() {
    _recorder.loadReplayMap(_mapLayer, _hudLayer);

    // 初始化资源掠夺数据（从回放数据读取）
    const auto& replayData = _recorder.getReplayData();
    _lootedGold = 0;
    _lootedElixir = 0;
    _totalLootableGold = replayData.lootedGold;
    _totalLootableElixir = replayData.lootedElixir;

    // 回放模式下重新初始化摧毁追踪
    DestructionTracker::getInstance()->initTracking();
    CCLOG("BattleScene: Destruction tracker re-initialized for replay mode");

    if (_recorder.isReplayMode()) {
        CCLOG("BattleScene: Skipping PREPARE phase, switching to FIGHTING immediately");
        switchState(BattleState::FIGHTING);
        startReplay();
    }
    CCLOG("BattleScene: Replay map loaded successfully");
}
