// TrainingLayer.h
// 训练营层声明，处理兵种训练和军队编辑功能

#ifndef __TRAINING_LAYER_H__
#define __TRAINING_LAYER_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <map>

struct TroopInfo;  // 前向声明

class TrainingLayer : public cocos2d::Layer {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(TrainingLayer);

private:
    // UI初始化辅助函数
    void initBackground();

    // 创建功能区块（生成"军队"条和"法术"条）
    void createSection(const std::string& title,
        const std::string& capacityStr,
        const std::string& subText,
        const cocos2d::Color4B& color,
        float posY,
        const std::function<void()>& callback);

    // 初始化底部的兵种选择面板
    void initTroopSelectionPanel();

    // 创建一个兵种卡片
    cocos2d::ui::Widget* createTroopCard(const TroopInfo& info, bool isUnlocked);

    // 交互逻辑
    void onCloseClicked();
    void onTroopsSectionClicked();
    void onSpellsSectionClicked();

    // 点击兵种卡片（训练）
    void onTroopCardClicked(int troopId);

    // 点击i按钮（显示详情）
    void onInfoClicked(int troopId);

    // 辅助功能
    void updateCapacityLabel();
    int getMaxBarracksLevel();

    // 初始化顶部军队展示面板
    void initTopArmyPanel();

    // 刷新顶部军队视图
    void updateArmyView();

    // 移除兵种逻辑
    void removeTroop(int troopId);

    // 创建上方的一个兵种小图标
    cocos2d::ui::Widget* createArmyUnitCell(int troopId, int count);

    // 设置长按连点功能
    void setupLongPressAction(cocos2d::ui::Widget* target, std::function<void()> callback, const std::string& scheduleKey);

    // 成员变量
    cocos2d::Node* _bgNode;
    cocos2d::ui::ScrollView* _scrollView;
    cocos2d::Label* _capacityLabel;

    std::map<int, int> _trainedTroops;  // 已训练的军队<ID, 数量>
    int _currentSpaceOccupied;
    const int MAX_SPACE = 40;

    cocos2d::ui::ScrollView* _armyScrollView;  // 顶部滚动容器

    std::string _activeLongPressKey;  // 当前正在长按的定时器key
};

#endif // __TRAINING_LAYER_H__
