#pragma execution_character_set("utf-8")
#include "BattleScene.h"
#include "Layer/BattleMapLayer.h"
#include "Layer/BattleHUDLayer.h"
#include "Layer/BattleResultLayer.h"
#include "Scene/VillageScene.h"
#include "Layer/BattleTroopLayer.h"
#include "Controller/BattleProcessController.h"
#include "Manager/VillageDataManager.h"
#include "Manager/BuildingManager.h"
#include "Model/BuildingConfig.h"
#include "Util/FindPathUtil.h"
#include "Util/GridMapUtils.h"
#include "Util/RandomBattleMapGenerator.h"
#include "Component/DefenseBuildingAnimation.h"

USING_NS_CC;

Scene* BattleScene::createScene() {
    return BattleScene::create();
}

bool BattleScene::init() {
    if (!Scene::init()) return false;

    _stateTimer = 0.0f;
    _resultLayer = nullptr;
    _currentState = BattleState::PREPARE;
    _isSearching = false;

    // 【新增】初始化战斗兵种数据
    initBattleTroops();

    // 1. 地图层
    _mapLayer = BattleMapLayer::create();
    if (_mapLayer) {
        this->addChild(_mapLayer, 0);

        // 【新增】把 TroopLayer 加为 MapLayer 的子节点，Tag 设为 999
        auto troopLayer = BattleTroopLayer::create();
        troopLayer->setTag(999);
        _mapLayer->addChild(troopLayer, 10);
    }

    // 2. UI 层 (Z=10)
    _hudLayer = BattleHUDLayer::create();
    if (_hudLayer) this->addChild(_hudLayer, 10);

    // 3. 初始化云层遮罩 (Z=100, 放在最高层以遮挡一切)
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

    // 4. 【关键修复】先切换到战斗模式，再加载敌人数据
    VillageDataManager::getInstance()->setInBattleMode(true);
    CCLOG("BattleScene: Entered battle mode, data source switched to battle map");

    // 加载敌人数据（这会触发 reloadMap 并输出正确的建筑布局）
    loadEnemyVillage();

    // ========== 更新寻路地图 ==========(
    FindPathUtil::getInstance()->updatePathfindingMap();
    CCLOG("BattleScene: Pathfinding map updated for battle");
    // ===========================================

    switchState(BattleState::PREPARE);

    // 5. 启动时执行一次"进入动画"
    float randomStartDelay = cocos2d::RandomHelper::random_real(0.5f, 1.0f);
    this->scheduleOnce([this](float) {
        performCloudTransition(nullptr, true);
    }, randomStartDelay, "start_anim_delay");

    this->scheduleUpdate();

    // 【新增】最后开启触摸监听
    setupTouchListener();
    
    // 【新增】设置建筑摧毁事件监听
    setupBuildingDestroyedListener();

    // ========== 安全的测试代码：使用 schedule 的自动管理 ==========
    this->scheduleOnce([this](float) {
        auto buildingManager = _mapLayer->getBuildingManager();
        if (!buildingManager) {
            CCLOG("❌ BattleScene: BuildingManager is null!");
            return;
        }

        auto dataManager = VillageDataManager::getInstance();
        const auto& buildings = dataManager->getAllBuildings();

        CCLOG("🔫 STARTING CANNON ROTATION TEST");

        for (const auto& building : buildings) {
            if (building.type == 301) {  // 加农炮
                // 【关键修复】每次在 lambda 内部重新获取 anim 指针
                std::string scheduleKey = "test_cannon_" + std::to_string(building.id);
                int buildingId = building.id;  // 按值捕获 ID,而不是整个对象

                this->schedule([this, buildingId](float dt) {
                    // 每次回调时重新获取指针,确保安全
                    auto buildingManager = _mapLayer->getBuildingManager();
                    if (!buildingManager) return;

                    auto anim = buildingManager->getDefenseAnimation(buildingId);
                    if (!anim) {
                        // 如果动画对象已被销毁,取消该 schedule
                        this->unschedule("test_cannon_" + std::to_string(buildingId));
                        return;
                    }

                    // 获取建筑数据
                    auto dataManager = VillageDataManager::getInstance();
                    auto buildingPtr = dataManager->getBuildingById(buildingId);
                    if (!buildingPtr) {
                        this->unschedule("test_cannon_" + std::to_string(buildingId));
                        return;
                    }

                    static std::map<int, float> angles;
                    float& angle = angles[buildingId];

                    angle += 2.0f;
                    if (angle >= 360.0f) angle = 0.0f;

                    Vec2 buildingPos = GridMapUtils::gridToPixel(buildingPtr->gridX, buildingPtr->gridY);
                    Vec2 fakeTarget = Vec2(
                        buildingPos.x + 200 * cos(CC_DEGREES_TO_RADIANS(angle)),
                        buildingPos.y + 200 * sin(CC_DEGREES_TO_RADIANS(angle))
                    );

                    anim->aimAt(fakeTarget);
                }, 0.05f, scheduleKey);

                CCLOG("✅ Started rotation test for cannon ID=%d", building.id);
            }
        }
    }, 2.0f, "test_all_cannons");
    // ============================================================
    
    return true;
}

