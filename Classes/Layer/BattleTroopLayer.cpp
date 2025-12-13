#include "BattleTroopLayer.h"
#include "../Manager/AnimationManager.h"
#include "../Manager/VillageDataManager.h"
#include "../Model/BuildingConfig.h"
#include "../Util/GridMapUtils.h"
#include "../Util/FindPathUtil.h"

USING_NS_CC;

BattleTroopLayer* BattleTroopLayer::create() {
    auto layer = new (std::nothrow) BattleTroopLayer();
    if (layer && layer->init()) {
        layer->autorelease();
        return layer;
    }
    CC_SAFE_DELETE(layer);
    return nullptr;
}

bool BattleTroopLayer::init() {
    if (!Layer::init()) {
        return false;
    }
    
    // ===== 确保层的坐标系与地图对齐 =====
    this->setAnchorPoint(Vec2::ZERO);  // 左下角锚点，与地图一致
    this->setPosition(Vec2::ZERO);     // 位置设为 (0, 0)
    // =========================================
    
    CCLOG("BattleTroopLayer: Initialized");
    CCLOG("  AnchorPoint: (0, 0)");
    CCLOG("  Position: (0, 0)");

    // ===== 调试模式：自动运行测试 =====
#if COCOS2D_DEBUG
    // 方案1：生成网格野蛮人（225个）
    // this->spawnUnitsGrid("Barbarian", 3);
    
    // 方案2：运行攻击/移动动画测试序列（1个会动的野蛮人）
    // this->runBarbarianTestSequence();
    
    // 方案3：运行寻路功能验证测试（3个野蛮人）
    // this->runPathfindingTest();
    
    // 方案4： 交互式寻路测试（生成1个野蛮人，等待建筑放置）
    this->startInteractivePathfindingTest();
    
    CCLOG("BattleTroopLayer: Interactive pathfinding test enabled");
    CCLOG("  -> Spawn a barbarian at (5, 5)");
    CCLOG("  -> Place a building, barbarian will walk to it");
#endif
    // =====================================
    
    return true;
}

// ========== 单位生成 ==========

BattleUnitSprite* BattleTroopLayer::spawnUnit(const std::string& unitType, int gridX, int gridY) {
    // 边界检查
    if (gridX < 0 || gridX >= GRID_WIDTH || gridY < 0 || gridY >= GRID_HEIGHT) {
        CCLOG("BattleTroopLayer: Invalid grid position (%d, %d)", gridX, gridY);
        return nullptr;
    }
    
    // 创建单位
    auto unit = BattleUnitSprite::create(unitType);
    if (!unit) {
        CCLOG("BattleTroopLayer: Failed to create unit '%s'", unitType.c_str());
        return nullptr;
    }
    
    // 设置位置
    unit->teleportToGrid(gridX, gridY);
    unit->playIdleAnimation();
    
    // 添加到层级
    this->addChild(unit);
    _units.push_back(unit);
    
    CCLOG("BattleTroopLayer: Spawned %s at grid(%d, %d)", unitType.c_str(), gridX, gridY);
    return unit;
}

void BattleTroopLayer::spawnUnitsGrid(const std::string& unitType, int spacing) {
    int count = 0;
    
    for (int gridY = 0; gridY < GRID_HEIGHT; gridY += spacing) {
        for (int gridX = 0; gridX < GRID_WIDTH; gridX += spacing) {
            if (spawnUnit(unitType, gridX, gridY)) {
                count++;
            }
        }
    }
    
    CCLOG("BattleTroopLayer: Spawned %d units in grid pattern", count);
}

void BattleTroopLayer::removeAllUnits() {
    for (auto unit : _units) {
        this->removeChild(unit);
    }
    _units.clear();
    CCLOG("BattleTroopLayer: Removed all units");
}

// ========== 调试测试序列 ==========

#if COCOS2D_DEBUG

