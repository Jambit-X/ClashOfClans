#include "DefenseSystem.h"
#include "../Layer/BattleTroopLayer.h"
#include "../Manager/VillageDataManager.h"
#include "../Model/BuildingConfig.h"
#include "../Util/GridMapUtils.h"
#include "../Sprite/BattleUnitSprite.h"
#include "../Sprite/BuildingSprite.h"
#include "../Component/DefenseBuildingAnimation.h"

USING_NS_CC;

DefenseSystem* DefenseSystem::_instance = nullptr;

DefenseSystem* DefenseSystem::getInstance() {
    if (!_instance) {
        _instance = new DefenseSystem();
    }
    return _instance;
}

void DefenseSystem::destroyInstance() {
    if (_instance) {
        delete _instance;
        _instance = nullptr;
    }
}

// ==========================================
// 查找攻击范围内最近的兵种
// ==========================================

BattleUnitSprite* DefenseSystem::findNearestUnitInRange(
    const BuildingInstance& building, 
    float attackRangeGrids,
    BattleTroopLayer* troopLayer) {
    
    if (!troopLayer) return nullptr;
    
    auto config = BuildingConfig::getInstance()->getConfig(building.type);
    if (!config) return nullptr;
    
    int centerX = building.gridX + config->gridWidth / 2;
    int centerY = building.gridY + config->gridHeight / 2;
    
    auto allUnits = troopLayer->getAllUnits();
    if (allUnits.empty()) return nullptr;
    
    BattleUnitSprite* nearestUnit = nullptr;
    int minGridDistance = INT_MAX;
    int attackRangeInt = static_cast<int>(attackRangeGrids);
    
    for (auto unit : allUnits) {
        if (!unit) continue;
        
        // 气球兵是飞行单位，只有箭塔（302）能攻击它
        // 加农炮（301）只能攻击地面单位
        if (unit->getUnitTypeID() == UnitTypeID::BALLOON && building.type != 302) {
            continue;  // 跳过气球兵
        }
        
        Vec2 unitGridPos = unit->getGridPosition();
        int unitGridX = static_cast<int>(unitGridPos.x);
        int unitGridY = static_cast<int>(unitGridPos.y);
        
        int gridDistance = std::max(
            std::abs(unitGridX - centerX),
            std::abs(unitGridY - centerY)
        );
        
        if (gridDistance <= attackRangeInt) {
            if (gridDistance < minGridDistance) {
                minGridDistance = gridDistance;
                nearestUnit = unit;
            }
        }
    }
    
    if (nearestUnit) {
        nearestUnit->setTargetedByBuilding(true);
    }
    
    return nearestUnit;
}

// ==========================================
// 获取攻击范围内所有兵种
// ==========================================

std::vector<BattleUnitSprite*> DefenseSystem::getAllUnitsInRange(
    const BuildingInstance& building, 
    float attackRangeGrids,
    BattleTroopLayer* troopLayer) {
    
    std::vector<BattleUnitSprite*> unitsInRange;
    
    if (!troopLayer) return unitsInRange;
    
    auto config = BuildingConfig::getInstance()->getConfig(building.type);
    if (!config) return unitsInRange;
    
    int centerX = building.gridX + config->gridWidth / 2;
    int centerY = building.gridY + config->gridHeight / 2;
    
    auto allUnits = troopLayer->getAllUnits();
    int attackRangeInt = static_cast<int>(attackRangeGrids);
    
    for (auto unit : allUnits) {
        if (!unit) continue;
        
        Vec2 unitGridPos = unit->getGridPosition();
        int unitGridX = static_cast<int>(unitGridPos.x);
        int unitGridY = static_cast<int>(unitGridPos.y);
        
        int gridDistance = std::max(
            std::abs(unitGridX - centerX),
            std::abs(unitGridY - centerY)
        );
        
        if (gridDistance <= attackRangeInt) {
            unitsInRange.push_back(unit);
        }
    }
    
    return unitsInRange;
}

// ==========================================
// 建筑防御自动更新系统
// ==========================================

