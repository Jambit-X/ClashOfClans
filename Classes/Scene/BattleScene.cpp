#include "BattleScene.h"
#include "Layer/BattleMapLayer.h"
#include "Layer/BattleHUDLayer.h"
#include "Layer/BattleResultLayer.h"
#include "Scene/VillageScene.h"
#include"Layer/BattleTroopLayer.h"
#include"Util/FindPathUtil.h"
#include"Util/GridMapUtils.h"

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

    // 1. 地图层
    _mapLayer = BattleMapLayer::create();
    if (_mapLayer) {
        this->addChild(_mapLayer, 0);

        // 【新增】把 TroopLayer 加为 MapLayer 的子节点，Tag 设为 999
        auto troopLayer = BattleTroopLayer::create();
        troopLayer->setTag(999);
        _mapLayer->addChild(troopLayer, 10); // Z=10 盖在建筑上面
    }



    // 2. UI 层 (Z=10)
    _hudLayer = BattleHUDLayer::create();
    if (_hudLayer) this->addChild(_hudLayer, 10);

    // 3. 初始化云层遮罩 (Z=100, 放在最高层以遮挡一切)
    // 路径: Resources/UI/battle/battle-prepare/cloud.jpg
    _cloudSprite = Sprite::create("UI/battle/battle-prepare/cloud.jpg");
    if (_cloudSprite) {
        auto visibleSize = Director::getInstance()->getVisibleSize();
        _cloudSprite->setPosition(visibleSize.width / 2, visibleSize.height / 2);

        // 计算缩放以覆盖全屏 (scale 稍微大一点防止黑边)
        float scaleX = visibleSize.width / _cloudSprite->getContentSize().width;
        float scaleY = visibleSize.height / _cloudSprite->getContentSize().height;
        float finalScale = std::max(scaleX, scaleY) * 1.1f;
        _cloudSprite->setScale(finalScale);

        // 初始状态：完全不透明 (模拟刚点进战斗时的搜索状态)
        _cloudSprite->setOpacity(255);
        _cloudSprite->setVisible(true);

        this->addChild(_cloudSprite, 100);
    }

    // 4. 加载敌人数据
    loadEnemyVillage();
    switchState(BattleState::PREPARE);

    // 5. 启动时执行一次“进入动画”（延迟一下再淡出，让玩家看清搜索云层）
    float randomStartDelay = cocos2d::RandomHelper::random_real(0.5f, 1.0f);
    this->scheduleOnce([this](float) {
        performCloudTransition(nullptr, true);
        }, randomStartDelay, "start_anim_delay");

    this->scheduleUpdate();
   
    // 【新增】最后开启触摸监听
    setupTouchListener();
    return true;
}

void BattleScene::update(float dt) {
    if (_currentState == BattleState::PREPARE || _currentState == BattleState::FIGHTING) {
        _stateTimer -= dt;
        if (_hudLayer) _hudLayer->updateTimer((int)_stateTimer);

        if (_stateTimer <= 0) {
            if (_currentState == BattleState::PREPARE) {
                onReturnHomeClicked(); // 侦查时间到回营
            }
            else if (_currentState == BattleState::FIGHTING) {
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
                _resultLayer = BattleResultLayer::create();
                this->addChild(_resultLayer, 100);
            }
            break;
    }
}

void BattleScene::loadEnemyVillage() {
    if (_mapLayer) {
        // 这里执行刷新地图的逻辑
        _mapLayer->reloadMap();
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
    switchState(BattleState::RESULT);
}

void BattleScene::onReturnHomeClicked() {
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
    }
    else {
        // 情况B：点击“下一个”，需要 淡入 -> 随机等待 -> 回调(刷新地图) -> 淡出
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
   _hudLayer->clearTroopSelection(); // 新增方法：清除兵种选择
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

    // 4. 生成士兵并启动 AI
 auto troopLayer = dynamic_cast<BattleTroopLayer*>(_mapLayer->getChildByTag(999));
    if (troopLayer) {
        int troopId = _hudLayer->getSelectedTroopId();

   // 简单映射名称 (实际项目应用配置表)
 std::string name = "Barbarian";
        if (troopId == 1002) name = "Archer";

        CCLOG("BattleScene: Spawning %s at grid(%d, %d)", name.c_str(), gx, gy);
        auto unit = troopLayer->spawnUnit(name, gx, gy);
 if (unit) {
            troopLayer->startUnitAI(unit); // 【这里直接调用AI】

      // 5. 自动切换到战斗状态
            if (_currentState == BattleState::PREPARE) {
     CCLOG("BattleScene: Switching from PREPARE to FIGHTING");
        switchState(BattleState::FIGHTING);
          }
            
         // 6. 下兵成功后，保持兵种选择状态（用户可以继续下兵）
    // 如果想要自动取消选择，可以调用：
         // _hudLayer->clearTroopSelection();
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