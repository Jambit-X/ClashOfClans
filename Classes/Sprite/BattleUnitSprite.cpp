// BattleUnitSprite.cpp
// 战斗单位精灵类实现，负责战斗单位的动画、移动、攻击和生命值管理

#include "BattleUnitSprite.h"
#include "Util/GridMapUtils.h"
#include "Util/FindPathUtil.h"
#include "Model/TroopConfig.h"
#include "Manager/AnimationManager.h"
#include <algorithm>
#include <cmath>

USING_NS_CC;

BattleUnitSprite* BattleUnitSprite::create(const std::string& unitType) {
  auto sprite = new (std::nothrow) BattleUnitSprite();
  if (sprite && sprite->init(unitType)) {
    sprite->autorelease();
    return sprite;
  }
  CC_SAFE_DELETE(sprite);
  return nullptr;
}

bool BattleUnitSprite::init(const std::string& unitType) {
    _unitType = unitType;
    _unitTypeID = parseUnitType(unitType);
    _currentAnimation = AnimationType::IDLE;
    _isAnimating = false;
    _currentGridPos = Vec2::ZERO;

    auto troopConfig = TroopConfig::getInstance();

    // 根据兵种类型映射troopID
    int troopID = 0;
    switch (_unitTypeID) {
        case UnitTypeID::BARBARIAN:    troopID = 1001; break;
        case UnitTypeID::ARCHER:       troopID = 1002; break;
        case UnitTypeID::GOBLIN:       troopID = 1003; break;
        case UnitTypeID::GIANT:        troopID = 1004; break;
        case UnitTypeID::WALL_BREAKER: troopID = 1005; break;
        case UnitTypeID::BALLOON:      troopID = 1006; break;
        default:
            CCLOG("BattleUnitSprite: Unknown unit type, defaulting to Barbarian HP");
            troopID = 1001;
            break;
    }

    // 从配置获取生命值
    TroopInfo troopInfo = troopConfig->getTroopById(troopID);
    _maxHP = troopInfo.hitpoints;
    _currentHP = _maxHP;

    CCLOG("BattleUnitSprite: Initialized %s with HP: %d/%d",
          unitType.c_str(), _currentHP, _maxHP);

    bool success = false;
    if (_unitTypeID == UnitTypeID::BALLOON) {
        // 气球兵使用独立图片文件
        success = Sprite::initWithFile("Animation/troop/balloon/balloon.png");

        if (!success) {
            CCLOG("BattleUnitSprite: Failed to load balloon image, trying fallback...");
            success = Sprite::initWithFile("Animation/troop/balloon/balloon1.0.png");
        }

        if (success) {
            CCLOG("BattleUnitSprite: Loaded balloon from independent image file");
        }
    } else {
        // 其他兵种从精灵帧缓存加载
        std::string unitTypeLower = unitType;
        std::transform(unitTypeLower.begin(), unitTypeLower.end(),
                       unitTypeLower.begin(), ::tolower);

        std::string firstFrameName = unitTypeLower + "1.0.png";
        success = Sprite::initWithSpriteFrameName(firstFrameName);
    }

    if (!success) {
        CCLOG("BattleUnitSprite: Failed to load sprite for %s", unitType.c_str());
        return false;
    }

    this->setAnchorPoint(Vec2(0.5f, 0.0f));

    if (_unitTypeID == UnitTypeID::BALLOON) {
        // 气球兵添加上下飘动动画
        auto floatUp = MoveBy::create(1.0f, Vec2(0, 10));
        auto floatDown = MoveBy::create(1.0f, Vec2(0, -10));
        auto floatSequence = Sequence::create(floatUp, floatDown, nullptr);
        auto floatForever = RepeatForever::create(floatSequence);
        floatForever->setTag(9999);
        this->runAction(floatForever);

        CCLOG("BattleUnitSprite: Balloon floating animation started");
    }

    float scale = getScaleForUnitType(_unitTypeID);
    this->setScale(scale);
    CCLOG("BattleUnitSprite: Set scale to %.2f for %s", scale, unitType.c_str());

    this->scheduleUpdate();

    CCLOG("BattleUnitSprite: Created %s (TypeID=%d)", unitType.c_str(), static_cast<int>(_unitTypeID));
    return true;
}

