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
    // ========== 核心 UI 组件 ==========
    cocos2d::Node* _bgNode;                     // 主面板容器
    cocos2d::ui::ScrollView* _scrollView;       // 兵种卡片滚动容器
    cocos2d::ui::Button* _instantFinishBtn;     // 立即完成按钮
    
    // ========== 初始化方法 ==========
    void initBackground();                       // 初始化背景图
    void initTroopCards();                       // 初始化兵种卡片列表

    // ========== 卡片创建方法 ==========
    // 卡片状态枚举
    enum class CardState {
        NORMAL,         // 可升级
        MAX_LEVEL,      // 已满级
        LAB_LOCKED,     // 实验室等级不足
        TROOP_LOCKED,   // 兵种未解锁（训练营等级不足）
        RESEARCHING,    // 正在研究此兵种
        RESEARCH_BUSY   // 其他兵种正在研究
    };

    cocos2d::ui::Widget* createTroopCard(const TroopInfo& info, CardState state, int currentLevel);
    CardState getCardState(int troopId, int currentLevel) const;

    // ========== 交互方法 ==========
    void onTroopCardClicked(int troopId);        // 点击兵种卡片
    void onCloseClicked();                       // 点击关闭按钮
    void showUpgradePopup(int troopId);          // 显示升级确认弹窗
    void onUpgradeConfirmed(int troopId);        // 确认升级
    void onInstantFinishClicked();               // 点击立即完成
    void showToast(const std::string& message);  // 显示提示消息

    // ========== 更新方法 ==========
    void update(float dt) override;               // 更新倒计时
    void refreshCards();                          // 刷新卡片列表

    // ========== 辅助方法 ==========
    int getMaxBarracksLevel() const;             // 获取最高训练营等级
    std::string formatTime(int seconds) const;   // 格式化时间显示
    int getInstantFinishCost() const;            // 获取立即完成所需钻石
    void updateInstantFinishButton();            // 更新立即完成按钮状态
};

#endif // __LABORATORY_LAYER_H__
