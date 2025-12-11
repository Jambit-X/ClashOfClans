#pragma once
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <functional>

// 放置确认UI - 显示勾叉按钮
class PlacementConfirmUI : public cocos2d::Node {
public:
  static PlacementConfirmUI* create();
  virtual bool init() override;

  // 设置回调
  using ConfirmCallback = std::function<void()>;
  using CancelCallback = std::function<void()>;

  void setConfirmCallback(ConfirmCallback callback);
  void setCancelCallback(CancelCallback callback);

  // 显示/隐藏
  void show();
  void hide();

  // 更新按钮状态（根据是否可放置）
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