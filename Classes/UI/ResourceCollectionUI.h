#pragma once
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <memory>  // ? 添加 shared_ptr 支持

// 资源收集UI组件 - 负责显示和收集按钮
class ResourceCollectionUI : public cocos2d::Node {
public:
  static ResourceCollectionUI* create();
  virtual bool init() override;
  virtual void onExit() override;

  void updateDisplay(int pendingGold, int pendingElixir);

private:
  void initButtons();
  void onCollectGold();
  void onCollectElixir();

  cocos2d::ui::Button* _collectGoldBtn;
  cocos2d::ui::Button* _collectElixirBtn;
  cocos2d::Label* _pendingGoldLabel;
  cocos2d::Label* _pendingElixirLabel;
  
  // ? 添加：用于跟踪对象是否有效的标志
  std::shared_ptr<bool> _isValid;
};