float BattleUnitSprite::getScaleForUnitType(UnitTypeID typeID) {
    switch (typeID) {
        case UnitTypeID::BARBARIAN:    return 0.8f;
        case UnitTypeID::ARCHER:       return 0.75f;
        case UnitTypeID::GOBLIN:       return 0.7f;
        case UnitTypeID::GIANT:        return 1.2f;
        case UnitTypeID::WALL_BREAKER: return 0.65f;
        case UnitTypeID::BALLOON:      return 1.0f;
        default:                       return 1.0f;
    }
}

void BattleUnitSprite::update(float dt) {
    Sprite::update(dt);
    
    // 获取当前网格位置
    Vec2 currentPos = this->getPosition();
    Vec2 gridPos = GridMapUtils::pixelToGrid(currentPos);
    int currentGridX = static_cast<int>(std::floor(gridPos.x));
    int currentGridY = static_cast<int>(std::floor(gridPos.y));
    
    if (currentGridX != _lastGridX || currentGridY != _lastGridY) {
        _currentGridPos = gridPos;
        _lastGridX = currentGridX;
        _lastGridY = currentGridY;
        
        // 根据网格位置更新Z轴顺序以实现正确的深度渲染
        int zOrder = GridMapUtils::calculateZOrder(currentGridX, currentGridY);
        
        if (_unitTypeID == UnitTypeID::BALLOON) {
            zOrder += 1000;
        }
        
        this->setLocalZOrder(zOrder);
    }
}

void BattleUnitSprite::setTargetedByBuilding(bool targeted) {
    if (this->isDead()) {
        // 死亡单位拒绝被锁定
        if (_isTargetedByBuilding) {
            _isTargetedByBuilding = false;
            this->setColor(Color3B::WHITE);
            CCLOG("BattleUnitSprite: Dead unit refusing targeting, color reset to WHITE");
        }
        return;
    }

    if (_isTargetedByBuilding == targeted) return;

    _isTargetedByBuilding = targeted;

    if (targeted) {
        this->setColor(Color3B(255, 100, 100));
    } else {
        this->setColor(Color3B::WHITE);
    }
}

