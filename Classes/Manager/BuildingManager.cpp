#include "BuildingManager.h"
#include "Manager/VillageDataManager.h"
#include "Model/BuildingConfig.h"
#include "Model/BattleMapData.h"
#include "Util/GridMapUtils.h"
#include "Component/DefenseBuildingAnimation.h" 

USING_NS_CC;

BuildingManager::BuildingManager(Layer* parentLayer, bool isBattleScene)
    : _parentLayer(parentLayer), _isBattleScene(isBattleScene) {
    
    // 战斗场景：优先加载战斗地图数据
    if (_isBattleScene) {
        auto dataManager = VillageDataManager::getInstance();
        if (dataManager->hasBattleMapData()) {
            loadFromBattleMapData();
        } else {
            // 没有预生成的战斗地图，自动生成一个
            dataManager->generateRandomBattleMap(0);
            loadFromBattleMapData();
        }
    } else {
        // 村庄场景：加载玩家村庄数据
        loadBuildingsFromData();
    }
    
    CCLOG("BuildingManager: Created for %s scene", isBattleScene ? "BATTLE" : "VILLAGE");
}

BuildingManager::~BuildingManager() {
    // 先清理防御动画（它们是子节点，会自动释放，这里只是清除引用）
    _defenseAnims.clear();

    // 【修复】先从场景中移除所有建筑精灵，再清除映射
    for (auto& pair : _buildings) {
        if (pair.second) {
            pair.second->removeFromParent();
        }
    }
    _buildings.clear();
    CCLOG("BuildingManager: Destroyed and removed all building sprites");
}

void BuildingManager::loadBuildingsFromData() {
    auto dataManager = VillageDataManager::getInstance();
    const auto& buildings = dataManager->getAllBuildings();

    for (const auto& building : buildings) {
        if (building.state != BuildingInstance::State::PLACING) {
            addBuilding(building);
        }
    }
    CCLOG("BuildingManager: Loaded %lu buildings from village data", _buildings.size());
}

void BuildingManager::loadFromBattleMapData() {
    auto dataManager = VillageDataManager::getInstance();
    const auto& mapData = dataManager->getBattleMapData();
    
    for (const auto& building : mapData.buildings) {
        addBuilding(building);
    }
    CCLOG("BuildingManager: Loaded %zu buildings from battle map (difficulty=%d)", 
          _buildings.size(), mapData.difficulty);
}


BuildingSprite* BuildingManager::addBuilding(const BuildingInstance& building) {
  auto sprite = BuildingSprite::create(building);
  if (!sprite) return nullptr;

  Vec2 worldPos = GridMapUtils::gridToPixel(building.gridX, building.gridY);
  Vec2 finalPos = worldPos + sprite->getVisualOffset();
  sprite->setPosition(finalPos);

  // 使用统一函数计算 Z-Order
  int zOrder = calculateZOrder(building.gridX, building.gridY);
  _parentLayer->addChild(sprite, zOrder);

  _buildings[building.id] = sprite;

  // 防御建筑特殊处理
  if (_isBattleScene && (building.type == 301 || building.type == 302)) {
      createDefenseAnimation(sprite, building);
  }

  CCLOG("BuildingManager: Added building ID=%d at grid(%d, %d), Z-Order=%d",
        building.id, building.gridX, building.gridY, zOrder);

  return sprite;
}
void BuildingManager::removeBuilding(int buildingId) {
    // 清理动画引用
    auto animIt = _defenseAnims.find(buildingId);
    if (animIt != _defenseAnims.end()) {
        _defenseAnims.erase(animIt);
    }

    // 清理建筑精灵
    auto it = _buildings.find(buildingId);
    if (it != _buildings.end()) {
        it->second->removeFromParent();
        _buildings.erase(it);
        CCLOG("BuildingManager: Removed building ID=%d", buildingId);
    }
}

