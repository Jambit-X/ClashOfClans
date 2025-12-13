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

private:
    // UI 元素
    cocos2d::Label* _timerLabel;
    cocos2d::ui::Button* _btnNext;     // 寻找对手
    cocos2d::ui::Button* _btnEnd;      // 红色结束战斗 (back.png)
    cocos2d::ui::Button* _btnReturn;   // 绿色回营 (finishbattle.png)
    cocos2d::Node* _troopBarNode;      // 底部兵种条容器

    int _selectedTroopId = -1; // 当前选中的兵种ID
    std::map<int, cocos2d::ui::Button*> _troopButtons; // 存储按钮以便控制高亮
    void onTroopSelected(int troopId);

    void initTopInfo();
    void initBottomButtons();
    void initTroopBar();

    // 辅助获取 Scene
    BattleScene* getBattleScene();
};

#endif // __BATTLE_HUD_LAYER_H__