UnitTypeID BattleUnitSprite::parseUnitType(const std::string& unitType) {
    std::string lower = unitType;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (lower.find("barbarian") != std::string::npos || lower.find("野蛮人") != std::string::npos) {
        return UnitTypeID::BARBARIAN;
    }
    if (lower.find("archer") != std::string::npos || lower.find("弓箭手") != std::string::npos) {
        return UnitTypeID::ARCHER;
    }
    if (lower.find("goblin") != std::string::npos || lower.find("哥布林") != std::string::npos) {
        return UnitTypeID::GOBLIN;
    }
    if (lower.find("giant") != std::string::npos || lower.find("巨人") != std::string::npos) {
        return UnitTypeID::GIANT;
    }
    if (lower.find("wall_breaker") != std::string::npos || lower.find("wallbreaker") != std::string::npos || lower.find("炸弹人") != std::string::npos) {
        return UnitTypeID::WALL_BREAKER;
    }
    if (lower.find("balloon") != std::string::npos || lower.find("气球") != std::string::npos) {
        return UnitTypeID::BALLOON;
    }
    
    CCLOG("BattleUnitSprite::parseUnitType: Unknown unit type '%s'", unitType.c_str());
    return UnitTypeID::UNKNOWN;
}
void BattleUnitSprite::playAnimation(
    AnimationType animType,
    bool loop,
    const std::function<void()>& callback
) {
    stopCurrentAnimation();

    if (_unitTypeID == UnitTypeID::BALLOON) {
        // 气球兵无帧动画，使用静态图片和延迟回调
        _currentAnimation = animType;
        _isAnimating = false;

        if (animType == AnimationType::DEATH) {
            auto tombstoneTexture = Director::getInstance()->getTextureCache()->addImage(
                "Animation/troop/balloon/balloon_death.png"
            );

            if (tombstoneTexture) {
                this->setTexture(tombstoneTexture);
                auto rect = Rect(0, 0, tombstoneTexture->getContentSize().width,
                                 tombstoneTexture->getContentSize().height);
                this->setTextureRect(rect);
                CCLOG("BattleUnitSprite: Balloon switched to tombstone texture");
            }

            if (callback) {
                this->runAction(Sequence::create(
                    DelayTime::create(0.5f),
                    CallFunc::create(callback),
                    nullptr
                ));
            }
        } else if (animType == AnimationType::ATTACK || 
                   animType == AnimationType::ATTACK_UP || 
                   animType == AnimationType::ATTACK_DOWN) {
            // 攻击动画添加延迟模拟投弹
            if (callback) {
                this->runAction(Sequence::create(
                    DelayTime::create(0.8f),
                    CallFunc::create(callback),
                    nullptr
                ));
            }
        } else {
            if (callback) {
                callback();
            }
        }
        return;
    }

    // 其他兵种使用AnimationManager创建帧动画
    auto animMgr = AnimationManager::getInstance();

    _currentAnimation = animType;
    _isAnimating = true;

    if (loop) {
        auto action = animMgr->createLoopAnimate(_unitType, animType);
        if (!action) {
            CCLOG("BattleUnitSprite: Failed to create loop animation");
            _isAnimating = false;
            return;
        }
        action->setTag(ANIMATION_TAG);
        this->runAction(action);
    } else {
        auto animate = animMgr->createOnceAnimate(_unitType, animType);
        if (!animate) {
            CCLOG("BattleUnitSprite: Failed to create animation");
            _isAnimating = false;
            return;
        }

        if (callback) {
            auto callbackFunc = CallFunc::create([this, callback]() {
                _isAnimating = false;
                callback();
            });
            auto seq = Sequence::create(animate, callbackFunc, nullptr);
            seq->setTag(ANIMATION_TAG);
            this->runAction(seq);
        } else {
            animate->setTag(ANIMATION_TAG);
            this->runAction(animate);
        }
    }
}

void BattleUnitSprite::stopCurrentAnimation() {
  this->stopActionByTag(ANIMATION_TAG);
  _isAnimating = false;
}

void BattleUnitSprite::playIdleAnimation() {
    AnimationType idleAnimType = AnimationType::IDLE;
    bool flipX = false;

    if (_lastMoveDirection != Vec2::ZERO) {
        // 根据上次移动方向选择待机动画朝向
        float angle = getAngleFromDirection(_lastMoveDirection);

        while (angle < 0) angle += 360;
        while (angle >= 360) angle -= 360;

        // 根据角度区间选择动画类型和翻转
        if (angle >= 337.5f || angle < 22.5f) {
            idleAnimType = AnimationType::IDLE;
            flipX = false;
        } else if (angle >= 22.5f && angle < 67.5f) {
            idleAnimType = AnimationType::IDLE_UP;
            flipX = false;
        } else if (angle >= 67.5f && angle < 112.5f) {
            idleAnimType = AnimationType::IDLE_UP;
            flipX = false;
        } else if (angle >= 112.5f && angle < 157.5f) {
            idleAnimType = AnimationType::IDLE_UP;
            flipX = true;
        } else if (angle >= 157.5f && angle < 202.5f) {
            idleAnimType = AnimationType::IDLE;
            flipX = true;
        } else if (angle >= 202.5f && angle < 247.5f) {
            idleAnimType = AnimationType::IDLE_DOWN;
            flipX = true;
        } else if (angle >= 247.5f && angle < 292.5f) {
            idleAnimType = AnimationType::IDLE_DOWN;
            flipX = false;
        } else {
            idleAnimType = AnimationType::IDLE_DOWN;
            flipX = false;
        }
    }

    this->setFlippedX(flipX);
    playAnimation(idleAnimType, true);
}

