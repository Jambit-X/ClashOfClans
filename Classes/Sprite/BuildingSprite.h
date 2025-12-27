// BuildingSprite.h
// 建筑精灵，支持建造动画、血条显示和选中效果

#pragma once
#include "cocos2d.h"
#include "../Model/VillageData.h"
#include "Component/HealthBarComponent.h"

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
  void showSelectionEffect();
  void hideSelectionEffect();
  bool isSelected() const { return _isSelected; }

  // 摧毁效果（战斗场景用）
  void showDestroyedRubble();
  bool isShowingRubble() const { return _isShowingRubble; }

  // 目标标记（战斗场景用）
  void showTargetBeacon();

  // 控制主贴图可见性（用于防御建筑动画切换）
  void setMainTextureVisible(bool visible);

  // Getter
  int getBuildingId() const { return _buildingId; }
  int getBuildingType() const { return _buildingType; }
  int getBuildingLevel() const { return _buildingLevel; }
  BuildingInstance::State getBuildingState() const { return _buildingState; }
  cocos2d::Vec2 getVisualOffset() const { return _visualOffset; }

  void loadSprite(int type, int level);
  void updateHealthBar(int currentHP, int maxHP);

private:
  void updateVisuals();

  // UI管理方法
  void initConstructionUI();
  void showConstructionUI();
  void hideConstructionUI();

  void createSelectionGlow();

  int _buildingId;
  int _buildingType;
  int _buildingLevel;
  BuildingInstance::State _buildingState;
  cocos2d::Vec2 _visualOffset;
  cocos2d::Vec2 _gridPos;

  // UI容器结构
  cocos2d::Node* _constructionUIContainer;

  // 子元素指针
  cocos2d::Sprite* _progressBg;
  cocos2d::ProgressTimer* _progressBar;
  cocos2d::Label* _countdownLabel;
  cocos2d::Label* _percentLabel;

  // 选中效果
  cocos2d::DrawNode* _selectionGlow;
  bool _isSelected;

  // 血条组件
  HealthBarComponent* _healthBar = nullptr;

  // 摧毁状态
  bool _isShowingRubble = false;

  // 目标标记
  cocos2d::Sprite* _targetBeacon = nullptr;
};