void BattleTroopLayer::runBarbarianTestSequence() {
    CCLOG("=========================================");
    CCLOG("BattleTroopLayer: Starting Barbarian Test Sequence");
    CCLOG("=========================================");
    
    // 创建测试野蛮人
    auto barbarian = spawnUnit("Barbarian", 0, 0);
    if (!barbarian) {
        CCLOG("BattleTroopLayer: Failed to spawn test barbarian");
        return;
    }
    
    // 使用 Cocos2d Action 系统重构测试序列（替代嵌套回调）
    float delay = 0.0f;
    const float ATTACK_DELAY = 1.0f;
    
    // ===== 第一轮攻击：8个方向 =====
    
    // 1. 向右上攻击
    auto attack1 = Sequence::create(
        DelayTime::create(delay += 1.0f),
        CallFunc::create([barbarian]() {
            CCLOG("  -> Attack RIGHT-UP");
            barbarian->attackInDirection(Vec2(1, 1));
        }),
        nullptr
    );
    
    // 2. 向右攻击
    auto attack2 = Sequence::create(
        DelayTime::create(delay += ATTACK_DELAY),
        CallFunc::create([barbarian]() {
            CCLOG("  -> Attack RIGHT");
            barbarian->attackInDirection(Vec2(1, 0));
        }),
        nullptr
    );
    
    // 3. 向右下攻击
    auto attack3 = Sequence::create(
        DelayTime::create(delay += ATTACK_DELAY),
        CallFunc::create([barbarian]() {
            CCLOG("  -> Attack RIGHT-DOWN");
            barbarian->attackInDirection(Vec2(1, -1));
        }),
        nullptr
    );
    
    // 4. 向左上攻击
    auto attack4 = Sequence::create(
        DelayTime::create(delay += ATTACK_DELAY),
        CallFunc::create([barbarian]() {
            CCLOG("  -> Attack LEFT-UP");
            barbarian->attackInDirection(Vec2(-1, 1));
        }),
        nullptr
    );
    
    // 5. 向左攻击
    auto attack5 = Sequence::create(
        DelayTime::create(delay += ATTACK_DELAY),
        CallFunc::create([barbarian]() {
            CCLOG("  -> Attack LEFT");
            barbarian->attackInDirection(Vec2(-1, 0));
        }),
        nullptr
    );
    
    // 6. 向左下攻击
    auto attack6 = Sequence::create(
        DelayTime::create(delay += ATTACK_DELAY),
        CallFunc::create([barbarian]() {
            CCLOG("  -> Attack LEFT-DOWN");
            barbarian->attackInDirection(Vec2(-1, -1));
        }),
        nullptr
    );
    
    // ===== 移动阶段 =====
    
    // 移动到 (0, 5)
    auto walk1 = Sequence::create(
        DelayTime::create(delay += ATTACK_DELAY + 0.5f),
        CallFunc::create([barbarian]() {
            CCLOG("Phase 2: Walking to grid (0, 5)");
            barbarian->walkToGrid(0, 5, 150.0f);
        }),
        nullptr
    );
    
    // 移动到 (5, 5)
    auto walk2 = Sequence::create(
        DelayTime::create(delay += 3.0f),
        CallFunc::create([barbarian]() {
            CCLOG("Phase 3: Walking to grid (5, 5)");
            barbarian->walkToGrid(5, 5, 150.0f);
        }),
        nullptr
    );
    
    // ===== 第二轮攻击 =====
    
    auto attack7 = Sequence::create(
        DelayTime::create(delay += 3.0f + 1.0f),
        CallFunc::create([barbarian]() {
            CCLOG("  -> Attack RIGHT-UP");
            barbarian->attackInDirection(Vec2(1, 1));
        }),
        nullptr
    );
    
    auto attack8 = Sequence::create(
        DelayTime::create(delay += ATTACK_DELAY),
        CallFunc::create([barbarian]() {
            CCLOG("  -> Attack RIGHT");
            barbarian->attackInDirection(Vec2(1, 0));
        }),
        nullptr
    );
    
    auto attack9 = Sequence::create(
        DelayTime::create(delay += ATTACK_DELAY),
        CallFunc::create([barbarian]() {
            CCLOG("  -> Attack RIGHT-DOWN");
            barbarian->attackInDirection(Vec2(1, -1));
        }),
        nullptr
    );
    
    auto attack10 = Sequence::create(
        DelayTime::create(delay += ATTACK_DELAY),
        CallFunc::create([barbarian]() {
            CCLOG("  -> Attack LEFT-UP");
            barbarian->attackInDirection(Vec2(-1, 1));
        }),
        nullptr
    );
    
    auto attack11 = Sequence::create(
        DelayTime::create(delay += ATTACK_DELAY),
        CallFunc::create([barbarian]() {
            CCLOG("  -> Attack LEFT");
            barbarian->attackInDirection(Vec2(-1, 0));
        }),
        nullptr
    );
    
    auto attack12 = Sequence::create(
        DelayTime::create(delay += ATTACK_DELAY),
        CallFunc::create([barbarian]() {
            CCLOG("  -> Attack LEFT-DOWN");
            barbarian->attackInDirection(Vec2(-1, -1));
        }),
        nullptr
    );
    
    // ===== 死亡动画 =====
    
    auto death = Sequence::create(
        DelayTime::create(delay += ATTACK_DELAY + 1.0f),
        CallFunc::create([]() {
            CCLOG("=========================================");
            CCLOG("Test Sequence Completed!");
            CCLOG("=========================================");
        }),
        CallFunc::create([barbarian]() {
            barbarian->playDeathAnimation();
        }),
        nullptr
    );
    
    // 执行所有动作
    this->runAction(attack1);
    this->runAction(attack2);
    this->runAction(attack3);
    this->runAction(attack4);
    this->runAction(attack5);
    this->runAction(attack6);
    this->runAction(walk1);
    this->runAction(walk2);
    this->runAction(attack7);
    this->runAction(attack8);
    this->runAction(attack9);
    this->runAction(attack10);
    this->runAction(attack11);
    this->runAction(attack12);
    this->runAction(death);
}

