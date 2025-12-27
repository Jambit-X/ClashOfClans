// VillageLayer.cpp
// 村庄层实现，处理村庄场景的地图、建筑和交互逻辑

#include "VillageLayer.h"
#include "../Controller/MoveMapController.h"
#include "../Controller/MoveBuildingController.h"
#include "../Util/GridMapUtils.h"
#include "../proj.win32/Constants.h"
#include "Manager/BuildingManager.h"
#include "Manager/VillageDataManager.h"
#include "Sprite/BattleUnitSprite.h"
#include "Sprite/BuildingSprite.h"   
#include "Model/BuildingConfig.h"      
#include "ui/CocosGUI.h"
#include <iostream>
#include "Layer/HUDLayer.h"

USING_NS_CC;

bool VillageLayer::init() {
  if (!Layer::init()) {
    return false;
  }

  // 创建地图精灵
  _mapSprite = createMapSprite();
  if (!_mapSprite) {
    return false;
  }
  this->addChild(_mapSprite);

  // 初始化基本属性
  initializeBasicProperties();

  // 初始化建筑管理器（村庄场景）
  _buildingManager = new BuildingManager(this, false);

  // 先初始化建筑移动控制器（优先级更高）
  _moveBuildingController = new MoveBuildingController(this, _buildingManager);
  _moveBuildingController->setupTouchListener();
  
  // 设置短按建筑回调
  _moveBuildingController->setOnBuildingTappedCallback([this](int buildingId) {
      CCLOG("VillageLayer: Building tapped ID=%d, showing menu", buildingId);
    
      // 处理选中效果
      auto tappedBuilding = _buildingManager->getBuildingSprite(buildingId);
      if (tappedBuilding) {
          // 隐藏之前选中建筑的效果
          if (_currentSelectedBuilding && _currentSelectedBuilding != tappedBuilding) {
              _currentSelectedBuilding->hideSelectionEffect();
          }
          // 显示当前建筑选中效果
          tappedBuilding->showSelectionEffect();
          _currentSelectedBuilding = tappedBuilding;
      }

      // 获取HUD层并显示菜单
      auto scene = this->getScene();
      if (scene) {
          auto hudLayer = dynamic_cast<HUDLayer*>(scene->getChildByTag(100));
          if (hudLayer) {
              hudLayer->showBuildingActions(buildingId);
          }
      }
  });

  // 后初始化地图移动控制器（优先级更低）
  _inputController = new MoveMapController(this);
  _inputController->setupInputListeners();

  // 设置点击检测回调
  setupInputCallbacks();

  // 启动建筑更新
  this->schedule([this](float dt) {
    _buildingManager->update(dt);
  }, 0.0f, "building_update");

  // 启动建造完成检测（每秒检查）
  this->schedule([this](float dt) {
    auto dataManager = VillageDataManager::getInstance();
    dataManager->checkAndFinishConstructions();
  }, 1.0f, "construction_checker");

  CCLOG("VillageLayer initialized successfully");
  CCLOG("  - MoveBuildingController added first (higher priority)");
  CCLOG("  - MoveMapController added second (lower priority)");

  // 监听建筑升级事件
  auto upgradeListener = EventListenerCustom::create("EVENT_BUILDING_UPGRADED", 
    [this](EventCustom* event) {
        int buildingId = *(int*)event->getUserData();
        
        // 更新建筑精灵外观
        auto dataManager = VillageDataManager::getInstance();
        auto building = dataManager->getBuildingById(buildingId);
        if (building) {
            for (auto child : this->getChildren()) {
                auto buildingSprite = dynamic_cast<BuildingSprite*>(child);
                if (buildingSprite && buildingSprite->getBuildingId() == buildingId) {
                    buildingSprite->updateLevel(building->level);
                    buildingSprite->updateState(building->state);
                    break;
                }
            }
        }
    });
  _eventDispatcher->addEventListenerWithSceneGraphPriority(upgradeListener, this);

  // 监听建筑加速完成事件
  auto speedupListener = EventListenerCustom::create("EVENT_BUILDING_SPEEDUP_COMPLETE",
                                                     [this](EventCustom* event) {
    int buildingId = *(int*)event->getUserData();

    CCLOG("VillageLayer: Building %d speedup complete, updating UI", buildingId);

    auto dataManager = VillageDataManager::getInstance();
    auto building = dataManager->getBuildingById(buildingId);

    if (building && _buildingManager) {
      auto sprite = _buildingManager->getBuildingSprite(buildingId);
      if (sprite) {
        sprite->hideConstructionProgress();
        sprite->finishConstruction();
        sprite->updateBuilding(*building);
      }
    }
  });
  _eventDispatcher->addEventListenerWithSceneGraphPriority(speedupListener, this);

  // 初始化当前场景主题
  auto dataManager = VillageDataManager::getInstance();
  int currentTheme = dataManager->getCurrentThemeId();

  // 如果不是默认场景，切换主题
  if (currentTheme != 1) {
      this->runAction(cocos2d::Sequence::create(
          cocos2d::DelayTime::create(0.1f),
          cocos2d::CallFunc::create([this, currentTheme]() {
          switchMapBackground(currentTheme);
      }),
          nullptr
      ));
  }
  return true;
}

