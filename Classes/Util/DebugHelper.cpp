#pragma execution_character_set("utf-8")
#include "DebugHelper.h"
#include "../Manager/VillageDataManager.h"
#include "../Layer/VillageLayer.h"
#include "../Layer/HUDLayer.h"
#include "../Manager/BuildingManager.h"
#include "../Model/BuildingConfig.h"
#include "../Scene/VillageScene.h"

USING_NS_CC;

// ========== 资源操作 ==========

void DebugHelper::setGold(int amount) {
    auto dataManager = VillageDataManager::getInstance();
    
    // 计算差值并调用已有方法
    int current = dataManager->getGold();
    int diff = amount - current;
    
    if (diff > 0) {
        // 临时移除上限检查，直接设置
        // 注意：VillageData 的 gold 是 public 的，可通过 getter 间接访问
        // 但为了不修改 VillageDataManager，我们用加减法绕过上限
        dataManager->addGold(diff);
    } else if (diff < 0) {
        dataManager->spendGold(-diff);
    }
    
    CCLOG("DebugHelper: Set gold to %d (diff=%d)", amount, diff);
}

void DebugHelper::setElixir(int amount) {
    auto dataManager = VillageDataManager::getInstance();
    int current = dataManager->getElixir();
    int diff = amount - current;
    
    if (diff > 0) {
        dataManager->addElixir(diff);
    } else if (diff < 0) {
        dataManager->spendElixir(-diff);
    }
    
    CCLOG("DebugHelper: Set elixir to %d", amount);
}

void DebugHelper::setGem(int amount) {
    auto dataManager = VillageDataManager::getInstance();
    int current = dataManager->getGem();
    int diff = amount - current;
    
    if (diff > 0) {
        dataManager->addGem(diff);
    } else if (diff < 0) {
        dataManager->spendGem(-diff);
    }
    
    CCLOG("DebugHelper: Set gem to %d", amount);
}

// ========== 建筑操作 ==========

void DebugHelper::setBuildingLevel(int buildingId, int level) {
    auto dataManager = VillageDataManager::getInstance();
    auto building = dataManager->getBuildingById(buildingId);
    
    if (!building) {
        CCLOG("DebugHelper: Building %d not found", buildingId);
        return;
    }
    
    // 直接修改等级（BuildingInstance 是可修改的）
    building->level = level;
    building->state = BuildingInstance::State::BUILT;
    building->finishTime = 0;
    
    CCLOG("DebugHelper: Building %d level set to %d", buildingId, level);
    
    // 更新精灵显示
    auto scene = Director::getInstance()->getRunningScene();
    if (scene) {
        auto villageLayer = dynamic_cast<VillageLayer*>(scene->getChildByTag(1));
        if (villageLayer) {
            villageLayer->updateBuildingDisplay(buildingId);
        }
    }
    
    // 保存
    dataManager->saveToFile("village.json");
    
    // 触发资源更新（可能影响存储容量等）
    Director::getInstance()->getEventDispatcher()
        ->dispatchCustomEvent("EVENT_RESOURCE_CHANGED");
}

void DebugHelper::deleteBuilding(int buildingId) {
    auto dataManager = VillageDataManager::getInstance();
    auto scene = Director::getInstance()->getRunningScene();
    
    if (!scene) {
        CCLOG("DebugHelper: No running scene");
        return;
    }
    
    // 1. 获取 VillageLayer 和 HUDLayer
    auto villageLayer = dynamic_cast<VillageLayer*>(scene->getChildByTag(1));
    auto hudLayer = dynamic_cast<HUDLayer*>(scene->getChildByTag(100));
    
    // 2. 隐藏 HUD 操作菜单
    if (hudLayer) {
        hudLayer->hideBuildingActions();
    }
    
    // 3. 删除精灵（内部会自动清除选中状态，避免光圈残留）
    if (villageLayer) {
        villageLayer->removeBuildingSprite(buildingId);
    }
    
    // 4. 从数据层删除（同时更新网格占用）
    dataManager->removeBuilding(buildingId);
    
    // 5. 保存存档
    dataManager->saveToFile("village.json");
    
    // 6. 触发资源更新事件
    Director::getInstance()->getEventDispatcher()
        ->dispatchCustomEvent("EVENT_RESOURCE_CHANGED");
    
    CCLOG("DebugHelper: Building %d deleted with full coordination", buildingId);
}

void DebugHelper::completeAllConstructions() {
    auto dataManager = VillageDataManager::getInstance();
    auto scene = Director::getInstance()->getRunningScene();
    
    // 获取所有建筑
    const auto& buildings = dataManager->getAllBuildings();
    std::vector<int> constructingIds;
    
    // 收集所有建造中的建筑ID
    for (const auto& building : buildings) {
        if (building.state == BuildingInstance::State::CONSTRUCTING) {
            constructingIds.push_back(building.id);
        }
    }
    
    // 完成每个建筑
    for (int id : constructingIds) {
        auto building = dataManager->getBuildingById(id);
        if (!building) continue;
        
        if (building->isInitialConstruction) {
            dataManager->finishNewBuildingConstruction(id);
        } else {
            dataManager->finishUpgradeBuilding(id);
        }
    }
    
    CCLOG("DebugHelper: Completed %lu constructions", constructingIds.size());
}

// ========== 存档操作 ==========

void DebugHelper::resetSaveData() {
    auto fileUtils = FileUtils::getInstance();
    std::string writablePath = fileUtils->getWritablePath();
    std::string savePath = writablePath + "village.json";
    
    if (fileUtils->isFileExist(savePath)) {
        fileUtils->removeFile(savePath);
        CCLOG("DebugHelper: Save file deleted: %s", savePath.c_str());
    }
    
    // 销毁数据管理器单例，让它下次重新初始化
    VillageDataManager::destroyInstance();
    
    // 重新加载 VillageScene（自动使用默认数据初始化）
    auto scene = VillageScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene, Color3B::BLACK));
    
    CCLOG("DebugHelper: Save reset complete, reloading scene with default data");
}

void DebugHelper::forceSave() {
    auto dataManager = VillageDataManager::getInstance();
    dataManager->saveToFile("village.json");
    CCLOG("DebugHelper: Force saved to village.json");
}