void BattleScene::update(float dt) {
    if (_currentState == BattleState::PREPARE || _currentState == BattleState::FIGHTING) {
        _stateTimer -= dt;
        if (_hudLayer) _hudLayer->updateTimer((int)_stateTimer);

        if (_stateTimer <= 0) {
            if (_currentState == BattleState::PREPARE) {
                onReturnHomeClicked(); // 侦查时间到回营
            } else if (_currentState == BattleState::FIGHTING) {
                onEndBattleClicked(); // 战斗时间到
            }
        }
    }
}

void BattleScene::switchState(BattleState newState) {
    _currentState = newState;
    if (_hudLayer) _hudLayer->updatePhase(newState);

    switch (_currentState) {
        case BattleState::PREPARE:
            _stateTimer = 30.0f;
            break;
        case BattleState::FIGHTING:
            _stateTimer = 180.0f;
            break;
        case BattleState::RESULT:
            _stateTimer = 0;
            if (!_resultLayer) {
                // 【修改】传递消耗数据和掠夺资源给结算页面
                _resultLayer = BattleResultLayer::createWithData(
                    _usedTroops, _troopLevels, _lootedGold, _lootedElixir);
                this->addChild(_resultLayer, 100);
            }
            break;
    }
}

void BattleScene::loadEnemyVillage() {
    if (_mapLayer) {
        // 这里执行刷新地图的逻辑
        _mapLayer->reloadMap();
        
        // 【新增】初始化资源掠夺数据
        _lootedGold = 0;
        _lootedElixir = 0;
        
        // 从 VillageDataManager 获取战斗地图数据
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
        
        // 【新增】通知 HUD 更新资源显示
        if (_hudLayer) {
            _hudLayer->initLootDisplay(_totalLootableGold, _totalLootableElixir);
        }
    }
}

void BattleScene::onNextOpponentClicked() {
    if (_isSearching) return; // 防止重复点击

    // 执行完整的云层过渡动画
    performCloudTransition([this]() {
        // 这里的代码会在云层完全变白（遮住屏幕）时执行
        this->loadEnemyVillage();

        // 重置侦查倒计时
        _stateTimer = 30.0f;
    }, false);
}

void BattleScene::onEndBattleClicked() {
    CCLOG("BattleScene: Ending battle, resetting building states");
    
    // 【新增】从村庄数据中扣除消耗的兵种
    auto dataManager = VillageDataManager::getInstance();
    for (const auto& pair : _usedTroops) {
        int troopId = pair.first;
        int usedCount = pair.second;
        if (usedCount > 0) {
            dataManager->removeTroop(troopId, usedCount);
            CCLOG("BattleScene: Deducted %d of troop %d from village data", usedCount, troopId);
        }
    }
    
    // 退出战斗前恢复所有建筑状态
    BattleProcessController::getInstance()->resetBattleState();
    
    switchState(BattleState::RESULT);
}

void BattleScene::onReturnHomeClicked() {
    CCLOG("BattleScene: Returning home, resetting building states");
    
    // ========== 关键修复：清除资源回调，防止回调访问即将销毁的HUDLayer ==========
    auto dataManager = VillageDataManager::getInstance();
    dataManager->setResourceCallback(nullptr);  // 清除旧的回调
    CCLOG("BattleScene: Resource callback cleared before adding loot");
    // ===========================================================================
    
    // 【新增】将掠夺的资源添加到玩家账户
    if (_lootedGold > 0) {
      dataManager->addGold(_lootedGold);
        CCLOG("BattleScene: Added %d gold to player", _lootedGold);
    }
    if (_lootedElixir > 0) {
        dataManager->addElixir(_lootedElixir);
        CCLOG("BattleScene: Added %d elixir to player", _lootedElixir);
    }
    
    // 【关键修复】退出战斗模式，切换回村庄数据源
    dataManager->setInBattleMode(false);
    
    // 退出战斗前恢复所有建筑状态
    BattleProcessController::getInstance()->resetBattleState();
    
    // 【新增】清理建筑摧毁事件监听器
    if (_buildingDestroyedListener) {
        _eventDispatcher->removeEventListener(_buildingDestroyedListener);
        _buildingDestroyedListener = nullptr;
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

void BattleScene::onExit() {
    CCLOG("BattleScene::onExit - Cleaning up resources");

    // 清理触摸监听器
    cleanupTouchListener();

    // 调用父类的 onExit
    Scene::onExit();
}

bool BattleScene::onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event) {
    _touchStartPos = touch->getLocation();
    _isTouchMoving = false;

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
    return true; // 开始跟踪触摸，等待判断是点击还是拖动
}

void BattleScene::onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event) {
    Vec2 currentPos = touch->getLocation();
    float distance = _touchStartPos.distance(currentPos);

    // 如果移动距离超过阈值，判定为拖动地图
    const float DRAG_THRESHOLD = 15.0f;
    if (distance > DRAG_THRESHOLD) {
        _isTouchMoving = true;
    }
}

