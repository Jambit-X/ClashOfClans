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
  void updatePendingResourceDisplay(int pendingGold, int pendingElixir);

  void showBuildingActions(int buildingId);
  void hideBuildingActions();
  void updateActionButtons(int buildingId);

  virtual void update(float dt) override;

  // 建筑UI放置方法
  void startBuildingPlacement(int buildingId);  // 从外部开始放置流程
  void showPlacementUI(int buildingId);
  void hidePlacementUI();
  void updatePlacementUIState(bool canPlace);

private:
  void initActionMenu();
  void initCollectButtons();

  cocos2d::Label* _goldLabel;
  cocos2d::Label* _elixirLabel;
  cocos2d::Label* _pendingGoldLabel;
  cocos2d::Label* _pendingElixirLabel;
  cocos2d::ui::Button* _collectGoldBtn;
  cocos2d::ui::Button* _collectElixirBtn;

  cocos2d::Node* _actionMenuNode;
  cocos2d::Label* _buildingNameLabel;
  cocos2d::Label* _upgradeCostLabel;
  cocos2d::ui::Button* _btnInfo;
  cocos2d::ui::Button* _btnUpgrade;
  cocos2d::ui::Button* _btnTrain;

  int _currentSelectedBuildingId = -1;

  PlacementConfirmUI* _placementUI;
  BuildingPlacementController* _placementController;

  // 放置模式的触摸监听器
  cocos2d::EventListenerTouchOneByOne* _placementTouchListener;
};