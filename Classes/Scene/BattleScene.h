#ifndef __BATTLE_SCENE_H__
#define __BATTLE_SCENE_H__

#include "cocos2d.h"
#include <functional>
#include <map>

// 前置声明
namespace cocos2d {
    class Touch;
    class Event;
}

class BattleMapLayer;
class BattleHUDLayer;
class BattleResultLayer;

class BattleScene : public cocos2d::Scene {
public:
    enum class BattleState {
        PREPARE,    // 侦查阶段
        FIGHTING,   // 战斗阶段
        RESULT      // 结算阶段
    };

    static cocos2d::Scene* createScene();
    virtual bool init();
    virtual void update(float dt);
    virtual void onExit() override;
    CREATE_FUNC(BattleScene);

    void switchState(BattleState newState);
    BattleState getCurrentState() const { return _currentState; }

    // --- 外部交互回调 ---
    void onNextOpponentClicked();
    void onEndBattleClicked();
    void onReturnHomeClicked();

    // ========== 兵种追踪系统 ==========
    int getRemainingTroopCount(int troopId) const;
    const std::map<int, int>& getUsedTroops() const { return _usedTroops; }
    const std::map<int, int>& getTroopLevels() const { return _troopLevels; }

    // ========== 资源掠夺系统 ==========
    void addLootedGold(int amount);
    void addLootedElixir(int amount);
    int getLootedGold() const { return _lootedGold; }
    int getLootedElixir() const { return _lootedElixir; }
    int getTotalLootableGold() const { return _totalLootableGold; }
    int getTotalLootableElixir() const { return _totalLootableElixir; }

private:
    BattleState _currentState = BattleState::PREPARE;
    float _stateTimer = 0.0f;

    BattleMapLayer* _mapLayer = nullptr;
    BattleHUDLayer* _hudLayer = nullptr;
    BattleResultLayer* _resultLayer = nullptr;

    // ========== 兵种追踪数据 ==========
    std::map<int, int> _remainingTroops;  // 剩余可用数量
    std::map<int, int> _usedTroops;       // 已消耗统计
    std::map<int, int> _troopLevels;      // 兵种等级缓存
    void initBattleTroops();              // 初始化战斗兵种数据

    // ========== 资源掠夺数据 ==========
    int _lootedGold = 0;           // 已掠夺金币
    int _lootedElixir = 0;         // 已掠夺圣水
    int _totalLootableGold = 0;    // 总可掠夺金币
    int _totalLootableElixir = 0;  // 总可掠夺圣水
    int _goldPerStorage = 0;       // 每个储金罐的资源
    int _elixirPerStorage = 0;     // 每个圣水瓶的资源
    cocos2d::EventListenerCustom* _buildingDestroyedListener = nullptr;
    void setupBuildingDestroyedListener();
    void onBuildingDestroyed(cocos2d::EventCustom* event);
    void checkAllBuildingsDestroyed();  // 检查是否所有建筑都已被摧毁

    // 监听点击,生成兵种,并触发 AI
    void setupTouchListener();
    void cleanupTouchListener();
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);
    
    // 触摸状态跟踪
    cocos2d::Vec2 _touchStartPos;
    bool _isTouchMoving;
    cocos2d::EventListenerTouchOneByOne* _touchListener = nullptr;
    
    // 辅助方法
    bool isTouchOnUI(const cocos2d::Vec2& touchPos);
    void showPlacementTip(const std::string& message, const cocos2d::Vec2& pos);

    // --- 云层遮罩相关 ---
    cocos2d::Sprite* _cloudSprite = nullptr;
    bool _isSearching = false;

    void loadEnemyVillage();
    void performCloudTransition(std::function<void()> onMapReloadCallback, bool isInitialEntry = false);
};

#endif // __BATTLE_SCENE_H__