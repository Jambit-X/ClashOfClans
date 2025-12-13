#ifndef __BATTLE_SCENE_H__
#define __BATTLE_SCENE_H__

#include "cocos2d.h"
#include <functional> // 需要包含这个头文件

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
    virtual void onExit() override; // 【新增】场景退出时清理资源
    CREATE_FUNC(BattleScene);

    void switchState(BattleState newState);
    BattleState getCurrentState() const { return _currentState; }

    // --- 外部交互回调 ---
    void onNextOpponentClicked(); // 点击“寻找对手”
    // void onStartBattleClicked(); // [已删除]
    void onEndBattleClicked();    // 点击“结束战斗”
    void onReturnHomeClicked();   // 点击“回营”

private:
    BattleState _currentState = BattleState::PREPARE;
    float _stateTimer = 0.0f;

    BattleMapLayer* _mapLayer = nullptr;
    BattleHUDLayer* _hudLayer = nullptr;
    BattleResultLayer* _resultLayer = nullptr;

    //监听点击,生成兵种,并触发 AI。
    void setupTouchListener();
    void cleanupTouchListener(); // 【新增】清理触摸监听器
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);
    
    // 触摸状态跟踪
    cocos2d::Vec2 _touchStartPos;
    bool _isTouchMoving;
    cocos2d::EventListenerTouchOneByOne* _touchListener = nullptr; // 【新增】保存监听器指针
    
    // 辅助方法：判断触摸是否在 UI 区域
    bool isTouchOnUI(const cocos2d::Vec2& touchPos);

    // --- 云层遮罩相关 ---
    cocos2d::Sprite* _cloudSprite = nullptr;
    bool _isSearching = false; // 防止重复点击标志位

    void loadEnemyVillage();

    /**
     * @brief 执行云层搜索动画
     * @param onMapReloadCallback 当云层完全遮住屏幕时，执行的回调（通常用于刷新地图）
     * @param isInitialEntry 是否是刚进入场景（如果是，则直接显示云层并只执行淡出）
     */
    void performCloudTransition(std::function<void()> onMapReloadCallback, bool isInitialEntry = false);
};

#endif // __BATTLE_SCENE_H__