void BattleUnitSprite::playWalkAnimation() {
  playAnimation(AnimationType::WALK, true);
}

void BattleUnitSprite::playAttackAnimation(const std::function<void()>& callback) {
  playAnimation(AnimationType::ATTACK, false, callback);
}

float BattleUnitSprite::getAngleFromDirection(const Vec2& direction) {
  return CC_RADIANS_TO_DEGREES(atan2f(direction.y, direction.x));
}

void BattleUnitSprite::selectWalkAnimation(const Vec2& direction,
                                           AnimationType& outAnimType,
                                           bool& outFlipX) {
  _lastMoveDirection = direction;

  float angle = getAngleFromDirection(direction);

  // 角度归一化到[0, 360)
  while (angle < 0) angle += 360;
  while (angle >= 360) angle -= 360;

  // 八方向选择行走动画
  if (angle >= 337.5f || angle < 22.5f) {
    outAnimType = AnimationType::WALK;
    outFlipX = false;
  } else if (angle >= 22.5f && angle < 67.5f) {
    outAnimType = AnimationType::WALK_UP;
    outFlipX = false;
  } else if (angle >= 67.5f && angle < 112.5f) {
    outAnimType = AnimationType::WALK_UP;
    outFlipX = false;
  } else if (angle >= 112.5f && angle < 157.5f) {
    outAnimType = AnimationType::WALK_UP;
    outFlipX = true;
  } else if (angle >= 157.5f && angle < 202.5f) {
    outAnimType = AnimationType::WALK;
    outFlipX = true;
  } else if (angle >= 202.5f && angle < 247.5f) {
    outAnimType = AnimationType::WALK_DOWN;
    outFlipX = true;
  } else if (angle >= 247.5f && angle < 292.5f) {
    outAnimType = AnimationType::WALK_DOWN;
    outFlipX = false;
  } else {
    outAnimType = AnimationType::WALK_DOWN;
    outFlipX = false;
  }
}

void BattleUnitSprite::selectAttackAnimation(const Vec2& direction,
                                             AnimationType& outAnimType,
                                             bool& outFlipX) {
  float angle = getAngleFromDirection(direction);

  // 角度归一化到[0, 360)
  while (angle < 0) angle += 360;
  while (angle >= 360) angle -= 360;

  // 八方向选择攻击动画
  if (angle >= 337.5f || angle < 22.5f) {
    outAnimType = AnimationType::ATTACK;
    outFlipX = false;
  } else if (angle >= 22.5f && angle < 67.5f) {
    outAnimType = AnimationType::ATTACK_UP;
    outFlipX = false;
  } else if (angle >= 67.5f && angle < 112.5f) {
    outAnimType = AnimationType::ATTACK_UP;
    outFlipX = false;
  } else if (angle >= 112.5f && angle < 157.5f) {
    outAnimType = AnimationType::ATTACK_UP;
    outFlipX = true;
  } else if (angle >= 157.5f && angle < 202.5f) {
    outAnimType = AnimationType::ATTACK;
    outFlipX = true;
  } else if (angle >= 202.5f && angle < 247.5f) {
    outAnimType = AnimationType::ATTACK_DOWN;
    outFlipX = true;
  } else if (angle >= 247.5f && angle < 292.5f) {
    outAnimType = AnimationType::ATTACK_DOWN;
    outFlipX = false;
  } else {
    outAnimType = AnimationType::ATTACK_DOWN;
    outFlipX = false;
  }
}

