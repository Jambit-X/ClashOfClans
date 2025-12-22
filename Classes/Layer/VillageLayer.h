#pragma once
#include "cocos2d.h"
#include "../Model/VillageData.h"

class BuildingManager;
class MoveMapController;
class MoveBuildingController;
class BuildingSprite;

class VillageLayer : public cocos2d::Layer {
public:
    virtual bool init();
    virtual void cleanup() override;
    CREATE_FUNC(VillageLayer);

    void onBuildingPurchased(int buildingId);
    void removeBuildingSprite(int buildingId);
    void updateBuildingDisplay(int buildingId);
    void clearSelectedBuilding();
    void updateBuildingPreviewPosition(int buildingId, const cocos2d::Vec2& worldPos);

    // ========== 修改：将地图切换改为 public ==========
    void switchMapBackground(int themeId);  // ✅ 移到 public 区域
    // ========== 新增：获取当前选中的建筑ID ==========
    int getSelectedBuildingId() const;

private:
    cocos2d::Sprite* createMapSprite();
    void initializeBasicProperties();
    void setupInputCallbacks();
    BuildingSprite* getBuildingAtScreenPos(const cocos2d::Vec2& screenPos);

private:
    cocos2d::Sprite* _mapSprite;
    BuildingManager* _buildingManager;
    MoveMapController* _inputController;
    MoveBuildingController* _moveBuildingController;
    BuildingSprite* _currentSelectedBuilding = nullptr;

    // 粒子效果管理
    cocos2d::ParticleSystemQuad* _currentParticleEffect = nullptr;
};
