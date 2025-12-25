#include "DestructionTracker.h"
#include "../Manager/VillageDataManager.h"
#include "../Model/BuildingConfig.h"
#include "../Model/VillageData.h"

USING_NS_CC;

DestructionTracker* DestructionTracker::_instance = nullptr;

DestructionTracker* DestructionTracker::getInstance() {
    if (!_instance) {
        _instance = new DestructionTracker();
    }
    return _instance;
}

void DestructionTracker::destroyInstance() {
    if (_instance) {
        delete _instance;
        _instance = nullptr;
    }
}

void DestructionTracker::reset() {
    _totalBuildingHP = 0;
    _currentStars = 0;
    _townHallDestroyed = false;
    _star50Awarded = false;
    _star100Awarded = false;
}

// ==========================================
// 初始化摧毁追踪
// ==========================================

void DestructionTracker::initTracking() {
    // 重置所有追踪变量
    reset();

    // 计算总血量
    _totalBuildingHP = calculateTotalBuildingHP();

    CCLOG("========================================");
    CCLOG("DestructionTracker: Destruction tracking initialized");
    CCLOG("Total Building HP: %d (excluding walls and traps)", _totalBuildingHP);
    CCLOG("========================================");
}

// ==========================================
// 计算总血量
// ==========================================

int DestructionTracker::calculateTotalBuildingHP() {
    auto dataManager = VillageDataManager::getInstance();
    const auto& buildings = dataManager->getAllBuildings();

    int totalHP = 0;
    int buildingCount = 0;

    for (const auto& building : buildings) {
        // 跳过城墙（type == 303）
        if (building.type == 303) continue;

        // 跳过陷阱（type >= 400 && type < 500）
        if (building.type >= 400 && building.type < 500) continue;

        // 跳过未建造完成的建筑
        if (building.state != BuildingInstance::State::BUILT) continue;

        // 获取建筑配置
        auto config = BuildingConfig::getInstance()->getConfig(building.type);
        if (!config) continue;

        // 累加血量
        int maxHP = config->hitPoints;
        if (maxHP > 0) {
            totalHP += maxHP;
            buildingCount++;

            CCLOG("  [%d] Type=%d, HP=%d", building.id, building.type, maxHP);
        }
    }

    CCLOG("DestructionTracker: Total %d buildings tracked, Total HP=%d",
          buildingCount, totalHP);

    return totalHP;
}

// ==========================================
// 更新摧毁进度
// ==========================================

void DestructionTracker::updateProgress() {
    // 如果总血量为0，说明没有初始化或没有建筑
    if (_totalBuildingHP <= 0) {
        CCLOG("DestructionTracker: Total HP is 0, skipping progress update");
        return;
    }

    auto dataManager = VillageDataManager::getInstance();
    const auto& buildings = dataManager->getAllBuildings();

    // 计算当前剩余血量
    int currentTotalHP = 0;
    bool townHallDestroyed = false;

    for (const auto& building : buildings) {
        // 使用与 calculateTotalBuildingHP 相同的过滤逻辑
        if (building.type == 303) continue;
        if (building.type >= 400 && building.type < 500) continue;
        if (building.state != BuildingInstance::State::BUILT) continue;

        // 检查大本营状态（type == 1）
        if (building.type == 1 && building.isDestroyed) {
            townHallDestroyed = true;
        }

        // 累加当前血量
        if (!building.isDestroyed && building.currentHP > 0) {
            currentTotalHP += building.currentHP;
        }
    }

    // 计算摧毁进度
    float progress = ((_totalBuildingHP - currentTotalHP) / (float)_totalBuildingHP) * 100.0f;

    // 限制在 0-100 范围内
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 100.0f) progress = 100.0f;

    CCLOG("========================================");
    CCLOG("DestructionTracker: Progress Updated");
    CCLOG("  Total HP: %d", _totalBuildingHP);
    CCLOG("  Current HP: %d", currentTotalHP);
    CCLOG("  Destroyed HP: %d", _totalBuildingHP - currentTotalHP);
    CCLOG("  Progress: %.1f%%", progress);
    CCLOG("  Town Hall Destroyed: %s", townHallDestroyed ? "YES" : "NO");
    CCLOG("========================================");

    // 先检查星级条件（此时 _townHallDestroyed 还是旧值）
    checkStarConditions(progress, townHallDestroyed);

    // 然后再更新大本营状态（放在检查之后）
    _townHallDestroyed = townHallDestroyed;

    // 发送进度更新事件
    DestructionProgressEventData eventData;
    eventData.progress = progress;
    eventData.stars = _currentStars;

    EventCustom event("EVENT_DESTRUCTION_PROGRESS_UPDATED");
    event.setUserData(&eventData);
    Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
}

