// DebugHelper.cpp
// 调试辅助工具实现，提供资源修改、建筑操作和存档管理功能

#pragma execution_character_set("utf-8")
#include "DebugHelper.h"
#include "../Manager/VillageDataManager.h"
#include "../Layer/VillageLayer.h"
#include "../Layer/HUDLayer.h"
#include "../Manager/BuildingManager.h"
#include "../Model/BuildingConfig.h"
#include "../Scene/VillageScene.h"

USING_NS_CC;

// ========== 资源操作实现 ==========

void DebugHelper::setGold(int amount) {
    auto dataManager = VillageDataManager::getInstance();
    
    // 计算当前值与目标值的差值
    int current = dataManager->getGold();
    int diff = amount - current;
    
    if (diff > 0) {
        // 需要增加金币：调用addGold()
        // 注意：addGold()内部有上限检查，这里绕过上限直接加
        dataManager->addGold(diff);
    } else if (diff < 0) {
        // 需要减少金币：调用spendGold()
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

// ========== 建筑操作实现 ==========

void DebugHelper::setBuildingLevel(int buildingId, int level) {
    auto dataManager = VillageDataManager::getInstance();
    auto building = dataManager->getBuildingById(buildingId);
    
    if (!building) {
        CCLOG("DebugHelper: Building %d not found", buildingId);
        return;
    }
    
    // 直接修改建筑等级（BuildingInstance的字段都是public）
    building->level = level;
    building->state = BuildingInstance::State::BUILT;
    building->finishTime = 0;
    
    CCLOG("DebugHelper: Building %d level set to %d", buildingId, level);
    
    // 更新精灵显示：获取VillageLayer并刷新该建筑的外观
    auto scene = Director::getInstance()->getRunningScene();
    if (scene) {
        auto villageLayer = dynamic_cast<VillageLayer*>(scene->getChildByTag(1));
        if (villageLayer) {
            villageLayer->updateBuildingDisplay(buildingId);
        }
    }
    
    // 保存到存档文件
    dataManager->saveToFile("village.json");
    
    // 触发资源更新事件（因为等级变化可能影响存储容量等属性）
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
    
    // 步骤1：获取VillageLayer和HUDLayer引用
    auto villageLayer = dynamic_cast<VillageLayer*>(scene->getChildByTag(1));
    auto hudLayer = dynamic_cast<HUDLayer*>(scene->getChildByTag(100));
    
    // 步骤2：隐藏HUD操作菜单（避免菜单指向已删除的建筑）
    if (hudLayer) {
        hudLayer->hideBuildingActions();
    }
    
    // 步骤3：删除精灵
    // removeBuildingSprite()内部会：
    // 1. 调用hideSelectionEffect()清除光圈
    // 2. 从场景中移除精灵节点
    // 3. 清空_buildingSprites映射表
    if (villageLayer) {
        villageLayer->removeBuildingSprite(buildingId);
    }
    
    // 步骤4：从数据层删除建筑
    // removeBuilding()内部会：
    // 1. 从buildings列表中移除
    // 2. 调用updateGridOccupancy()更新网格占用状态
    dataManager->removeBuilding(buildingId);
    
    // 步骤5：保存存档
    dataManager->saveToFile("village.json");
    
    // 步骤6：触发资源更新事件（删除资源建筑可能影响容量）
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
    
    // 收集所有处于CONSTRUCTING状态的建筑ID
    for (const auto& building : buildings) {
        if (building.state == BuildingInstance::State::CONSTRUCTING) {
            constructingIds.push_back(building.id);
        }
    }
    
    // 完成每个建造中的建筑
    for (int id : constructingIds) {
        auto building = dataManager->getBuildingById(id);
        if (!building) continue;
        
        // 根据是新建还是升级，调用不同的完成方法
        if (building->isInitialConstruction) {
            // 新建建筑：finishNewBuildingConstruction()
            // 内部会：更新状态、扣除费用、更新精灵
            dataManager->finishNewBuildingConstruction(id);
        } else {
            // 升级建筑：finishUpgradeBuilding()
            // 内部会：更新状态、等级+1、扣除费用、更新精灵
            dataManager->finishUpgradeBuilding(id);
        }
    }
    
    CCLOG("DebugHelper: Completed %lu constructions", constructingIds.size());
}

// ========== 存档操作实现 ==========

void DebugHelper::resetSaveData() {
    auto fileUtils = FileUtils::getInstance();
    std::string writablePath = fileUtils->getWritablePath();
    std::string savePath = writablePath + "village.json";
    
    // 检查并删除存档文件
    if (fileUtils->isFileExist(savePath)) {
        fileUtils->removeFile(savePath);
        CCLOG("DebugHelper: Save file deleted: %s", savePath.c_str());
    }
    
    // 销毁数据管理器单例，强制下次重新初始化
    VillageDataManager::destroyInstance();
    
    // 重新加载VillageScene（将使用默认数据初始化）
    auto scene = VillageScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene, Color3B::BLACK));
    
    CCLOG("DebugHelper: Save reset complete, reloading scene with default data");
}

void DebugHelper::forceSave() {
    auto dataManager = VillageDataManager::getInstance();
    dataManager->saveToFile("village.json");
    CCLOG("DebugHelper: Force saved to village.json");
}
