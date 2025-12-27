// BattleMapLayer.cpp
// 战斗地图层实现，管理战斗场景的地图背景、建筑和输入控制

#include "BattleMapLayer.h"
#include "Manager/BuildingManager.h"
#include "Manager/VillageDataManager.h"
#include "Controller/MoveMapController.h"
#include "Util/GridMapUtils.h"

USING_NS_CC;

BattleMapLayer::~BattleMapLayer() {
    // 清理MoveMapController
    if (_inputController) {
        delete _inputController;
        _inputController = nullptr;
    }

    // 清理BuildingManager
    if (_buildingManager) {
        delete _buildingManager;
        _buildingManager = nullptr;
    }

    CCLOG("BattleMapLayer: Destroyed and cleaned up resources");
}

bool BattleMapLayer::init() {
    if (!Layer::init()) return false;

    // 创建地图背景
    _mapSprite = createMapSprite();
    this->addChild(_mapSprite);

    // 设置Layer大小与地图一致
    if (_mapSprite) {
        this->setContentSize(_mapSprite->getContentSize());
        this->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
    }

    // 初始化建筑管理器（战斗场景模式）
    _buildingManager = new BuildingManager(this, true);

    // 输出建筑布局
    logBuildingLayout("INITIAL LOAD");

    // 初始化地图移动控制器（仅拖拽缩放）
    _inputController = new MoveMapController(this);
    _inputController->setupInputListeners();

    // 启动定时更新
    this->scheduleUpdate();

    // 监听目标锁定事件
    auto listener = EventListenerCustom::create("EVENT_UNIT_TARGET_LOCKED", [this](EventCustom* event) {
        if (!_buildingManager) return;
        intptr_t rawId = reinterpret_cast<intptr_t>(event->getUserData());
        int targetID = static_cast<int>(rawId);

        BuildingSprite* b = _buildingManager->getBuildingSprite(targetID);
        if (b) {
            b->showTargetBeacon();
        }
    });
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}

void BattleMapLayer::reloadMap() {
    CCLOG("BattleMapLayer: Reloading map with new random data...");

    // 随机更换地图背景
    if (_mapSprite) {
        _mapSprite->removeFromParent();
        _mapSprite = nullptr;
    }
    _mapSprite = createMapSprite();
    if (_mapSprite) {
        this->addChild(_mapSprite, -1);
    }

    // 生成新的随机地图
    auto dataManager = VillageDataManager::getInstance();
    dataManager->generateRandomBattleMap(0);

    // 清理旧的BuildingManager
    if (_buildingManager) {
        delete _buildingManager;
        _buildingManager = nullptr;
    }

    // 创建新的BuildingManager
    _buildingManager = new BuildingManager(this, true);

    // 输出建筑布局
    logBuildingLayout("RELOAD MAP");

    CCLOG("BattleMapLayer: Map reloaded with %zu buildings",
          dataManager->getBattleMapData().buildings.size());
}

void BattleMapLayer::logBuildingLayout(const std::string& context) {
    auto dataManager = VillageDataManager::getInstance();
    const auto& buildings = dataManager->getAllBuildings();

    CCLOG("========================================");
    CCLOG("BATTLE BUILDING LAYOUT [%s]", context.c_str());
    CCLOG("========================================");
    CCLOG("Total Buildings: %zu", buildings.size());
    CCLOG("----------------------------------------");

    int cannonCount = 0;
    int archerTowerCount = 0;

    for (const auto& building : buildings) {
        // 跳过放置中的建筑
        if (building.state == BuildingInstance::State::PLACING) {
            continue;
        }

        // 计算像素坐标
        Vec2 pixelPos = GridMapUtils::gridToPixel(building.gridX, building.gridY);

        // 建筑类型名称
        std::string typeName;
        bool isDefense = false;

        if (building.type == 1) typeName = "TownHall";
        else if (building.type == 101) typeName = "ArmyCamp";
        else if (building.type == 102) typeName = "Barracks";
        else if (building.type == 103) typeName = "Laboratory";
        else if (building.type == 201) typeName = "BuildersHut";
        else if (building.type == 202) typeName = "GoldMine";
        else if (building.type == 203) typeName = "ElixirCollector";
        else if (building.type == 204) typeName = "GoldStorage";
        else if (building.type == 205) typeName = "ElixirStorage";
        else if (building.type == 301) {
            typeName = "Cannon";
            isDefense = true;
            cannonCount++;
        } else if (building.type == 302) {
            typeName = "ArcherTower";
            isDefense = true;
            archerTowerCount++;
        } else if (building.type == 303) typeName = "Wall";
        else if (building.type >= 401 && building.type <= 404) typeName = "Trap";
        else typeName = "Unknown";

        // 输出建筑信息
        if (isDefense) {
            CCLOG("🎯 ID=%d | %s Lv%d | Grid(%d,%d) | Pixel(%.0f,%.0f) | HP=%d | %s",
                  building.id,
                  typeName.c_str(),
                  building.level,
                  building.gridX,
                  building.gridY,
                  pixelPos.x,
                  pixelPos.y,
                  building.currentHP,
                  building.isDestroyed ? "DESTROYED" : "ACTIVE");
        } else {
            CCLOG("  ID=%d | %s Lv%d | Grid(%d,%d) | Pixel(%.0f,%.0f)",
                  building.id,
                  typeName.c_str(),
                  building.level,
                  building.gridX,
                  building.gridY,
                  pixelPos.x,
                  pixelPos.y);
        }
    }

    CCLOG("----------------------------------------");
    CCLOG("Defense Buildings Summary:");
    CCLOG("  - Cannons: %d", cannonCount);
    CCLOG("  - Archer Towers: %d", archerTowerCount);
    CCLOG("========================================\n");
}

Sprite* BattleMapLayer::createMapSprite() {
    // 从4张地图中随机选择一张
    static const std::vector<std::string> mapImages = {
        "Scene/VillageScene.png",
        "Scene/Map_Classic_Winter.png",
        "Scene/Map_Crossover.png",
        "Scene/Map_Royale.png"
    };
    
    int randomIndex = cocos2d::RandomHelper::random_int(0, (int)mapImages.size() - 1);
    const std::string& selectedMap = mapImages[randomIndex];
    CCLOG("BattleMapLayer: Selected random map: %s", selectedMap.c_str());
    
    auto mapSprite = Sprite::create(selectedMap);
    if (!mapSprite) {
        CCLOG("Error: Failed to load map image: %s", selectedMap.c_str());
        return nullptr;
    }
    mapSprite->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
    mapSprite->setPosition(Vec2::ZERO);
    return mapSprite;
}

void BattleMapLayer::update(float dt) {
    Layer::update(dt);

    if (_buildingManager) {
        _buildingManager->update(dt);
    }
}

void BattleMapLayer::reloadMapFromData() {
    CCLOG("BattleMapLayer: Reloading map from existing data (replay mode)...");

    // 清理旧的BuildingManager
    if (_buildingManager) {
        delete _buildingManager;
        _buildingManager = nullptr;
    }

    // 创建新的BuildingManager（从VillageDataManager读取当前数据）
    _buildingManager = new BuildingManager(this, true);

    // 输出建筑布局
    logBuildingLayout("REPLAY MAP LOADED");

    auto dataManager = VillageDataManager::getInstance();
    CCLOG("BattleMapLayer: Replay map reloaded with %zu buildings",
          dataManager->getBattleMapData().buildings.size());
}