void VillageLayer::cleanup() {
  // 清理粒子效果
  if (_currentParticleEffect) {
    _currentParticleEffect->stopSystem();
    _currentParticleEffect->removeFromParent();
    _currentParticleEffect = nullptr;
  }

  // 清理建筑移动控制器
  if (_moveBuildingController) {
    delete _moveBuildingController;
    _moveBuildingController = nullptr;
  }

  // 清理建筑管理器
  if (_buildingManager) {
    delete _buildingManager;
    _buildingManager = nullptr;
  }

  if (_inputController) {
    _inputController->cleanup();
    delete _inputController;
    _inputController = nullptr;
  }
  
  Layer::cleanup();
}

void VillageLayer::onBuildingPurchased(int buildingId) {
  CCLOG("VillageLayer: Building purchased ID=%d, entering placement mode", buildingId);

  auto dataManager = VillageDataManager::getInstance();
  auto building = dataManager->getBuildingById(buildingId);

  if (building) {
    // 创建建筑精灵
    auto sprite = _buildingManager->addBuilding(*building);
    
    // 通知HUDLayer开始放置流程
    auto scene = this->getScene();
    if (scene) {
      auto hudLayer = dynamic_cast<HUDLayer*>(scene->getChildByTag(100));
      if (hudLayer) {
        hudLayer->startBuildingPlacement(buildingId);
      }
    }

    CCLOG("VillageLayer: Building sprite created, waiting for placement");
  }
}

void VillageLayer::initializeBasicProperties() {
  auto mapSize = _mapSprite->getContentSize();
  this->setContentSize(mapSize);

  // 使用左下角锚点
  this->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);

  CCLOG("VillageLayer basic properties initialized:");
  CCLOG("  Map size: %.0fx%.0f", mapSize.width, mapSize.height);
}

Sprite* VillageLayer::createMapSprite() {
  auto mapSprite = Sprite::create("Scene/Map_Crossover.png");
  if (!mapSprite) {
    CCLOG("Error: Failed to load map image");
    return nullptr;
  }

  // 使用左下角锚点
  mapSprite->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
  mapSprite->setPosition(Vec2::ZERO);
  return mapSprite;
}

BuildingSprite* VillageLayer::getBuildingAtScreenPos(const Vec2& screenPos) {
  Vec2 worldPos = this->convertToNodeSpace(screenPos);
  return _buildingManager->getBuildingAtWorldPos(worldPos);
}

int VillageLayer::getSelectedBuildingId() const {
    if (_currentSelectedBuilding) {
        return _currentSelectedBuilding->getBuildingId();
    }
    return -1;
}

