#pragma once
#include "cocos2d.h"
#include "../Model/VillageData.h"

//  不再需要 ConstructionAnimation 类
// class ConstructionAnimation;

class BuildingSprite : public cocos2d::Sprite {
public:
  static BuildingSprite* create(const BuildingInstance& building);
  virtual bool init(const BuildingInstance& building);

  void updateBuilding(const BuildingInstance& building);
  void updateLevel(int level);
  void updateState(BuildingInstance::State state);

  void startConstruction();
  void updateConstructionProgress(float progress);
  void finishConstruction();

  void showConstructionProgress(float progress);
  void hideConstructionProgress();
  void showCountdown(int seconds);

  // 网格相关方法
  cocos2d::Size getGridSize() const;
  cocos2d::Vec2 getGridPos() const { return _gridPos; }
  void setGridPos(const cocos2d::Vec2& pos) { _gridPos = pos; }

  // 拖动和放置预览
  void setDraggingMode(bool isDragging);
  void setPlacementPreview(bool isValid);
  void clearPlacementPreview();

  // 选中效果
  void showSelectionEffect();   // 显示选中效果（弹跳 + 光圈）
  void hideSelectionEffect();   // 隐藏选中效果
  bool isSelected() const { return _isSelected; }

  // Getter
  int getBuildingId() const { return _buildingId; }
  int getBuildingType() const { return _buildingType; }
  int getBuildingLevel() const { return _buildingLevel; }
  BuildingInstance::State getBuildingState() const { return _buildingState; }
  cocos2d::Vec2 getVisualOffset() const { return _visualOffset; }

private:
  void loadSprite(int type, int level);
  void updateVisuals();

  //  新增：UI 管理方法
  void initConstructionUI();    // 创建 UI 容器（只调用一次）
  void showConstructionUI();    // 显示建造 UI
  void hideConstructionUI();    // 隐藏建造 UI

  // 选中效果辅助方法
  void createSelectionGlow();   // 创建光圈

  int _buildingId;
  int _buildingType;
  int _buildingLevel;
  BuildingInstance::State _buildingState;
  cocos2d::Vec2 _visualOffset;
  cocos2d::Vec2 _gridPos;

  //  新的 UI 容器结构
  cocos2d::Node* _constructionUIContainer;  // UI 容器

  //  子元素指针（便于访问）
  cocos2d::Sprite* _progressBg;            // 进度条背景
  cocos2d::ProgressTimer* _progressBar;     // 进度条
  cocos2d::Label* _countdownLabel;          // 倒计时
  cocos2d::Label* _percentLabel;            // 百分比标签

  // 选中效果
  cocos2d::DrawNode* _selectionGlow;        // 底部光圈
  bool _isSelected;                          // 选中状态
};
