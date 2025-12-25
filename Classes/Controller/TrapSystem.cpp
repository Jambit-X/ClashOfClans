#include "TrapSystem.h"
#include "../Layer/BattleTroopLayer.h"
#include "../Manager/VillageDataManager.h"
#include "../Model/BuildingConfig.h"
#include "../Util/GridMapUtils.h"
#include "../Sprite/BattleUnitSprite.h"

USING_NS_CC;

TrapSystem* TrapSystem::_instance = nullptr;

TrapSystem* TrapSystem::getInstance() {
    if (!_instance) {
        _instance = new TrapSystem();
    }
    return _instance;
}

void TrapSystem::destroyInstance() {
    if (_instance) {
        delete _instance;
        _instance = nullptr;
    }
}

void TrapSystem::reset() {
    _triggeredTraps.clear();
    _trapTimers.clear();
}

// ==========================================
// 检查兵种是否在陷阱范围内
// ==========================================

bool TrapSystem::isUnitInTrapRange(const BuildingInstance& trap, BattleUnitSprite* unit) {
    if (!unit || unit->isDead()) return false;
    
    // 获取兵种网格位置
    Vec2 unitGridPos = unit->getGridPosition();
    int unitGridX = static_cast<int>(std::floor(unitGridPos.x));
    int unitGridY = static_cast<int>(std::floor(unitGridPos.y));
    
    int trapX = trap.gridX;
    int trapY = trap.gridY;
    
    // 401: 炸弹 - 1x1 格子，只检查陷阱所在的格子
    if (trap.type == 401) {
        return (unitGridX == trapX && unitGridY == trapY);
    }
    // 404: 巨型炸弹 - 2x2 格子，检查陷阱所在的4个格子
    else if (trap.type == 404) {
        // 巨型炸弹占据 (trapX, trapY) 到 (trapX+1, trapY+1) 的范围
        return (unitGridX >= trapX && unitGridX <= trapX + 1 &&
                unitGridY >= trapY && unitGridY <= trapY + 1);
    }
    
    return false;
}

// ==========================================
// 更新陷阱检测（每帧调用）
// ==========================================

void TrapSystem::updateTrapDetection(BattleTroopLayer* troopLayer) {
    if (!troopLayer) return;
    
    auto dataManager = VillageDataManager::getInstance();
    auto& buildings = const_cast<std::vector<BuildingInstance>&>(dataManager->getAllBuildings());
    float deltaTime = Director::getInstance()->getDeltaTime();
    
    auto allUnits = troopLayer->getAllUnits();
    if (allUnits.empty()) return;
    
    // 遍历所有陷阱
    for (auto& building : buildings) {
        // 只处理陷阱（401: 炸弹, 404: 巨型炸弹）
        if (building.type != 401 && building.type != 404) continue;
        
        // 跳过已摧毁或已触发的陷阱
        if (building.isDestroyed || building.currentHP <= 0) continue;
        
        int trapId = building.id;
        
        // 检查陷阱是否已经被触发（正在倒计时）
        if (_triggeredTraps.find(trapId) != _triggeredTraps.end()) {
            // 更新计时器
            _trapTimers[trapId] -= deltaTime;
            
            if (_trapTimers[trapId] <= 0.0f) {
                // 时间到，执行爆炸
                CCLOG("TrapSystem: Trap %d exploding!", trapId);
                explodeTrap(&building, troopLayer);
                
                // 清除触发状态
                _triggeredTraps.erase(trapId);
                _trapTimers.erase(trapId);
            }
            continue;
        }
        
        // 检查是否有兵种进入陷阱范围
        for (auto unit : allUnits) {
            if (!unit || unit->isDead()) continue;
            
            // 气球兵是飞行单位，不会触发地面陷阱
            if (unit->getUnitTypeID() == UnitTypeID::BALLOON) continue;
            
            if (isUnitInTrapRange(building, unit)) {
                // 触发陷阱，开始0.5秒倒计时
                CCLOG("TrapSystem: Trap %d (type=%d) triggered by unit at grid(%d, %d)!",
                      trapId, building.type,
                      static_cast<int>(unit->getGridPosition().x),
                      static_cast<int>(unit->getGridPosition().y));
                
                _triggeredTraps.insert(trapId);
                _trapTimers[trapId] = 0.5f;  // 0.5秒延迟
                
                // 让陷阱显示出来
                auto mapLayer = troopLayer->getParent();
                if (mapLayer) {
                    std::string spriteName = "Building_" + std::to_string(trapId);
                    auto trapSprite = mapLayer->getChildByName(spriteName);
                    if (trapSprite) {
                        trapSprite->setVisible(true);
                        CCLOG("TrapSystem: Trap %d now VISIBLE!", trapId);
                    }
                }
                
                break;  // 一个陷阱只能被触发一次
            }
        }
    }
}

