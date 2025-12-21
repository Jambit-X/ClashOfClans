#pragma once
#ifndef __BATTLE_HUD_LAYER_H__
#define __BATTLE_HUD_LAYER_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "../Scene/BattleScene.h"

class BattleHUDLayer : public cocos2d::Layer {
public:
    virtual bool init() override;
    CREATE_FUNC(BattleHUDLayer);

    // 根据战斗阶段更新 UI
    void updatePhase(BattleScene::BattleState state);

    // 更新倒计时文字
    void updateTimer(int seconds);

    // 【新增】设置按钮交互状态 (用于云层动画时禁用点击)
    void setButtonsEnabled(bool enabled);

    int getSelectedTroopId() const { return _selectedTroopId; }
    
    // 【新增】清除兵种选择状态
    void clearTroopSelection();

    // 【新增】更新指定兵种的数量显示
    void updateTroopCount(int troopId, int newCount);

    // 【新增】资源显示接口
    void initLootDisplay(int totalGold, int totalElixir);
    void updateLootDisplay(int lootedGold, int lootedElixir, int totalGold, int totalElixir);
    void hideReplayControls();  // 隐藏回放模式下的UI控件
private:
    // UI 元素
    cocos2d::Label* _timerLabel;
    cocos2d::ui::Button* _btnNext;     // 寻找对手
    cocos2d::ui::Button* _btnEnd;      // 红色结束战斗 (back.png)
    cocos2d::ui::Button* _btnReturn;   // 绿色回营 (finishbattle.png)
    cocos2d::Node* _troopBarNode;      // 底部兵种条容器

    // 【新增】资源显示UI
    cocos2d::Sprite* _goldIcon = nullptr;
    cocos2d::Sprite* _elixirIcon = nullptr;
    cocos2d::Label* _goldLabel = nullptr;
    cocos2d::Label* _elixirLabel = nullptr;

    int _selectedTroopId = -1; // 当前选中的兵种ID
    std::map<int, cocos2d::ui::Button*> _troopButtons; // 存储按钮以便控制高亮
    std::map<int, cocos2d::Label*> _troopCountLabels;  // 【新增】存储数量标签以便更新
    void onTroopSelected(int troopId);

    void initTopInfo();
    void initBottomButtons();
    void initTroopBar();

    // 辅助获取 Scene
    BattleScene* getBattleScene();
};

#endif // __BATTLE_HUD_LAYER_H__