// 【新增】创建防御建筑动画
void BuildingManager::createDefenseAnimation(BuildingSprite* sprite, const BuildingInstance& building) {
    if (!sprite) return;

    // 1. 【关键】保存原始尺寸
    auto originalSize = sprite->getContentSize();
    CCLOG("BuildingManager: Original size: (%.0f, %.0f)", originalSize.width, originalSize.height);

    // 2. 隐藏原始静态贴图
    sprite->setOpacity(0);
    sprite->setTextureRect(Rect(0, 0, 0, 0));  // 这会让 contentSize 变成 (0,0)

    // 3. 【关键】恢复 contentSize，确保 UI 元素位置正确
    sprite->setContentSize(originalSize);
    CCLOG("BuildingManager: Restored contentSize to (%.0f, %.0f)", originalSize.width, originalSize.height);

    // 4. 创建动画组件
    auto anim = DefenseBuildingAnimation::create(sprite, building.type);
    if (!anim) {
        CCLOG("BuildingManager: Failed to create defense animation for building ID=%d", building.id);
        sprite->setOpacity(255);
        sprite->loadSprite(building.type, building.level);
        return;
    }

    anim->setName("DefenseAnim");
    sprite->addChild(anim, -1);

    // 5. 根据建筑类型设置默认偏移量
    if (building.type == 301) {  // 加农炮
        anim->setAnimationOffset(Vec2(0, 0));  // 你调整的偏移量
        CCLOG("BuildingManager: Applied offset (0, -20) for cannon");
    }
    
    _defenseAnims[building.id] = anim;
    CCLOG("BuildingManager: Defense animation created for building ID=%d", building.id);
}

// 获取防御建筑动画
DefenseBuildingAnimation* BuildingManager::getDefenseAnimation(int buildingId) const {
    auto it = _defenseAnims.find(buildingId);
    return (it != _defenseAnims.end()) ? it->second : nullptr;
}

void BuildingManager::updateBuilding(int buildingId, const BuildingInstance& building) {
  auto sprite = getBuildingSprite(buildingId);
  if (sprite) {
    sprite->updateBuilding(building);

    Vec2 worldPos = GridMapUtils::gridToPixel(building.gridX, building.gridY);
    Vec2 finalPos = worldPos + sprite->getVisualOffset();
    sprite->setPosition(finalPos);

    // 使用统一函数计算 Z-Order 并重新排序
    int zOrder = calculateZOrder(building.gridX, building.gridY);
    _parentLayer->reorderChild(sprite, zOrder);

    // 状态切换到建造中时的特殊处理
    if (building.state == BuildingInstance::State::CONSTRUCTING) {
      CCLOG("BuildingManager: Building %d entering CONSTRUCTING state", buildingId);

      // 彻底清除拖动和预览状态
      sprite->setDraggingMode(false);
      sprite->clearPlacementPreview();

      // 启动建造动画（会把建筑变暗灰色）
      sprite->startConstruction();
    }

    CCLOG("BuildingManager: Updated building ID=%d", buildingId);
  }
}

BuildingSprite* BuildingManager::getBuildingSprite(int buildingId) const {
    auto it = _buildings.find(buildingId);
    return (it != _buildings.end()) ? it->second : nullptr;
}

// ? 修复：使用网格查询（O(1)，精准，无透明区域问题）
BuildingSprite* BuildingManager::getBuildingAtGrid(int gridX, int gridY) const {
    CCLOG("BuildingManager::getBuildingAtGrid - Looking at grid(%d, %d)", gridX, gridY);
    
    auto dataManager = VillageDataManager::getInstance();
    
    // 遍历所有建筑，找到占据该网格的建筑
    const auto& buildings = dataManager->getAllBuildings();
    for (const auto& building : buildings) {
        if (building.state == BuildingInstance::State::PLACING) continue;
        
        auto config = BuildingConfig::getInstance()->getConfig(building.type);
        if (!config) continue;
        
        // 检查网格是否在建筑范围内
        if (gridX >= building.gridX && gridX < building.gridX + config->gridWidth &&
            gridY >= building.gridY && gridY < building.gridY + config->gridHeight) {
            CCLOG("  -> FOUND! Building ID=%d", building.id);
            return getBuildingSprite(building.id);
        }
    }
    
    CCLOG("  -> NOT FOUND");
    return nullptr;
}