// ==========================================
// 执行陷阱爆炸
// ==========================================

void TrapSystem::explodeTrap(BuildingInstance* trap, BattleTroopLayer* troopLayer) {
    if (!trap || !troopLayer) return;
    
    auto config = BuildingConfig::getInstance()->getConfig(trap->type);
    if (!config) return;
    
    int damage = config->damagePerSecond;  // 对于陷阱，这个字段存储爆炸伤害
    
    CCLOG("TrapSystem: Trap %d (type=%d) exploding with %d damage!",
          trap->id, trap->type, damage);
    
    // 获取所有在范围内的兵种
    auto allUnits = troopLayer->getAllUnits();
    std::vector<BattleUnitSprite*> affectedUnits;
    
    for (auto unit : allUnits) {
        if (!unit || unit->isDead()) continue;
        
        // 气球兵是飞行单位，不会受到地面陷阱伤害
        if (unit->getUnitTypeID() == UnitTypeID::BALLOON) continue;
        
        if (isUnitInTrapRange(*trap, unit)) {
            affectedUnits.push_back(unit);
        }
    }
    
    CCLOG("TrapSystem: %zu units affected by trap explosion", affectedUnits.size());
    
    // 对范围内的所有兵种造成伤害
    for (auto unit : affectedUnits) {
        unit->takeDamage(damage);
        CCLOG("TrapSystem: Unit %s took %d damage from trap, HP: %d",
              unit->getUnitType().c_str(), damage, unit->getCurrentHP());
        
        // 检查是否死亡
        if (unit->isDead()) {
            unit->stopAllActions();
            unit->playDeathAnimation([troopLayer, unit]() {
                troopLayer->removeUnit(unit);
            });
        }
    }
    
    // 播放爆炸特效
    Vec2 trapPixelPos = GridMapUtils::gridToPixelCenter(trap->gridX, trap->gridY);
    
    // 对于巨型炸弹，爆炸位置在2x2的中心
    if (trap->type == 404) {
        trapPixelPos = GridMapUtils::gridToPixelCenter(trap->gridX, trap->gridY + 1);
    }
    
    auto explosion = ParticleExplosion::create();
    explosion->setPosition(trapPixelPos);
    explosion->setDuration(0.3f);
    
    // 巨型炸弹爆炸更大
    float scale = (trap->type == 404) ? 0.6f : 0.3f;
    explosion->setScale(scale);
    explosion->setAutoRemoveOnFinish(true);
    troopLayer->getParent()->addChild(explosion, 1000);
    
    // 标记陷阱为已摧毁（消失）
    trap->isDestroyed = true;
    trap->currentHP = 0;
    
    // 发送陷阱摧毁事件（用于隐藏精灵）
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(
        "EVENT_BUILDING_DESTROYED",
        static_cast<void*>(trap)
    );
    
    CCLOG("TrapSystem: Trap %d destroyed after explosion", trap->id);
}