// =====  新增：寻路功能验证测试 =====

void BattleTroopLayer::runPathfindingTest() {
    CCLOG("=========================================");
    CCLOG("BattleTroopLayer: Starting Pathfinding Test");
    CCLOG("=========================================");
    
    // 创建3个测试野蛮人
    auto barbarian1 = spawnUnit("Barbarian", 2, 2);   // 左下角
    auto barbarian2 = spawnUnit("Barbarian", 40, 2);  // 右下角
    auto barbarian3 = spawnUnit("Barbarian", 2, 40);  // 左上角
    
    if (!barbarian1 || !barbarian2 || !barbarian3) {
        CCLOG("!!! Failed to spawn test barbarians");
        return;
    }
    
    CCLOG(" Spawned 3 test barbarians:");
    CCLOG("   - Barbarian1 at (2, 2)");
    CCLOG("   - Barbarian2 at (40, 2)");
    CCLOG("   - Barbarian3 at (2, 40)");
    
    // ===== 测试1：野蛮人1 从左下角走到右上角（需要绕过建筑） =====
    auto test1 = Sequence::create(
        DelayTime::create(1.0f),
        CallFunc::create([barbarian1]() {
            CCLOG("========================================");
            CCLOG("TEST 1: Barbarian1 walks from (2,2) to (40,40)");
            CCLOG("  Expected: Should pathfind around buildings");
            CCLOG("========================================");
            
            barbarian1->moveToGridWithPathfinding(40, 40, 100.0f, []() {
                CCLOG(" TEST 1 COMPLETED: Barbarian1 arrived at (40,40)");
            });
        }),
        nullptr
    );
    
    // ===== 测试2：野蛮人2 从右下角走到左上角 =====
    auto test2 = Sequence::create(
        DelayTime::create(3.0f),
        CallFunc::create([barbarian2]() {
            CCLOG("========================================");
            CCLOG("TEST 2: Barbarian2 walks from (40,2) to (2,40)");
            CCLOG("  Expected: Should find diagonal path");
            CCLOG("========================================");
            
            barbarian2->moveToGridWithPathfinding(2, 40, 100.0f, []() {
                CCLOG(" TEST 2 COMPLETED: Barbarian2 arrived at (2,40)");
            });
        }),
        nullptr
    );
    
    // ===== 测试3：野蛮人3 从左上角走到中心 =====
    auto test3 = Sequence::create(
        DelayTime::create(5.0f),
        CallFunc::create([barbarian3]() {
            CCLOG("========================================");
            CCLOG("TEST 3: Barbarian3 walks from (2,40) to (22,22)");
            CCLOG("  Expected: Should navigate to center");
            CCLOG("========================================");
            
            barbarian3->moveToGridWithPathfinding(22, 22, 100.0f, []() {
                CCLOG(" TEST 3 COMPLETED: Barbarian3 arrived at (22,22)");
            });
        }),
        nullptr
    );
    
    // ===== 测试4：测试无法到达的目标（如果目标被建筑包围） =====
    auto test4 = Sequence::create(
        DelayTime::create(15.0f),
        CallFunc::create([barbarian1]() {
            CCLOG("========================================");
            CCLOG("TEST 4: Barbarian1 tries to reach an occupied tile");
            CCLOG("  Expected: Should fail gracefully");
            CCLOG("========================================");
            
            // 尝试走到一个可能被建筑占用的位置
            barbarian1->moveToGridWithPathfinding(0, 0, 100.0f, []() {
                CCLOG(" TEST 4 COMPLETED (or failed, check logs)");
            });
        }),
        nullptr
    );
    
    // ===== 测试完成总结 =====
    auto summary = Sequence::create(
        DelayTime::create(25.0f),
        CallFunc::create([]() {
            CCLOG("=========================================");
            CCLOG("PATHFINDING TEST SEQUENCE COMPLETED!");
            CCLOG("=========================================");
            CCLOG("Check the logs above to verify:");
            CCLOG("  1. Units successfully navigated around obstacles");
            CCLOG("  2. FindPathUtil logged path waypoints");
            CCLOG("  3. No units got stuck or crashed");
            CCLOG("=========================================");
        }),
        nullptr
    );
    
    // 执行所有测试
    this->runAction(test1);
    this->runAction(test2);
    this->runAction(test3);
    this->runAction(test4);
    this->runAction(summary);
}