void VillageLayer::setupInputCallbacks() {
    CCLOG("VillageLayer: Setting up input callbacks");

    // 回调1: 点击检测
    _inputController->setOnTapCallback([this](const Vec2& screenPos) -> TapTarget {
        auto building = getBuildingAtScreenPos(screenPos);

        // 获取HUD层
        auto scene = this->getScene();
        HUDLayer* hudLayer = nullptr;
        if (scene) {
            hudLayer = dynamic_cast<HUDLayer*>(scene->getChildByTag(100));
        }

        if (building) {
            CCLOG("VillageLayer: Tap detected on building ID=%d", building->getBuildingId());
            return TapTarget::BUILDING;
        }

        // 点击空白处，隐藏菜单和选中效果
        if (hudLayer) {
            hudLayer->hideBuildingActions();
        }
        if (_currentSelectedBuilding) {
            _currentSelectedBuilding->hideSelectionEffect();
            _currentSelectedBuilding = nullptr;
        }

        return TapTarget::NONE;
    });

    // 回调2: 建筑选中
    _inputController->setOnBuildingSelectedCallback([this](const Vec2& screenPos) {
      auto building = getBuildingAtScreenPos(screenPos);

      auto scene = this->getScene();
      HUDLayer* hudLayer = nullptr;
      if (scene) {
        hudLayer = dynamic_cast<HUDLayer*>(scene->getChildByTag(100));
      }

      if (building) {
        int buildingId = building->getBuildingId();
        CCLOG("VillageLayer: Building selected ID=%d", buildingId);

        // 处理选中效果
        if (_currentSelectedBuilding && _currentSelectedBuilding != building) {
            _currentSelectedBuilding->hideSelectionEffect();
        }
        building->showSelectionEffect();
        _currentSelectedBuilding = building;

        auto dataManager = VillageDataManager::getInstance();
        auto buildingData = dataManager->getBuildingById(buildingId);

        if (buildingData) {
          if (buildingData->state == BuildingInstance::State::BUILT) {
            // 已完成的建筑：显示完整操作菜单
            if (hudLayer) {
              hudLayer->showBuildingActions(buildingId);
            }
          } else if (buildingData->state == BuildingInstance::State::CONSTRUCTING) {
            CCLOG("VillageLayer: Constructing building tapped, can still be moved by long press");
          } else if (buildingData->state == BuildingInstance::State::PLACING) {
            CCLOG("VillageLayer: PLACING building should not trigger select callback");
          }
        }
      }
    });

    CCLOG("VillageLayer: Input callbacks configured");
}

void VillageLayer::removeBuildingSprite(int buildingId) {
  if (_buildingManager) {
    // 检查是否是当前选中的建筑
    auto sprite = _buildingManager->getBuildingSprite(buildingId);
    if (sprite && _currentSelectedBuilding == sprite) {
      _currentSelectedBuilding->hideSelectionEffect();
      _currentSelectedBuilding = nullptr;
      CCLOG("VillageLayer: Cleared selection for deleted building ID=%d", buildingId);
    }
    
    _buildingManager->removeBuildingSprite(buildingId);
    CCLOG("VillageLayer: Removed building sprite ID=%d", buildingId);
  }
}

void VillageLayer::updateBuildingDisplay(int buildingId) {
  auto dataManager = VillageDataManager::getInstance();
  auto building = dataManager->getBuildingById(buildingId);
  
  if (building && _buildingManager) {
    _buildingManager->updateBuilding(buildingId, *building);
    CCLOG("VillageLayer: Updated building display for ID=%d", buildingId);
  }
}

void VillageLayer::clearSelectedBuilding() {
  if (_currentSelectedBuilding) {
    _currentSelectedBuilding->hideSelectionEffect();
    _currentSelectedBuilding = nullptr;
    CCLOG("VillageLayer: Cleared selected building");
  }
}

void VillageLayer::updateBuildingPreviewPosition(int buildingId, const cocos2d::Vec2& worldPos) {
  auto sprite = _buildingManager->getBuildingSprite(buildingId);
  if (!sprite) {
    CCLOG("VillageLayer: Building sprite not found ID=%d", buildingId);
    return;
  }

  // 转换为网格坐标
  cocos2d::Vec2 gridPosFloat = GridMapUtils::pixelToGrid(worldPos);

  auto config = BuildingConfig::getInstance()->getConfig(sprite->getBuildingType());
  if (!config) return;

  // 计算建筑左下角网格坐标
  float leftBottomGridX = gridPosFloat.x - config->gridWidth * 0.5f;
  float leftBottomGridY = gridPosFloat.y - config->gridHeight * 0.5f;

  int gridX = (int)std::round(leftBottomGridX);
  int gridY = (int)std::round(leftBottomGridY);

  // 计算对齐后的世界坐标
  cocos2d::Vec2 alignedWorldPos = GridMapUtils::getVisualPosition(
    gridX, gridY, sprite->getVisualOffset()
  );

  // 更新精灵位置（平滑移动）
  cocos2d::Vec2 currentPos = sprite->getPosition();
  cocos2d::Vec2 smoothPos = currentPos.lerp(alignedWorldPos, 0.3f);
  sprite->setPosition(smoothPos);

  // 更新网格坐标
  sprite->setGridPos(cocos2d::Vec2(gridX, gridY));

  // 更新数据层坐标
  auto dataManager = VillageDataManager::getInstance();
  dataManager->setBuildingPosition(buildingId, gridX, gridY);

  // 检查是否可以放置
  bool canPlace = !dataManager->isAreaOccupied(
    gridX,
    gridY,
    config->gridWidth,
    config->gridHeight,
    buildingId
  );

  // 显示视觉反馈
  sprite->setDraggingMode(true);
  sprite->setPlacementPreview(canPlace);

  // 通知HUDLayer更新按钮状态
  auto scene = this->getScene();
  if (scene) {
    auto hudLayer = dynamic_cast<HUDLayer*>(scene->getChildByTag(100));
    if (hudLayer) {
      hudLayer->updatePlacementUIState(canPlace);
    }
  }

  CCLOG("VillageLayer: Preview at grid(%d, %d) - %s",
        gridX, gridY, canPlace ? "VALID" : "INVALID");
}