void BattleScene::onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event) {
    // 如果没有选中兵种，不处理
    if (!_hudLayer || _hudLayer->getSelectedTroopId() == -1) {
        return;
    }

    // 如果是拖动操作，取消兵种选择并允许地图拖动
    if (_isTouchMoving) {
        CCLOG("BattleScene: Drag detected, deselecting troop");
        _hudLayer->clearTroopSelection();
        return;
    }

    // 到这里说明是点击操作，处理下兵逻辑
    Vec2 touchPos = touch->getLocation();

    // 再次检查是否在 UI 区域（可能是快速点击）
    if (isTouchOnUI(touchPos)) {
        CCLOG("BattleScene: Touch ended on UI, ignoring");
        return;
    }

    CCLOG("BattleScene: Tap detected, spawning troop");

    // 2. 转换坐标
    Vec2 worldPos = _mapLayer->convertToNodeSpace(touchPos);
    Vec2 gridPos = GridMapUtils::pixelToGrid(worldPos);
    int gx = (int)std::round(gridPos.x);
    int gy = (int)std::round(gridPos.y);

    CCLOG("BattleScene: Touch at grid(%d, %d)", gx, gy);

    // 3. 简单的边界检查 (红线逻辑以后再细化)
    if (!GridMapUtils::isValidGridPosition(gx, gy)) {
        CCLOG("BattleScene: Invalid grid position, ignoring");
        return;
    }
    
    // 4. 检查点击位置是否有建筑或城墙（包括已摧毁的）
    // 注意：不能用 getBuildingAtGrid，因为它会跳过已摧毁建筑
    auto dataManager = VillageDataManager::getInstance();
    const auto& buildings = dataManager->getAllBuildings();
    bool isOnBuilding = false;
    
    for (const auto& building : buildings) {
        // 跳过放置中的建筑
        if (building.state == BuildingInstance::State::PLACING) {
            continue;
        }
        
        // 获取建筑配置以确定尺寸
        auto config = BuildingConfig::getInstance()->getConfig(building.type);
        if (!config) continue;
        
        // 检查点击格子是否在建筑范围内
        int bx = building.gridX;
        int by = building.gridY;
        int bw = config->gridWidth;
        int bh = config->gridHeight;
        
        if (gx >= bx && gx < bx + bw && gy >= by && gy < by + bh) {
            isOnBuilding = true;
            CCLOG("BattleScene: Cannot deploy on building (ID=%d, type=%d, destroyed=%s) at grid(%d, %d)", 
                  building.id, building.type, building.isDestroyed ? "true" : "false", gx, gy);
            break;
        }
    }
    
    if (isOnBuilding) {
        showPlacementTip("无法在建筑上部署兵种", touchPos);
        return;
    }

    // 【新增】检查兵种剩余数量
    int troopId = _hudLayer->getSelectedTroopId();
    if (getRemainingTroopCount(troopId) <= 0) {
        showPlacementTip("该兵种已用完", touchPos);
        return;
    }

    // 4. 生成士兵并启动 AI
    auto troopLayer = dynamic_cast<BattleTroopLayer*>(_mapLayer->getChildByTag(999));
    if (troopLayer) {
        // 兵种ID映射
        std::string name = "Barbarian";

        if (troopId == 1001) {
            name = "Barbarian";
        } else if (troopId == 1002) {
            name = "Archer";
        } else if (troopId == 1003) {
            name = "Goblin";
        } else if (troopId == 1004) {
            name = "Giant";
        } else {
            CCLOG("BattleScene: Unknown troop ID %d, defaulting to Barbarian", troopId);
        }

        CCLOG("BattleScene: Spawning %s (ID %d) at grid(%d, %d)", name.c_str(), troopId, gx, gy);
        auto unit = troopLayer->spawnUnit(name, gx, gy);
        if (unit) {
            auto controller = BattleProcessController::getInstance();
            controller->startUnitAI(unit, troopLayer);

            // 【新增】更新兵种数量统计
            _remainingTroops[troopId]--;
            _usedTroops[troopId]++;
            CCLOG("BattleScene: Troop %d deployed. Remaining: %d, Used: %d", 
                  troopId, _remainingTroops[troopId], _usedTroops[troopId]);
            
            // 【新增】更新 HUD 显示
            if (_hudLayer) {
                _hudLayer->updateTroopCount(troopId, _remainingTroops[troopId]);
            }

            // 自动切换到战斗状态
            if (_currentState == BattleState::PREPARE) {
                CCLOG("BattleScene: Switching from PREPARE to FIGHTING");
                switchState(BattleState::FIGHTING);
            }
        } else {
            CCLOG("BattleScene: Failed to spawn unit");
        }
    } else {
        CCLOG("BattleScene: TroopLayer not found!");
    }
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
    
    // 【新增】检查是否所有建筑都已被摧毁
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
        if (building.type == 303) {  // 城墙不计入判定
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
