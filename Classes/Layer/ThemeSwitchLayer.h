// ThemeSwitchLayer.h
// 主题切换层声明，处理地图主题的选择和购买

#pragma once
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <map>

// 场景数据结构
struct MapTheme {
    int id;
    std::string name;
    std::string previewImage;
    std::string mapImage;
    int unlockTownHallLevel;
    int gemCost;
    bool isPurchased;
    bool hasParticleEffect;
};

class ThemeSwitchLayer : public cocos2d::Layer {
public:
    virtual bool init();
    CREATE_FUNC(ThemeSwitchLayer);

private:
    void initBackground();
    void initScrollView();
    void initBottomButton();
    void loadThemeData();

    void createThemeCard(const MapTheme& theme, float startX);
    void onThemeSelected(int themeId);
    void updateBottomButton();

    void onPurchaseClicked();
    void onConfirmClicked();
    void onCloseClicked();

private:
    cocos2d::Node* _panelNode;
    cocos2d::ui::ScrollView* _scrollView;
    cocos2d::ui::Button* _bottomButton;

    std::map<int, MapTheme> _themes;
    int _currentSelectedTheme;
    std::map<int, cocos2d::Sprite*> _themeCards;

    const std::string FONT_PATH = "fonts/simhei.ttf";
};
