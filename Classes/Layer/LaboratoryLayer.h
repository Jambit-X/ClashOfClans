// LaboratoryLayer.h
// 实验室层声明，处理兵种升级研究功能

#pragma once
#ifndef __LABORATORY_LAYER_H__
#define __LABORATORY_LAYER_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Model/TroopConfig.h"

class LaboratoryLayer : public cocos2d::Layer {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(LaboratoryLayer);

private:
    // 核心UI组件
    cocos2d::Node* _bgNode;                     // 主面板容器
    cocos2d::ui::ScrollView* _scrollView;       // 兵种卡片滚动容器
    cocos2d::ui::Button* _instantFinishBtn;     // 立即完成按钮
    
    // 初始化方法
    void initBackground();
    void initTroopCards();

    // 卡片状态枚举
    enum class CardState {
        NORMAL,         // 可升级
        MAX_LEVEL,      // 已满级
        LAB_LOCKED,     // 实验室等级不足
        TROOP_LOCKED,   // 兵种未解锁
        RESEARCHING,    // 正在研究此兵种
        RESEARCH_BUSY   // 其他兵种正在研究
    };

    cocos2d::ui::Widget* createTroopCard(const TroopInfo& info, CardState state, int currentLevel);
    CardState getCardState(int troopId, int currentLevel) const;

    // 交互方法
    void onTroopCardClicked(int troopId);
    void onCloseClicked();
    void showUpgradePopup(int troopId);
    void onUpgradeConfirmed(int troopId);
    void onInstantFinishClicked();
    void showToast(const std::string& message);

    // 更新方法
    void update(float dt) override;
    void refreshCards();

    // 辅助方法
    int getMaxBarracksLevel() const;
    std::string formatTime(int seconds) const;
    int getInstantFinishCost() const;
    void updateInstantFinishButton();
};

#endif // __LABORATORY_LAYER_H__
