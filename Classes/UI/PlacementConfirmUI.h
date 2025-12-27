// PlacementConfirmUI.h
// 建筑放置确认UI类，显示确认和取消按钮

#pragma once
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <functional>

class PlacementConfirmUI : public cocos2d::Node {
public:
  static PlacementConfirmUI* create();
  virtual bool init() override;

  using ConfirmCallback = std::function<void()>;
  using CancelCallback = std::function<void()>;

  void setConfirmCallback(ConfirmCallback callback);
  void setCancelCallback(CancelCallback callback);

  // 显示/隐藏UI
  void show();
  void hide();

  // 根据是否可放置更新按钮状态
  void updateButtonState(bool canPlace);

private:
  void initButtons();
  void onConfirmClicked();
  void onCancelClicked();

  cocos2d::ui::Button* _confirmBtn;
  cocos2d::ui::Button* _cancelBtn;

  ConfirmCallback _confirmCallback;
  CancelCallback _cancelCallback;

  bool _canPlace;
};
