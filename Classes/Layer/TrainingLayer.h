#ifndef __TRAINING_LAYER_H__
#define __TRAINING_LAYER_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <map>

// 前向声明
struct TroopInfo;

class TrainingLayer : public cocos2d::Layer {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(TrainingLayer);

private:
    // --- UI 初始化辅助函数 ---

    // 创建通用的背景板
    void initBackground();

    // 创建一个功能区块（用于生成“军队”条和“法术”条）
    void createSection(const std::string& title,
        const std::string& capacityStr,
        const std::string& subText,
        const cocos2d::Color4B& color,
        float posY,
        const std::function<void()>& callback);

    // 【新增】初始化底部的兵种选择面板
    void initTroopSelectionPanel();

    // 【新增】创建一个兵种卡片
    cocos2d::ui::Widget* createTroopCard(const TroopInfo& info, bool isUnlocked);

    // --- 交互逻辑 ---
    void onCloseClicked();
    void onTroopsSectionClicked();
    void onSpellsSectionClicked();

    // 【新增】点击兵种卡片（训练）
    void onTroopCardClicked(int troopId);

    // 【新增】点击 i 按钮（显示详情）
    void onInfoClicked(int troopId);

    // 【新增】辅助功能
    void updateCapacityLabel(); // 刷新 "0/40"
    int getMaxBarracksLevel();  // 获取当前最高训练营等级

    // 【新增】初始化顶部军队展示面板
    void initTopArmyPanel();

    // 【新增】刷新顶部军队视图 (核心逻辑：负责堆叠和显示)
    void updateArmyView();

    // 【新增】移除兵种逻辑
    void removeTroop(int troopId);

    // 【新增】创建上方的一个兵种小图标 (Cell)
    cocos2d::ui::Widget* createArmyUnitCell(int troopId, int count);

    // 【新增】设置长按连点功能的辅助函数
    void setupLongPressAction(cocos2d::ui::Widget* target, std::function<void()> callback, const std::string& scheduleKey);

    // --- 成员变量 ---
    cocos2d::Node* _bgNode;             // 主背景容器
    cocos2d::ui::ScrollView* _scrollView; // 底部兵种滚动容器
    cocos2d::Label* _capacityLabel;       // 顶部的 "0/40" 标签引用

    // 数据存储：已训练的军队 <ID, 数量>
    std::map<int, int> _trainedTroops;
    int _currentSpaceOccupied;          // 当前占用人口
    const int MAX_SPACE = 40;           // 固定最大人口 40

    // 【新增】顶部滚动容器
    cocos2d::ui::ScrollView* _armyScrollView;
};

#endif // __TRAINING_LAYER_H__