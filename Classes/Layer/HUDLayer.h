#pragma once
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "UI/PlacementConfirmUI.h"
#include "Controller/BuildingPlacementController.h"

class HUDLayer : public cocos2d::Layer {
public:
  virtual bool init();
  virtual void cleanup() override;  // ? 添加 cleanup 方法
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

private:
  void initActionMenu();

  // 按钮布局配置
  struct ButtonLayout {
    cocos2d::Vec2 infoPos;
    cocos2d::Vec2 upgradePos;
    cocos2d::Vec2 trainPos;
  };

  static const ButtonLayout LAYOUT_TWO_BUTTONS;
  static const ButtonLayout LAYOUT_THREE_BUTTONS;

  // 提示消息复用
  void showTips(const std::string& text, const cocos2d::Color3B& color);
  void hideTips();

  // 加速按钮回调
  void onSpeedupClicked(int buildingId);

  cocos2d::Label* _goldLabel;
  cocos2d::Label* _elixirLabel;
  cocos2d::Label* _gemLabel;

  cocos2d::Node* _actionMenuNode;
  cocos2d::Label* _buildingNameLabel;
  cocos2d::Label* _upgradeCostLabel;
  cocos2d::ui::Button* _btnInfo;
  cocos2d::ui::Button* _btnUpgrade;
  cocos2d::ui::Button* _btnTrain;
  cocos2d::ui::Button* _btnSpeedup;  // 加速按钮

  int _currentSelectedBuildingId = -1;

  PlacementConfirmUI* _placementUI;
  BuildingPlacementController* _placementController;

  cocos2d::EventListenerTouchOneByOne* _placementTouchListener;

  // 复用的提示Label
  cocos2d::Label* _tipsLabel;
};