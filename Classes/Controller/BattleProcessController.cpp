#include "BattleProcessController.h"
#include "../Layer/BattleTroopLayer.h"
#include "../Manager/VillageDataManager.h"
#include "../Model/BuildingConfig.h"
#include "../Util/GridMapUtils.h"
#include "../Util/FindPathUtil.h"

USING_NS_CC;

BattleProcessController* BattleProcessController::_instance = nullptr;

BattleProcessController* BattleProcessController::getInstance() {
    if (!_instance) {
        _instance = new BattleProcessController();
    }
    return _instance;
}

void BattleProcessController::destroyInstance() {
    if (_instance) {
        delete _instance;
        _instance = nullptr;
    }
}

void BattleProcessController::startUnitAI(BattleUnitSprite* unit, BattleTroopLayer* troopLayer) {
    if (!unit) {
        CCLOG("BattleProcessController: Invalid unit pointer");
        return;
    }

    // 1. 智能寻找最佳目标建筑（考虑距离和城墙）
    const BuildingInstance* target = findBestTargetBuilding(unit->getPosition());

    if (!target) {
        CCLOG("BattleProcessController: No target found! Unit idling.");
        unit->playIdleAnimation();
        return;
    }

    auto pathfinder = FindPathUtil::getInstance();
    Vec2 unitPos = unit->getPosition();
    Vec2 targetCenter = GridMapUtils::gridToPixelCenter(target->gridX, target->gridY);

    // ========== 智能决策：比较破墙路径和绕墙路径 ==========/
    
    // 路径A：忽略城墙（假设会攻击城墙)
    std::vector<Vec2> pathWithWalls = pathfinder->findPathIgnoringWalls(unitPos, targetCenter);
    
    // 路径B：绕过城墙（城墙作为障碍）
    std::vector<Vec2> pathAroundWalls = pathfinder->findPathToAttackBuilding(unitPos, *target);
    
    int pathALen = pathWithWalls.size();
    int pathBLen = pathAroundWalls.size();
    
    CCLOG("BattleProcessController: Path comparison:");
    CCLOG("  Path A (through walls): %d waypoints", pathALen);
    CCLOG("  Path B (around walls): %d waypoints", pathBLen);
    
    std::vector<Vec2> chosenPath;
    
    // 决策逻辑
    if (pathBLen == 0) {
        // 无法绕路，只能破墙
        CCLOG("  -> Decision: NO DETOUR POSSIBLE, must attack walls");
        chosenPath = pathWithWalls;
    } else if (pathALen == 0) {
        // 破墙路径失败，使用绕路
        CCLOG("  -> Decision: Using detour path");
        chosenPath = pathAroundWalls;
    } else {
        int detourCost = pathBLen - pathALen;
        
        if (detourCost <= WALL_DETOUR_THRESHOLD) {
            // 绕路成本可接受，选择绕路
            CCLOG("  -> Decision: DETOUR (cost: %d <= %d)", detourCost, WALL_DETOUR_THRESHOLD);
            chosenPath = pathAroundWalls;
        } else {
            // 绕路太远，选择破墙
            CCLOG("  -> Decision: ATTACK WALLS (detour cost: %d > %d)", detourCost, WALL_DETOUR_THRESHOLD);
            chosenPath = pathWithWalls;
        }
    }
    
    if (chosenPath.empty()) {
        CCLOG("BattleProcessController: No valid path found!");
        unit->playIdleAnimation();
        return;
    }

    // ========== 执行移动 ==========/
    
    CCLOG("BattleProcessController: Starting AI for unit, path length: %lu", chosenPath.size());

    unit->followPath(chosenPath, 100.0f, [unit, target]() {
        CCLOG("BattleProcessController: Unit arrived at target!");

        Vec2 buildingPos = GridMapUtils::gridToPixelCenter(target->gridX, target->gridY);

        unit->attackTowardPosition(buildingPos, [unit]() {
            unit->playIdleAnimation();
            // TODO: 循环攻击和扣血逻辑
        });
    });
}

// ========== 智能目标选择实现 ==========