void BattleUnitSprite::walkToPosition(const Vec2& targetPos, float duration,
                                      const std::function<void()>& callback) {
  Vec2 currentPos = this->getPosition();
  Vec2 direction = targetPos - currentPos;

  if (direction.length() < 0.1f) {
    CCLOG("BattleUnitSprite: Target too close, skipping walk");
    if (callback) callback();
    return;
  }

  direction.normalize();

  AnimationType animType;
  bool flipX;
  selectWalkAnimation(direction, animType, flipX);

  this->setFlippedX(flipX);
  playAnimation(animType, true);

  auto moveAction = MoveTo::create(duration, targetPos);

  auto finishCallback = CallFunc::create([this, callback]() {
    this->setFlippedX(false);
    playIdleAnimation();

    if (callback) {
      callback();
    }
  });

  auto sequence = Sequence::create(moveAction, finishCallback, nullptr);
  sequence->setTag(MOVE_TAG);
  this->runAction(sequence);
}

void BattleUnitSprite::walkByOffset(const Vec2& offset, float duration,
                                    const std::function<void()>& callback) {
  Vec2 currentPos = this->getPosition();
  Vec2 targetPos = currentPos + offset;
  walkToPosition(targetPos, duration, callback);
}

void BattleUnitSprite::setGridPosition(int gridX, int gridY) {
  _currentGridPos = Vec2(gridX, gridY);
  CCLOG("BattleUnitSprite: Grid position set to (%d, %d)", gridX, gridY);
}

void BattleUnitSprite::teleportToGrid(int gridX, int gridY) {
  if (!GridMapUtils::isValidGridPosition(gridX, gridY)) {
    CCLOG("BattleUnitSprite: Invalid grid position (%d, %d)", gridX, gridY);
    return;
  }

  _currentGridPos = Vec2(gridX, gridY);
  Vec2 pixelPos = GridMapUtils::gridToPixelCenter(gridX, gridY);
  this->setPosition(pixelPos);

  CCLOG("BattleUnitSprite: Teleported to grid(%d, %d) -> pixel(%.0f, %.0f)",
        gridX, gridY, pixelPos.x, pixelPos.y);
}

void BattleUnitSprite::walkToGrid(int targetGridX, int targetGridY, float speed,
                                  const std::function<void()>& callback) {
  if (!GridMapUtils::isValidGridPosition(targetGridX, targetGridY)) {
    CCLOG("BattleUnitSprite: Invalid target grid position (%d, %d)",
          targetGridX, targetGridY);
    if (callback) callback();
    return;
  }

  // 计算移动距离和持续时间
  Vec2 currentPixelPos = this->getPosition();
  Vec2 targetPixelPos = GridMapUtils::gridToPixelCenter(targetGridX, targetGridY);
  float distance = currentPixelPos.distance(targetPixelPos);
  float duration = distance / speed;

  CCLOG("BattleUnitSprite: Walking to grid(%d, %d), distance=%.1f, duration=%.2f",
        targetGridX, targetGridY, distance, duration);

  walkToPosition(targetPixelPos, duration, [this, targetGridX, targetGridY, callback]() {
    _currentGridPos = Vec2(targetGridX, targetGridY);
    CCLOG("BattleUnitSprite: Arrived at grid(%d, %d)", targetGridX, targetGridY);

    if (callback) {
      callback();
    }
  });
}

void BattleUnitSprite::walkFromGridToGrid(int startGridX, int startGridY,
                                          int targetGridX, int targetGridY,
                                          float speed,
                                          const std::function<void()>& callback) {
  if (!GridMapUtils::isValidGridPosition(startGridX, startGridY)) {
    CCLOG("BattleUnitSprite: Invalid start grid position (%d, %d)",
          startGridX, startGridY);
    if (callback) callback();
    return;
  }

  if (!GridMapUtils::isValidGridPosition(targetGridX, targetGridY)) {
    CCLOG("BattleUnitSprite: Invalid target grid position (%d, %d)",
          targetGridX, targetGridY);
    if (callback) callback();
    return;
  }

  // 先传送到起点，再移动到终点
  teleportToGrid(startGridX, startGridY);
  walkToGrid(targetGridX, targetGridY, speed, callback);

  CCLOG("BattleUnitSprite: Walking from grid(%d, %d) to grid(%d, %d)",
        startGridX, startGridY, targetGridX, targetGridY);
}