void DefenseSystem::updateBuildingDefense(BattleTroopLayer* troopLayer) {
    if (!troopLayer) return;

    auto dataManager = VillageDataManager::getInstance();
    auto& buildings = const_cast<std::vector<BuildingInstance>&>(dataManager->getAllBuildings());

    std::set<BattleUnitSprite*> targetedUnitsThisFrame;
    float deltaTime = Director::getInstance()->getDeltaTime();

    for (auto& building : buildings) {
        // 跳过非防御建筑
        if (building.isDestroyed || building.currentHP <= 0) continue;
        if (building.state == BuildingInstance::State::PLACING) continue;
        if (building.type != 301 && building.type != 302) continue;

        auto config = BuildingConfig::getInstance()->getConfig(building.type);
        if (!config) continue;

        float attackRange = config->attackRange;
        float attackSpeed = config->attackSpeed;

        BattleUnitSprite* currentTarget = static_cast<BattleUnitSprite*>(building.lockedTarget);

        // ========== 目标有效性检查 ==========
        bool targetValid = false;

        if (currentTarget) {
            auto allUnits = troopLayer->getAllUnits();

            // 检查1: 目标是否还存活
            for (auto unit : allUnits) {
                if (unit == currentTarget && !unit->isDead()) {
                    targetValid = true;
                    break;
                }
            }

            // 检查2: 是否还在范围内
            if (targetValid) {
                int centerX = building.gridX + config->gridWidth / 2;
                int centerY = building.gridY + config->gridHeight / 2;

                Vec2 unitGridPos = currentTarget->getGridPosition();
                int gridDistance = std::max(
                    std::abs((int)unitGridPos.x - centerX),
                    std::abs((int)unitGridPos.y - centerY)
                );

                if (gridDistance > static_cast<int>(attackRange)) {
                    targetValid = false;
                }
            }

            // 目标无效，清除锁定
            if (!targetValid) {
                building.lockedTarget = nullptr;
                currentTarget = nullptr;
            }
        }

        // ========== 寻找新目标 ==========
        if (!currentTarget) {
            BattleUnitSprite* newTarget = findNearestUnitInRange(building, attackRange, troopLayer);
            if (newTarget && !newTarget->isDead()) {
                building.lockedTarget = static_cast<void*>(newTarget);
                currentTarget = newTarget;
                building.attackCooldown = 0.0f;
            }
        }

        // ========== 攻击逻辑 ==========
        if (currentTarget) {
            targetedUnitsThisFrame.insert(currentTarget);

            building.attackCooldown -= deltaTime;

            if (building.attackCooldown <= 0.0f) {
                int damagePerShot = static_cast<int>(config->damagePerSecond * attackSpeed);
                currentTarget->takeDamage(damagePerShot);

                // 转换到 MapLayer 坐标系播放攻击动画
                auto mapLayer = troopLayer->getParent();
                if (mapLayer) {
                    std::string spriteName = "Building_" + std::to_string(building.id);
                    auto buildingSprite = dynamic_cast<BuildingSprite*>(mapLayer->getChildByName(spriteName));

                    if (buildingSprite) {
                        auto defenseAnim = dynamic_cast<DefenseBuildingAnimation*>(
                            buildingSprite->getChildByName("DefenseAnim")
                            );

                        if (defenseAnim) {
                            Vec2 unitPosInTroopLayer = currentTarget->getPosition();
                            Vec2 targetPosInMapLayer = troopLayer->convertToNodeSpace(
                                troopLayer->getParent()->convertToWorldSpace(unitPosInTroopLayer)
                            );

                            CCLOG("DefenseSystem: Aiming at target - Unit pos: (%.1f, %.1f)",
                                  unitPosInTroopLayer.x, unitPosInTroopLayer.y);

                            defenseAnim->playAttackAnimation(targetPosInMapLayer);
                        }
                    }
                }

                building.attackCooldown = attackSpeed;

                // 目标死亡，立即清除锁定并播放死亡动画
                if (currentTarget->isDead()) {
                    building.lockedTarget = nullptr;
                    targetedUnitsThisFrame.erase(currentTarget);
                    currentTarget->setTargetedByBuilding(false);
                    currentTarget->stopAllActions();

                    currentTarget->playDeathAnimation([troopLayer, currentTarget]() {
                        troopLayer->removeUnit(currentTarget);
                    });

                    CCLOG("DefenseSystem: Unit killed, playing death animation");
                }
            }
        }
    }

    // ========== 更新兵种锁定状态 ==========
    auto allUnits = troopLayer->getAllUnits();
    for (auto unit : allUnits) {
        if (!unit || unit->isDead()) continue;

        bool shouldBeTargeted = (targetedUnitsThisFrame.find(unit) != targetedUnitsThisFrame.end());
        if (unit->isTargetedByBuilding() != shouldBeTargeted) {
            unit->setTargetedByBuilding(shouldBeTargeted);
        }
    }
}
