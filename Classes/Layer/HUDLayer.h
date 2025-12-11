
#pragma once
#include "cocos2d.h"
#include "ui/CocosGUI.h"

class HUDLayer : public cocos2d::Layer {
public:
  virtual bool init();
  CREATE_FUNC(HUDLayer);

  void updateResourceDisplay(int gold, int elixir);
  
  // 新增：更新待收集资源显示
  void updatePendingResourceDisplay(int pendingGold, int pendingElixir);

  void showBuildingActions(int buildingId);
  void hideBuildingActions();
  void updateActionButtons(int buildingId);

  virtual void update(float dt) override;

private:
  void initActionMenu();
  
  // 新增：初始化收集按钮
  void initCollectButtons();

  // 资源标签
  cocos2d::Label* _goldLabel;
  cocos2d::Label* _elixirLabel;
  
  // 新增：待收集资源标签
  cocos2d::Label* _pendingGoldLabel;
  cocos2d::Label* _pendingElixirLabel;
  
  // 新增：收集按钮
  cocos2d::ui::Button* _collectGoldBtn;
  cocos2d::ui::Button* _collectElixirBtn;

  // 建筑操作菜单
  cocos2d::Node* _actionMenuNode;
  cocos2d::Label* _buildingNameLabel;
  cocos2d::Label* _upgradeCostLabel;
  cocos2d::ui::Button* _btnInfo;
  cocos2d::ui::Button* _btnUpgrade;
  cocos2d::ui::Button* _btnTrain;

  int _currentSelectedBuildingId = -1;
};