// ==========================================
// 检查星级条件
// ==========================================

void DestructionTracker::checkStarConditions(float progress, bool townHallDestroyed) {
    int oldStars = _currentStars;

    // 三个条件独立累加，每个条件各加1颗星
    int newStars = 0;

    // ========== 第1颗星：摧毁进度 >= 50% ==========
    if (progress >= 50.0f) {
        newStars++;
        
        if (!_star50Awarded) {
            _star50Awarded = true;
            
            CCLOG("*** STAR AWARDED! ***");
            CCLOG("  Reason: 50%% Destruction");
            CCLOG("  Progress: %.1f%%", progress);

            // 发送星星获得事件
            StarAwardedEventData starData;
            starData.starIndex = 0;  // 第1颗星（索引0）
            starData.reason = "50%";

            EventCustom event("EVENT_STAR_AWARDED");
            event.setUserData(&starData);
            Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
        }
    }

    // ========== 第2颗星：大本营被摧毁 ==========
    if (townHallDestroyed) {
        newStars++;
        
        if (!_townHallDestroyed) {
            CCLOG("*** STAR AWARDED! ***");
            CCLOG("  Reason: Town Hall Destroyed");

            // 发送星星获得事件
            StarAwardedEventData starData;
            starData.starIndex = 1;  // 第2颗星（索引1）
            starData.reason = "townhall";

            EventCustom event("EVENT_STAR_AWARDED");
            event.setUserData(&starData);
            Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
        }
    }

    // ========== 第3颗星：摧毁进度 == 100% ==========
    if (progress >= 99.9f) {  // 使用 99.9 避免浮点误差
        newStars++;
        
        if (!_star100Awarded) {
            _star100Awarded = true;

            CCLOG("*** STAR AWARDED! ***");
            CCLOG("  Reason: 100%% Destruction");
            CCLOG("  Progress: %.1f%%", progress);

            // 发送星星获得事件
            StarAwardedEventData starData;
            starData.starIndex = 2;  // 第3颗星（索引2）
            starData.reason = "100%";

            EventCustom event("EVENT_STAR_AWARDED");
            event.setUserData(&starData);
            Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
        }
    }

    // 更新星数
    _currentStars = newStars;

    // 如果星数发生变化，输出日志
    if (_currentStars != oldStars) {
        CCLOG("DestructionTracker: Stars updated: %d -> %d", oldStars, _currentStars);
        CCLOG("  - 50%% progress: %s", progress >= 50.0f ? "YES" : "NO");
        CCLOG("  - Town Hall destroyed: %s", townHallDestroyed ? "YES" : "NO");
        CCLOG("  - 100%% progress: %s", progress >= 99.9f ? "YES" : "NO");
    }
}

// ==========================================
// 获取当前进度
// ==========================================

float DestructionTracker::getProgress() {
    if (_totalBuildingHP <= 0) return 0.0f;

    auto dataManager = VillageDataManager::getInstance();
    const auto& buildings = dataManager->getAllBuildings();

    int currentTotalHP = 0;

    for (const auto& building : buildings) {
        if (building.type == 303) continue;
        if (building.type >= 400 && building.type < 500) continue;
        if (building.state != BuildingInstance::State::BUILT) continue;

        if (!building.isDestroyed && building.currentHP > 0) {
            currentTotalHP += building.currentHP;
        }
    }

    float progress = ((_totalBuildingHP - currentTotalHP) / (float)_totalBuildingHP) * 100.0f;

    if (progress < 0.0f) progress = 0.0f;
    if (progress > 100.0f) progress = 100.0f;

    return progress;
}

int DestructionTracker::getStars() {
    return _currentStars;
}