void BattleUnitSprite::attackInDirection(const Vec2& direction,
                                         const std::function<void()>& callback) {
  Vec2 normalizedDir = direction;

  if (normalizedDir.length() < 0.1f) {
    // 方向向量过短，默认朝右攻击
    normalizedDir = Vec2(1, 0);
    CCLOG("BattleUnitSprite: Direction too short, defaulting to right");
  } else {
    normalizedDir.normalize();
  }

  AnimationType animType;
  bool flipX;
  selectAttackAnimation(normalizedDir, animType, flipX);

  this->setFlippedX(flipX);

  CCLOG("BattleUnitSprite: Attacking in direction (%.2f, %.2f), angle=%.1f",
        normalizedDir.x, normalizedDir.y, getAngleFromDirection(normalizedDir));

  playAnimation(animType, false, [this, flipX, callback]() {
    this->setFlippedX(false);
    CCLOG("BattleUnitSprite: Attack animation completed");

    if (callback) {
      callback();
    }
  });
}

void BattleUnitSprite::attackTowardPosition(const Vec2& targetPos,
                                            const std::function<void()>& callback) {
  Vec2 currentPos = this->getPosition();
  Vec2 direction = targetPos - currentPos;

  CCLOG("BattleUnitSprite: Attacking toward position (%.0f, %.0f)",
        targetPos.x, targetPos.y);

  attackInDirection(direction, callback);
}

void BattleUnitSprite::attackTowardGrid(int targetGridX, int targetGridY,
                                        const std::function<void()>& callback) {
  if (!GridMapUtils::isValidGridPosition(targetGridX, targetGridY)) {
    CCLOG("BattleUnitSprite: Invalid target grid position (%d, %d)",
          targetGridX, targetGridY);
    if (callback) callback();
    return;
  }

  Vec2 targetPixelPos = GridMapUtils::gridToPixelCenter(targetGridX, targetGridY);

  CCLOG("BattleUnitSprite: Attacking toward grid(%d, %d)",
        targetGridX, targetGridY);

  attackTowardPosition(targetPixelPos, callback);
}

void BattleUnitSprite::moveToTargetWithPathfinding(
    const Vec2& targetWorldPos,
    float speed,
    const std::function<void()>& callback) {

    Vec2 currentWorldPos = this->getPosition();

    // 使用寻路工具查找路径
    auto pathfinder = FindPathUtil::getInstance();
    std::vector<Vec2> path = pathfinder->findPathInWorld(currentWorldPos, targetWorldPos);

    if (path.empty()) {
        CCLOG("BattleUnitSprite: No path found to target (%.0f, %.0f)",
              targetWorldPos.x, targetWorldPos.y);
        if (callback) callback();
        return;
    }

    CCLOG("BattleUnitSprite: Path found with %lu waypoints", path.size());

    followPath(path, speed, callback);
}

void BattleUnitSprite::moveToGridWithPathfinding(
    int targetGridX,
    int targetGridY,
    float speed,
    const std::function<void()>& callback) {

    if (!GridMapUtils::isValidGridPosition(targetGridX, targetGridY)) {
        CCLOG("BattleUnitSprite: Invalid target grid (%d, %d)", targetGridX, targetGridY);
        if (callback) callback();
        return;
    }

    // 网格坐标转换为世界坐标后寻路
    Vec2 targetWorldPos = GridMapUtils::gridToPixelCenter(targetGridX, targetGridY);

    moveToTargetWithPathfinding(targetWorldPos, speed, callback);
}

