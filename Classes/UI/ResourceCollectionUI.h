#pragma once
#include "cocos2d.h"
#include "ui/CocosGUI.h"

// 资源收集UI组件 - 负责显示和收集按钮
class ResourceCollectionUI : public cocos2d::Node {
public:
  static ResourceCollectionUI* create();
  virtual bool init() override;

  void updateDisplay(int pendingGold, int pendingElixir);

private:
  void initButtons();
  void onCollectGold();
  void onCollectElixir();

  cocos2d::ui::Button* _collectGoldBtn;
  cocos2d::ui::Button* _collectElixirBtn;
  cocos2d::Label* _pendingGoldLabel;
  cocos2d::Label* _pendingElixirLabel;
};