const BuildingInstance* BattleProcessController::findBestTargetBuilding(const Vec2& unitWorldPos) {
    auto dataManager = VillageDataManager::getInstance();
    const auto& buildings = dataManager->getAllBuildings();
    auto pathfinder = FindPathUtil::getInstance();

    if (buildings.empty()) {
        CCLOG("BattleProcessController: No buildings in village!");
        return nullptr;
    }

    // ========== 第一阶段：预筛选（按直线距离排序） ==========/
    struct Candidate {
        const BuildingInstance* building;
        float distance;
    };
    std::vector<Candidate> candidates;
    candidates.reserve(buildings.size());

    for (const auto& building : buildings) {
        auto config = BuildingConfig::getInstance()->getConfig(building.type);
        if (!config) continue;

        Vec2 bPos = GridMapUtils::gridToPixelCenter(building.gridX, building.gridY);
        float dist = unitWorldPos.distance(bPos);
        candidates.push_back({ &building, dist });
    }

    // 按距离排序
    std::sort(candidates.begin(), candidates.end(), 
              [](const Candidate& a, const Candidate& b) {
                  return a.distance < b.distance;
              });

    // ========== 第二阶段：扩大搜索范围 ==========/
    // 基础搜索5个 + 阈值2个 = 7个候选建筑
    int searchCount = std::min((int)candidates.size(), EXPANDED_SEARCH_COUNT);
    
    CCLOG("BattleProcessController: Evaluating top %d candidates (base=%d + threshold=%d)",
          searchCount, BASE_SEARCH_COUNT, WALL_DETOUR_THRESHOLD);

    std::vector<TargetCandidate> evaluatedTargets;
    evaluatedTargets.reserve(searchCount);

    // ========== 第三阶段：评估每个候选建筑 ==========/
    for (int i = 0; i < searchCount; ++i) {
        const auto* building = candidates[i].building;
        float directDist = candidates[i].distance;
        
        Vec2 bPos = GridMapUtils::gridToPixelCenter(building->gridX, building->gridY);
        
        // 先尝试绕墙路径
        auto pathAround = pathfinder->findPathToAttackBuilding(unitWorldPos, *building);
        
        if (!pathAround.empty()) {
            // 可以绕墙到达
            float score = calculateBuildingScore(directDist, pathAround.size(), false);
            evaluatedTargets.push_back({ 
                building, 
                directDist, 
                (int)pathAround.size(), 
                false,  // 不需要破墙
                score 
            });
            
            CCLOG("  [%d] Building at (%d,%d): dist=%.1f, path=%d, walls=NO, score=%.1f",
                  i, building->gridX, building->gridY, directDist, 
                  (int)pathAround.size(), score);
            continue;
        }
        
        // 再尝试破墙路径
        auto pathThrough = pathfinder->findPathIgnoringWalls(unitWorldPos, bPos);
        if (!pathThrough.empty()) {
            // 可以破墙到达
            float score = calculateBuildingScore(directDist, pathThrough.size(), true);
            evaluatedTargets.push_back({ 
                building, 
                directDist, 
                (int)pathThrough.size(), 
                true,  // 需要破墙
                score 
            });
            
            CCLOG("  [%d] Building at (%d,%d): dist=%.1f, path=%d, walls=YES, score=%.1f",
                  i, building->gridX, building->gridY, directDist, 
                  (int)pathThrough.size(), score);
        } else {
            // 完全无法到达
            CCLOG("  [%d] Building at (%d,%d): UNREACHABLE, skipping", 
                  i, building->gridX, building->gridY);
        }
    }

    if (evaluatedTargets.empty()) {
        CCLOG("BattleProcessController: No reachable buildings in search range!");
        return nullptr;
    }

    // ========== 第四阶段：选择评分最低的目标 ==========/
    std::sort(evaluatedTargets.begin(), evaluatedTargets.end(), 
              [](const TargetCandidate& a, const TargetCandidate& b) {
                  return a.score < b.score;
              });

    const auto& best = evaluatedTargets[0];
    
    CCLOG("BattleProcessController: Selected BEST target:");
    CCLOG("  Building at grid(%d, %d)", best.building->gridX, best.building->gridY);
    CCLOG("  Direct distance: %.2f", best.directDistance);
    CCLOG("  Path length: %d waypoints", best.pathLength);
    CCLOG("  Needs breaking walls: %s", best.needsBreakingWalls ? "YES" : "NO");
    CCLOG("  Final score: %.2f (lower is better)", best.score);

    return best.building;
}

float BattleProcessController::calculateBuildingScore(
    float directDist, 
    int pathLength, 
    bool needsBreakingWalls
) {
    // 基础分数 = 路径长度 × 10
    float score = pathLength * 10.0f;
    
    // 如果需要破墙，增加惩罚
    if (needsBreakingWalls) {
        score += 100.0f;  // 破墙惩罚（可调整）
    }
    
    // 加上直线距离的小权重（用于路径长度相同时的区分）
    score += directDist * 0.5f;
    
    return score;
}

const BuildingInstance* BattleProcessController::findNearestBuilding(const Vec2& unitWorldPos) {
    auto dataManager = VillageDataManager::getInstance();
    const auto& buildings = dataManager->getAllBuildings();

    const BuildingInstance* nearest = nullptr;
    float minDistSq = FLT_MAX;

    for (const auto& building : buildings) {
        auto config = BuildingConfig::getInstance()->getConfig(building.type);
        if (!config) continue;

        Vec2 bPos = GridMapUtils::gridToPixelCenter(building.gridX, building.gridY);
        float distSq = unitWorldPos.distanceSquared(bPos);

        if (distSq < minDistSq) {
            minDistSq = distSq;
            nearest = &building;
        }
    }

    if (nearest) {
        CCLOG("BattleProcessController: Found nearest building at grid(%d, %d)", 
              nearest->gridX, nearest->gridY);
    }

    return nearest;
}