// ? 修复：点击检测改为网格查询（不再用 BoundingBox）
BuildingSprite* BuildingManager::getBuildingAtWorldPos(const Vec2& worldPos) const {
    CCLOG("BuildingManager::getBuildingAtWorldPos - Checking world pos (%.0f, %.0f)",
        worldPos.x, worldPos.y);

    // ? 1. 转换为网格坐标
    Vec2 gridPosFloat = GridMapUtils::pixelToGrid(worldPos);
    
    // ? 2. 四舍五入到整数网格
    int gridX = (int)std::round(gridPosFloat.x);
    int gridY = (int)std::round(gridPosFloat.y);
    
    CCLOG("  -> Converted to grid(%d, %d)", gridX, gridY);
    
    // ? 3. 查询网格（O(1)，精准，无透明区域问题）
    return getBuildingAtGrid(gridX, gridY);
}

void BuildingManager::update(float dt) {
  auto dataManager = VillageDataManager::getInstance();
  const auto& buildings = dataManager->getAllBuildings();
  long long currentTime = time(nullptr);

  for (auto& building : buildings) {
    if (building.state == BuildingInstance::State::CONSTRUCTING) {
      auto sprite = getBuildingSprite(building.id);
      if (!sprite) continue;

      // 检查是否完成
      if (building.finishTime > 0 && currentTime >= building.finishTime) {
        CCLOG("BuildingManager: Building ID=%d construction complete", building.id);
        
        // 1. 隐藏进度UI
        sprite->hideConstructionProgress();
        
        // 2. 使用标志位判断是新建筑还是升级
        if (building.isInitialConstruction) {
          // 新建筑：保持等级
          CCLOG("BuildingManager: New building construction (level=%d)", building.level);
          dataManager->finishNewBuildingConstruction(building.id);
        } else {
          // 升级：等级+1
          CCLOG("BuildingManager: Upgrade (level %d -> %d)", building.level, building.level + 1);
          dataManager->finishUpgradeBuilding(building.id);
        }
        
        // 3. 更新精灵
        sprite->finishConstruction();
        sprite->updateState(BuildingInstance::State::BUILT);
        
        auto updatedBuilding = dataManager->getBuildingById(building.id);
        if (updatedBuilding) {
            sprite->updateBuilding(*updatedBuilding);
        }
        
        continue;
      }

      // 更新建造进度
      if (building.finishTime > 0) {
        long long remainTime = building.finishTime - currentTime;

        auto config = BuildingConfig::getInstance()->getConfig(building.type);
        long long totalTime = config ? config->buildTimeSeconds : 300;
        if (totalTime <= 0) totalTime = 1;

        float progress = 1.0f - (static_cast<float>(remainTime) / (float)totalTime);
        progress = clampf(progress, 0.0f, 1.0f);

        sprite->showConstructionProgress(progress);
        sprite->showCountdown((int)remainTime);
        sprite->updateConstructionProgress(progress);
      }
    }
    // ========== 核心修复：只在战斗场景才处理 HP 状态 ==========
    if (_isBattleScene) {
      // 同步渲染：处理建筑的 HP 和摧毁状态
      for (const auto& b : buildings) {
        auto s = getBuildingSprite(b.id);
        if (!s) continue;

        if (b.isDestroyed) {
          // 如果已摧毁，显示废墟贴图并隐藏血条
          s->showDestroyedRubble();
        } else {
          // 恢复正常显示
          s->setColor(Color3B::WHITE);
          s->setOpacity(255);
          
          // 【新增】更新血条显示
          auto config = BuildingConfig::getInstance()->getConfig(b.type);
          int maxHP = (config && config->hitPoints > 0) ? config->hitPoints : 100;
          s->updateHealthBar(b.currentHP, maxHP);
        }
      }
    }
    // =========================================================
  }
}

void BuildingManager::removeBuildingSprite(int buildingId) {
  auto it = _buildings.find(buildingId);
  if (it != _buildings.end()) {
    CCLOG("BuildingManager: Removing sprite for building ID=%d", buildingId);
    it->second->removeFromParent();
    _buildings.erase(it);
  }
}
