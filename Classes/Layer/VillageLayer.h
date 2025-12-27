// VillageLayer.h
// 村庄层声明，管理村庄场景的地图、建筑和输入控制

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

    // 地图主题切换
    void switchMapBackground(int themeId);
    
    // 获取当前选中的建筑ID
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

    cocos2d::ParticleSystemQuad* _currentParticleEffect = nullptr;  // 粒子效果管理
};