#endif // COCOS2D_DEBUG

// ========== 交互式寻路测试 ==========

#if COCOS2D_DEBUG

void BattleTroopLayer::startInteractivePathfindingTest() {
    CCLOG("=========================================");
    CCLOG("BattleTroopLayer: Starting Interactive Pathfinding Test");
    CCLOG("=========================================");
    
    // 初始化建筑计数
    auto dataManager = VillageDataManager::getInstance();
    _lastBuildingCount = dataManager->getAllBuildings().size();
    
    // 在地图中心生成一个野蛮人
    _testBarbarian = spawnUnit("Barbarian", 5, 5);
    
    if (!_testBarbarian) {
        CCLOG("!!! Failed to spawn test barbararian");
        return;
    }
    
    CCLOG(" Test barbarian spawned at (5, 5)");
    CCLOG("📌 Instructions:");
    CCLOG("   1. Place a building anywhere on the map");
    CCLOG("   2. Barbarian will automatically walk to the building");
    CCLOG("   3. Watch it pathfind around obstacles!");
    CCLOG("   Current building count: %lu", _lastBuildingCount);
    
    // 每秒检查是否有新建筑放置
    this->schedule([this](float dt) {
        this->checkForNewBuildings();
    }, 1.0f, "check_new_buildings");
}

void BattleTroopLayer::checkForNewBuildings() {
    auto dataManager = VillageDataManager::getInstance();
    const auto& buildings = dataManager->getAllBuildings();
    
    // ===== 检查1：是否有新建筑 =====
    if (buildings.size() > _lastBuildingCount) {
        CCLOG("========================================");
        CCLOG("🏗️ New building detected!");
        CCLOG("  Previous count: %lu", _lastBuildingCount);
        CCLOG("  Current count: %lu", buildings.size());
        CCLOG("========================================");
        
        const auto& newBuilding = buildings.back();
        
        CCLOG("New building info:");
        CCLOG("  ID: %d", newBuilding.id);
        CCLOG("  Type: %d", static_cast<int>(newBuilding.type));
        CCLOG("  Position: grid(%d, %d)", newBuilding.gridX, newBuilding.gridY);
        
        if (_testBarbarian) {
            this->walkBarbarianToBuilding(newBuilding);
        }
        
        _lastBuildingCount = buildings.size();
        
        // 记录所有建筑的位置（用于检测移动）
        _buildingPositions.clear();
        for (const auto& b : buildings) {
            _buildingPositions[b.id] = Vec2(b.gridX, b.gridY);
        }
        
        return;
    }
    
    // ===== 检查2：建筑是否移动了 =====
    for (const auto& building : buildings) {
        Vec2 currentPos(building.gridX, building.gridY);
        
        // 如果这个建筑之前存在，检查位置是否改变
        auto it = _buildingPositions.find(building.id);
        if (it != _buildingPositions.end()) {
            Vec2 oldPos = it->second;
            
            // 位置改变了！
            if (oldPos.x != currentPos.x || oldPos.y != currentPos.y) {
                CCLOG("========================================");
                CCLOG("📦 Building moved!");
                CCLOG("  Building ID: %d", building.id);
                CCLOG("  From: grid(%d, %d)", (int)oldPos.x, (int)oldPos.y);
                CCLOG("  To: grid(%d, %d)", building.gridX, building.gridY);
                CCLOG("========================================");
                
                // 让野蛮人走向新位置
                if (_testBarbarian) {
                    this->walkBarbarianToBuilding(building);
                }
                
                // 更新位置记录
                _buildingPositions[building.id] = currentPos;
                
                break; // 一次只处理一个移动事件
            }
        } else {
            // 新建筑，记录位置
            _buildingPositions[building.id] = currentPos;
        }
    }
}

