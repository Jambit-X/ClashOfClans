#pragma once
#include "cocos2d.h"
#include "ui/CocosGUI.h"

/**
 * @brief 调试面板层
 * 提供可视化界面修改游戏数据
 */
class DebugLayer : public cocos2d::Layer {
public:
    virtual bool init() override;
    CREATE_FUNC(DebugLayer);

private:
    // UI 初始化
    void initBackground();
    void initPanel();
    void initResourceSection();
    void initBuildingSection();
    void initSaveSection();

    // 关闭面板
    void close();

    // 刷新建筑列表
    void refreshBuildingList();

    // 资源回调
    void onGoldChanged(int delta);
    void onElixirChanged(int delta);
    void onGemChanged(int delta);

    // 建筑回调
    void onBuildingSelected(int buildingId);
    void onSetBuildingLevel(int level);
    void onDeleteBuilding();
    void onCompleteAllConstructions();

    // 存档回调
    void onResetSave();
    void onForceSave();

    // UI 成员
    cocos2d::Node* _panel;
    cocos2d::Label* _goldValueLabel;
    cocos2d::Label* _elixirValueLabel;
    cocos2d::Label* _gemValueLabel;
    
    // 当前选中的建筑
    int _selectedBuildingId = -1;
    cocos2d::Label* _selectedBuildingLabel;

    // 建筑列表滚动视图
    cocos2d::ui::ScrollView* _buildingScrollView;
};