void VillageLayer::switchMapBackground(int themeId) {
    CCLOG("VillageLayer: Switching to theme %d", themeId);

    auto dataManager = VillageDataManager::getInstance();

    // 确定地图文件路径
    std::string mapPath;
    bool needParticle = false;

    switch (themeId) {
        case 1:
            mapPath = "Scene/VillageScene.png";
            needParticle = false;
            break;
        case 2:
            mapPath = "Scene/Map_Classic_Winter.png";
            needParticle = true;
            break;
        case 3:
            mapPath = "Scene/Map_Royale.png";
            needParticle = false;
            break;
        case 4:
            mapPath = "Scene/Map_Crossover.png";
            needParticle = false;
            break;
        default:
            CCLOG("VillageLayer: Unknown theme ID %d, using default", themeId);
            mapPath = "Scene/VillageScene.png";
            needParticle = false;
            break;
    }

    // 清理当前粒子效果
    if (_currentParticleEffect) {
        _currentParticleEffect->stopSystem();
        _currentParticleEffect->removeFromParent();
        _currentParticleEffect = nullptr;
        CCLOG("VillageLayer: Cleaned up previous particle effect");
    }

    // 替换地图精灵
    if (_mapSprite) {
        cocos2d::Vec2 oldPos = _mapSprite->getPosition();
        _mapSprite->removeFromParent();

        _mapSprite = cocos2d::Sprite::create(mapPath);
        if (_mapSprite) {
            _mapSprite->setAnchorPoint(cocos2d::Vec2::ANCHOR_BOTTOM_LEFT);
            _mapSprite->setPosition(oldPos);
            this->addChild(_mapSprite, -1);
            CCLOG("VillageLayer: Map background changed to %s", mapPath.c_str());
        } else {
            CCLOG("VillageLayer: ERROR - Failed to load map %s", mapPath.c_str());
        }
    }

    // 如果需要，创建雪花粒子效果
    if (needParticle) {
        CCLOG("VillageLayer: Creating snow particle effect...");

        cocos2d::Size mapSize = _mapSprite->getContentSize();

        _currentParticleEffect = cocos2d::ParticleSnow::create();
        if (_currentParticleEffect) {
            _currentParticleEffect->setPosition(cocos2d::Vec2(
                mapSize.width / 2,
                mapSize.height + 50
            ));

            _currentParticleEffect->setPosVar(cocos2d::Vec2(mapSize.width / 2, 0));

            _currentParticleEffect->setGravity(cocos2d::Vec2(0, -50));
            _currentParticleEffect->setSpeed(80);
            _currentParticleEffect->setSpeedVar(20);
            _currentParticleEffect->setLife(12.0f);
            _currentParticleEffect->setLifeVar(3.0f);
            _currentParticleEffect->setEmissionRate(20);
            _currentParticleEffect->setTotalParticles(500);

            _currentParticleEffect->setStartSize(15.0f);
            _currentParticleEffect->setStartSizeVar(6.0f);
            _currentParticleEffect->setEndSize(18.0f);

            _currentParticleEffect->setStartColor(cocos2d::Color4F(1.0f, 1.0f, 1.0f, 1.0f));
            _currentParticleEffect->setEndColor(cocos2d::Color4F(1.0f, 1.0f, 1.0f, 0.0f));

            _mapSprite->addChild(_currentParticleEffect, 100);

            CCLOG("VillageLayer: Snow particle effect created successfully");
            CCLOG("  - Position: (%.0f, %.0f)", mapSize.width / 2, mapSize.height + 50);
            CCLOG("  - PosVar: (%.0f, 0)", mapSize.width / 2);
            CCLOG("  - Emission Rate: 20, Total Particles: 500");
            CCLOG("  - Size: Start=15, End=18");
        } else {
            CCLOG("VillageLayer: ERROR - Failed to create snow particle effect!");
        }
    }

    CCLOG("VillageLayer: Theme switch complete");
}