void BattleTroopLayer::walkBarbarianToBuilding(const BuildingInstance& building) {
    CCLOG("========================================");
    CCLOG("🚶 Walking Barbarian to Building");
    CCLOG("========================================");
    
    if (!_testBarbarian) {
        CCLOG("❌ Test barbarian not found!");
        return;
    }
    
    // 获取野蛮人当前世界坐标
    Vec2 barbarianWorldPos = _testBarbarian->getPosition();
    
    CCLOG("Barbarian position: world(%.1f, %.1f)", barbarianWorldPos.x, barbarianWorldPos.y);
    CCLOG("Target building: Type=%d, grid(%d, %d)", 
          static_cast<int>(building.type), building.gridX, building.gridY);
    
    //  核心逻辑：直接调用 FindPathUtil 的智能寻路
    auto pathfinder = FindPathUtil::getInstance();
    std::vector<Vec2> worldPath = pathfinder->findPathToAttackBuilding(barbarianWorldPos, building);
    
    if (worldPath.empty()) {
        CCLOG("❌ No path found to building!");
        CCLOG("========================================");
        return;
    }
    
    CCLOG(" Path found with %lu waypoints", worldPath.size());
    CCLOG("========================================");
    
    // 让野蛮人跟随路径移动
    _testBarbarian->followPath(worldPath, 100.0f, []() {
        CCLOG("========================================");
        CCLOG(" Barbarian arrived at building!");
        CCLOG("========================================");
        CCLOG("📌 You can place or move another building to test again!");
    });
}

#endif // COCOS2D_DEBUG
void BattleTroopLayer::startUnitAI(BattleUnitSprite* unit) {
    if (!unit) return;

    // 1. 寻找最近的目标
    const BuildingInstance* target = findNearestBuilding(unit->getPosition());

    if (!target) {
        CCLOG("No target found! Unit idling.");
        unit->playIdleAnimation();
        return;
    }

    // 2.【核心】直接调用已有的寻路功能
    auto pathfinder = FindPathUtil::getInstance();
    // 使用 findPathToAttackBuilding 自动计算绕过障碍的路径
    std::vector<Vec2> path = pathfinder->findPathToAttackBuilding(unit->getPosition(), *target);

    if (path.empty()) {
        CCLOG("No path to target!");
        unit->playIdleAnimation();
        return;
    }

    // 3. 执行移动
    // followPath 也是你已经写好的，直接用
    unit->followPath(path, 100.0f, [unit, target, this]() {
        CCLOG("Unit arrived at target!");

        // 到达后，计算朝向并攻击
        Vec2 buildingPos = GridMapUtils::gridToPixelCenter(target->gridX, target->gridY);

        // 播放攻击动画 (暂无伤害逻辑，先做表现)
        unit->attackTowardPosition(buildingPos, [unit]() {
            unit->playIdleAnimation(); // 攻击一次后待机
            // TODO: 这里以后接循环攻击和扣血逻辑
            });
        });
}

const BuildingInstance* BattleTroopLayer::findNearestBuilding(const Vec2& unitWorldPos) {
    auto dataManager = VillageDataManager::getInstance();
    const auto& buildings = dataManager->getAllBuildings();

    const BuildingInstance* nearest = nullptr;
    float minStartDistSq = FLT_MAX;

    for (const auto& building : buildings) {
        // 简单计算距离，寻找最近的建筑
        auto config = BuildingConfig::getInstance()->getConfig(building.type);
        if (!config) continue;

        Vec2 bPos = GridMapUtils::gridToPixelCenter(building.gridX, building.gridY);
        float distSq = unitWorldPos.distanceSquared(bPos);

        if (distSq < minStartDistSq) {
            minStartDistSq = distSq;
            nearest = &building;
        }
    }
    return nearest;
}