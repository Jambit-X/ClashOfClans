#pragma once
#include "cocos2d.h"
#include "../Model/VillageData.h"

class ConstructionAnimation;

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
  void clearPlacementPreview();  // 新增方法

  // Getter
  int getBuildingId() const { return _buildingId; }
  int getBuildingType() const { return _buildingType; }
  int getBuildingLevel() const { return _buildingLevel; }
  BuildingInstance::State getBuildingState() const { return _buildingState; }
  cocos2d::Vec2 getVisualOffset() const { return _visualOffset; }

private:
  void loadSprite(int type, int level);
  void updateVisuals();

  int _buildingId;
  int _buildingType;
  int _buildingLevel;
  BuildingInstance::State _buildingState;
  cocos2d::Vec2 _visualOffset;
  cocos2d::Vec2 _gridPos;

  cocos2d::ProgressTimer* _progressBar;
  cocos2d::Label* _countdownLabel;
  ConstructionAnimation* _constructionAnim;
};