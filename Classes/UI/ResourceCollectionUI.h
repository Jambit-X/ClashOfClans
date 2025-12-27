// ResourceCollectionUI.h
// 资源收集UI类，管理金币和圣水的待收集资源显示与收集

#pragma once
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <memory>

class ResourceCollectionUI : public cocos2d::Node {
public:
  static ResourceCollectionUI* create();
  virtual bool init() override;
  virtual void onExit() override;

  // 更新待收集资源显示
  void updateDisplay(int pendingGold, int pendingElixir);

private:
  void initButtons();
  void onCollectGold();
  void onCollectElixir();

  cocos2d::ui::Button* _collectGoldBtn;
  cocos2d::ui::Button* _collectElixirBtn;
  cocos2d::Label* _pendingGoldLabel;
  cocos2d::Label* _pendingElixirLabel;
  
  // 用于跟踪对象是否有效的标志
  std::shared_ptr<bool> _isValid;
};
