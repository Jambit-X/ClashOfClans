#pragma once
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "UI/PlacementConfirmUI.h"
#include "Controller/BuildingPlacementController.h"

class HUDLayer : public cocos2d::Layer {
public:
    virtual bool init();
    virtual void cleanup() override;
    CREATE_FUNC(HUDLayer);

    void updateResourceDisplay(int gold, int elixir);

    void showBuildingActions(int buildingId);
    void hideBuildingActions();
    void updateActionButtons(int buildingId);

    virtual void update(float dt) override;

    // 放置UI相关方法
    void startBuildingPlacement(int buildingId);
    void showPlacementUI(int buildingId);
    void hidePlacementUI();
    void updatePlacementUIState(bool canPlace);

    // ========== 连续建造模式 ==========
    void enterContinuousBuildMode(int buildingType);
    void exitContinuousBuildMode(const std::string& reason);
    bool isInContinuousBuildMode() const { return _isContinuousBuildMode; }

private:
    void initActionMenu();

    // 按钮布局配置
    struct ButtonLayout {
        cocos2d::Vec2 infoPos;
        cocos2d::Vec2 upgradePos;
        cocos2d::Vec2 trainPos;
    };

    static const ButtonLayout LAYOUT_ONE_BUTTON;
    static const ButtonLayout LAYOUT_TWO_BUTTONS;
    static const ButtonLayout LAYOUT_THREE_BUTTONS;

    // 提示消息复用
    void showTips(const std::string& text, const cocos2d::Color3B& color);
    void hideTips();

    // 加速按钮回调
    void onSpeedupClicked(int buildingId);

    // ========== 新增：场景切换按钮回调 ==========
    void onThemeSwitchClicked();

    cocos2d::Label* _goldLabel;
    cocos2d::Label* _elixirLabel;
    cocos2d::Label* _gemLabel;
    cocos2d::Label* _workerLabel;

    void updateWorkerDisplay();

    cocos2d::Node* _actionMenuNode;
    cocos2d::Label* _buildingNameLabel;
    cocos2d::Label* _upgradeCostLabel;
    cocos2d::ui::Button* _btnInfo;
    cocos2d::ui::Button* _btnUpgrade;
    cocos2d::ui::Button* _btnTrain;
    cocos2d::ui::Button* _btnSpeedup;
    cocos2d::ui::Button* _btnResearch;
    cocos2d::ui::Button* _btnThemeSwitch;  // ✅ 新增：切换场景按钮

    int _currentSelectedBuildingId = -1;

    PlacementConfirmUI* _placementUI;
    BuildingPlacementController* _placementController;

    cocos2d::EventListenerTouchOneByOne* _placementTouchListener;

    // 复用的提示Label
    cocos2d::Label* _tipsLabel;

    // 连续建造的辅助方法
    void createNextWall();
    bool canContinueBuild();
    void updateContinuousModeUI();
    void setupKeyboardListener();
    void onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

    // 连续建造模式的状态变量
    bool _isContinuousBuildMode = false;
    int _continuousBuildingType = -1;
    cocos2d::Label* _modeHintLabel = nullptr;
    cocos2d::EventListenerKeyboard* _keyboardListener = nullptr;
};