void BattleUnitSprite::followPath(
    const std::vector<Vec2>& path,
    float speed,
    const std::function<void()>& callback) {

    if (path.empty()) {
        CCLOG("BattleUnitSprite: Empty path, nothing to follow");
        if (callback) callback();
        return;
    }

    this->stopActionByTag(MOVE_TAG);

    Vector<FiniteTimeAction*> actions;

    Vec2 currentPos = this->getPosition();
    bool hasValidMovement = false;

    // 遍历路径点创建移动动画序列
    for (const auto& waypoint : path) {
        Vec2 direction = waypoint - currentPos;
        float distance = direction.length();

        if (distance < 0.1f) {
            continue;
        }

        hasValidMovement = true;
        direction.normalize();

        AnimationType animType;
        bool flipX;
        selectWalkAnimation(direction, animType, flipX);

        // 为每个路径点创建动画和移动动作
        auto playAnim = CallFunc::create([this, animType, flipX]() {
            this->setFlippedX(flipX);
            playAnimation(animType, true);
        });

        float duration = distance / speed;
        auto moveAction = MoveTo::create(duration, waypoint);

        actions.pushBack(playAnim);
        actions.pushBack(moveAction);

        currentPos = waypoint;
    }

    if (!hasValidMovement) {
        CCLOG("BattleUnitSprite: No valid movement, adding minimum delay");
        actions.pushBack(DelayTime::create(0.1f));
    }

    // 路径完成后恢复待机状态
    auto finishCallback = CallFunc::create([this, callback]() {
        this->setFlippedX(false);
        playIdleAnimation();

        CCLOG("BattleUnitSprite: Path completed");

        if (callback) {
            callback();
        }
    });

    actions.pushBack(finishCallback);

    auto sequence = Sequence::create(actions);
    sequence->setTag(MOVE_TAG);
    this->runAction(sequence);

    CCLOG("BattleUnitSprite: Following path with %lu waypoints", path.size());
}

void BattleUnitSprite::takeDamage(int damage) {
    if (_currentHP <= 0) return;

    _currentHP -= damage;
    if (_currentHP < 0) _currentHP = 0;

    updateHealthBar();

    CCLOG("BattleUnitSprite: %s took %d damage, HP: %d/%d",
          _unitType.c_str(), damage, _currentHP, _maxHP);
}

void BattleUnitSprite::updateHealthBar() {
    if (!_healthBar) {
        // 首次创建血条，根据兵种类型设置宽度
        HealthBarComponent::Config barConfig;

        switch (_unitTypeID) {
            case UnitTypeID::GOBLIN:
            case UnitTypeID::WALL_BREAKER:
                barConfig.width = 30.0f;
                break;
            case UnitTypeID::BARBARIAN:
            case UnitTypeID::ARCHER:
                barConfig.width = 40.0f;
                break;
            case UnitTypeID::GIANT:
            case UnitTypeID::BALLOON:
                barConfig.width = 60.0f;
                break;
            default:
                barConfig.width = 40.0f;
                break;
        }

        barConfig.height = 6.0f;
        barConfig.offset = Vec2(0, 10);
        barConfig.highThreshold = 60.0f;
        barConfig.mediumThreshold = 30.0f;
        barConfig.showWhenFull = false;
        barConfig.fadeInDuration = 0.2f;

        _healthBar = HealthBarComponent::create(barConfig);
        this->addChild(_healthBar, 100);

        _healthBar->updatePosition(this->getContentSize());
    }

    _healthBar->updateHealth(_currentHP, _maxHP);
}

void BattleUnitSprite::playDeathAnimation(const std::function<void()>& callback) {
    // 重置颜色和不透明度
    this->setColor(Color3B::WHITE);
    this->setOpacity(255);

    this->setTargetedByBuilding(false);

    if (_healthBar) {
        _healthBar->hide();
    }

    CCLOG("BattleUnitSprite: Death animation started, color reset to WHITE, targeting cleared");

    playAnimation(AnimationType::DEATH, false, callback);
}
