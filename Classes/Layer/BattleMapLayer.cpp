#include "BattleMapLayer.h"
#include "Manager/BuildingManager.h"
#include "Manager/VillageDataManager.h"
#include "Controller/MoveMapController.h"
#include "Util/GridMapUtils.h"  // 【新增】用于坐标转换

USING_NS_CC;

BattleMapLayer::~BattleMapLayer() {
    // 清理 MoveMapController（它会自动清理事件监听器）
    if (_inputController) {
        delete _inputController;
        _inputController = nullptr;
    }

    // 清理 BuildingManager
    if (_buildingManager) {
        delete _buildingManager;
        _buildingManager = nullptr;
    }

    CCLOG("BattleMapLayer: Destroyed and cleaned up resources");
}

bool BattleMapLayer::init() {
    if (!Layer::init()) return false;

    // 1. 创建地图背景
    _mapSprite = createMapSprite();
    this->addChild(_mapSprite);

    // 设置 Layer 大小与地图一致
    if (_mapSprite) {
        this->setContentSize(_mapSprite->getContentSize());
        this->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
    }

    // 2. 初始化建筑管理器（战斗场景：isBattleScene = true）
    _buildingManager = new BuildingManager(this, true);

    // 【新增】输出建筑布局
    logBuildingLayout("INITIAL LOAD");

    // 3. 初始化地图移动控制器 (仅拖拽缩放)
    _inputController = new MoveMapController(this);
    _inputController->setupInputListeners();

    // 【新增】启动定时更新
    this->scheduleUpdate();

    // 【新增】监听目标锁定事件
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

    // 1. 生成新的随机地图
    auto dataManager = VillageDataManager::getInstance();
    dataManager->generateRandomBattleMap(0);

    // 2. 清理旧的 BuildingManager
    if (_buildingManager) {
        delete _buildingManager;
        _buildingManager = nullptr;
    }

    // 3. 创建新的 BuildingManager
    _buildingManager = new BuildingManager(this, true);

    // 【新增】输出建筑布局
    logBuildingLayout("RELOAD MAP");

    CCLOG("BattleMapLayer: Map reloaded with %zu buildings",
          dataManager->getBattleMapData().buildings.size());
}

// 【新增】输出建筑布局的辅助方法
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
    auto mapSprite = Sprite::create("Scene/LinedVillageScene.jpg");
    if (!mapSprite) {
        CCLOG("Error: Failed to load map image");
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
