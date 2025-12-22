#pragma once
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <map>

// 场景数据结构
struct MapTheme {
    int id;                          // 场景ID
    std::string name;                // 场景名称（中文）
    std::string previewImage;        // 预览图路径
    std::string mapImage;            // 地图背景图路径
    int unlockTownHallLevel;         // 解锁所需大本营等级
    int gemCost;                     // 宝石价格（0表示免费/初始拥有）
    bool isPurchased;                // 是否已购买
    bool hasParticleEffect;          // 是否有粒子效果
};

class ThemeSwitchLayer : public cocos2d::Layer {
public:
    virtual bool init();
    CREATE_FUNC(ThemeSwitchLayer);

private:
    void initBackground();           // 初始化背景面板
    void initScrollView();           // 初始化可滚动的场景列表
    void initBottomButton();         // 初始化底部按钮区域
    void loadThemeData();            // 加载场景数据

    void createThemeCard(const MapTheme& theme, float startX);  // 创建场景卡片
    void onThemeSelected(int themeId);  // 选中场景回调
    void updateBottomButton();       // 更新底部按钮状态

    void onPurchaseClicked();        // 购买场景回调
    void onConfirmClicked();         // 确认切换场景回调
    void onCloseClicked();           // 关闭面板回调

private:
    cocos2d::Node* _panelNode;                          // 主面板容器
    cocos2d::ui::ScrollView* _scrollView;               // 场景列表滚动容器
    cocos2d::ui::Button* _bottomButton;                 // 底部按钮（购买/确认/锁定）

    std::map<int, MapTheme> _themes;                    // 所有场景数据
    int _currentSelectedTheme;                          // 当前选中的场景ID
    std::map<int, cocos2d::Sprite*> _themeCards;        // 场景卡片精灵（用于更新边框）

    const std::string FONT_PATH = "fonts/simhei.ttf";
};
