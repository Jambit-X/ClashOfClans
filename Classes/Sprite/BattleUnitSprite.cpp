// BattleUnitSprite.cpp
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

    // 初始化生命值
    auto troopConfig = TroopConfig::getInstance();

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

    TroopInfo troopInfo = troopConfig->getTroopById(troopID);
    _maxHP = troopInfo.hitpoints;
    _currentHP = _maxHP;

    CCLOG("BattleUnitSprite: Initialized %s with HP: %d/%d",
          unitType.c_str(), _currentHP, _maxHP);

    // ========== ✅ 气球兵特殊处理:直接加载独立图片 ==========
    bool success = false;
    if (_unitTypeID == UnitTypeID::BALLOON) {
        // 气球兵使用独立图片,不使用精灵图集
        success = Sprite::initWithFile("Animation/troop/balloon/balloon.png");

        if (!success) {
            CCLOG("BattleUnitSprite: Failed to load balloon image, trying fallback...");
            // 尝试备用路径
            success = Sprite::initWithFile("Animation/troop/balloon/balloon1.0.png");
        }

        if (success) {
            CCLOG("BattleUnitSprite: Loaded balloon from independent image file");
        }
    } else {
        // 其他兵种使用精灵图集
        std::string unitTypeLower = unitType;
        std::transform(unitTypeLower.begin(), unitTypeLower.end(),
                       unitTypeLower.begin(), ::tolower);

        std::string firstFrameName = unitTypeLower + "1.0.png";
        success = Sprite::initWithSpriteFrameName(firstFrameName);
    }
    // ========================================================

    if (!success) {
        CCLOG("BattleUnitSprite: Failed to load sprite for %s", unitType.c_str());
        return false;
    }

    this->setAnchorPoint(Vec2(0.5f, 0.0f));

    // ========== ✅ 气球兵特殊效果:上下飘动 ==========
    if (_unitTypeID == UnitTypeID::BALLOON) {
        auto floatUp = MoveBy::create(1.0f, Vec2(0, 10));   // 向上飘 10 像素
        auto floatDown = MoveBy::create(1.0f, Vec2(0, -10)); // 向下飘 10 像素
        auto floatSequence = Sequence::create(floatUp, floatDown, nullptr);
        auto floatForever = RepeatForever::create(floatSequence);
        floatForever->setTag(9999); // 使用特殊 Tag,避免与移动动作冲突
        this->runAction(floatForever);

        CCLOG("BattleUnitSprite: Balloon floating animation started");
    }

    // 根据兵种类型设置缩放比例
    float scale = getScaleForUnitType(_unitTypeID);
    this->setScale(scale);
    CCLOG("BattleUnitSprite: Set scale to %.2f for %s", scale, unitType.c_str());


    this->scheduleUpdate();

    CCLOG("BattleUnitSprite: Created %s (TypeID=%d)", unitType.c_str(), static_cast<int>(_unitTypeID));
    return true;
}

// ========== ✅ 新增：根据兵种类型返回缩放比例 ==========
float BattleUnitSprite::getScaleForUnitType(UnitTypeID typeID) {
    switch (typeID) {
        case UnitTypeID::BARBARIAN:
            return 0.8f;  // 野蛮人：缩小到80%
        case UnitTypeID::ARCHER:
            return 0.75f; // 弓箭手：缩小到75%
        case UnitTypeID::GOBLIN:
            return 0.7f;  // 哥布林：缩小到70%（体型较小）
        case UnitTypeID::GIANT:
            return 1.2f;  // 巨人：放大到120%（体型较大）
        case UnitTypeID::WALL_BREAKER:
            return 0.65f; // 炸弹兵：缩小到65%（体型很小）
        case UnitTypeID::BALLOON:
            return 1.0f;  // 气球兵：保持原始大小
        default:
            return 1.0f;  // 默认：不缩放
    }
}

void BattleUnitSprite::update(float dt) {
    Sprite::update(dt);
    
    Vec2 currentPos = this->getPosition();
    Vec2 gridPos = GridMapUtils::pixelToGrid(currentPos);
    int currentGridX = static_cast<int>(std::floor(gridPos.x));
    int currentGridY = static_cast<int>(std::floor(gridPos.y));
    
    if (currentGridX != _lastGridX || currentGridY != _lastGridY) {
        _currentGridPos = gridPos;
        _lastGridX = currentGridX;
        _lastGridY = currentGridY;
        
        // ✅ 动态更新 Z-Order，确保移动时透视正确
        int zOrder = GridMapUtils::calculateZOrder(currentGridX, currentGridY);
        
        // 飞行单位（气球兵）额外加 1000 偏移
        if (_unitTypeID == UnitTypeID::BALLOON) {
            zOrder += 1000;
        }
        
        this->setLocalZOrder(zOrder);
    }
}

// ========== 建筑锁定状态可视化 ==========
void BattleUnitSprite::setTargetedByBuilding(bool targeted) {
    // ✅ 如果兵种已死亡，拒绝接受锁定
    if (this->isDead()) {
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
        this->setColor(Color3B(255, 100, 100));  // 红色高亮
    } else {
        this->setColor(Color3B::WHITE);
    }
}

// ========== 静态方法：解析单位类型字符串为枚举（只在初始化时调用一次）==========
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

// ===== 基础动画控制 =====

void BattleUnitSprite::playAnimation(
    AnimationType animType,
    bool loop,
    const std::function<void()>& callback
) {
    stopCurrentAnimation();

    // ========== ✅ 气球兵特殊处理:没有帧动画,直接返回 ==========
    if (_unitTypeID == UnitTypeID::BALLOON) {
        _currentAnimation = animType;
        _isAnimating = false;

        // 如果是死亡动画,切换到墓碑图片
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

            // 延迟触发回调
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
            // ✅ 所有方向的攻击动画都需要延迟，模拟投弹时间
            if (callback) {
                this->runAction(Sequence::create(
                    DelayTime::create(0.8f),  // 0.8 秒攻击间隔
                    CallFunc::create(callback),
                    nullptr
                ));
            }
        } else {
            // 其他动画(待机/行走)不需要做任何事,直接触发回调
            if (callback) {
                callback();
            }
        }
        return;
    }
    // ============================================================

    // 其他兵种的正常动画逻辑
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

// ✅ 修改：根据上次移动方向选择待机动画
// ========== 修复：待机动画也支持镜像翻转 ==========
void BattleUnitSprite::playIdleAnimation() {
    AnimationType idleAnimType = AnimationType::IDLE;  // 默认向右
    bool flipX = false;

    // 如果有上次移动方向记录,根据方向选择待机动画
    if (_lastMoveDirection != Vec2::ZERO) {
        float angle = getAngleFromDirection(_lastMoveDirection);

        // 归一化角度到 [0, 360)
        while (angle < 0) angle += 360;
        while (angle >= 360) angle -= 360;

        // ========== 8 方向判断（与行走动画保持一致）==========
        if (angle >= 337.5f || angle < 22.5f) {
            // 向右 (0°)
            idleAnimType = AnimationType::IDLE;
            flipX = false;
        } else if (angle >= 22.5f && angle < 67.5f) {
            // 向右上 (45°)
            idleAnimType = AnimationType::IDLE_UP;
            flipX = false;
        } else if (angle >= 67.5f && angle < 112.5f) {
            // 向上 (90°) - 使用右上动画，不翻转
            idleAnimType = AnimationType::IDLE_UP;
            flipX = false;
        } else if (angle >= 112.5f && angle < 157.5f) {
            // ✅ 向左上 (135°) - 使用右上动画 + 镜像翻转
            idleAnimType = AnimationType::IDLE_UP;
            flipX = true;
        } else if (angle >= 157.5f && angle < 202.5f) {
            // ✅ 向左 (180°) - 使用向右动画 + 镜像翻转
            idleAnimType = AnimationType::IDLE;
            flipX = true;
        } else if (angle >= 202.5f && angle < 247.5f) {
            // ✅ 向左下 (225°) - 使用右下动画 + 镜像翻转
            idleAnimType = AnimationType::IDLE_DOWN;
            flipX = true;
        } else if (angle >= 247.5f && angle < 292.5f) {
            // 向下 (270°) - 使用右下动画，不翻转
            idleAnimType = AnimationType::IDLE_DOWN;
            flipX = false;
        } else {
            // 向右下 (315°)
            idleAnimType = AnimationType::IDLE_DOWN;
            flipX = false;
        }
    }

    // ✅ 应用镜像翻转
    this->setFlippedX(flipX);
    playAnimation(idleAnimType, true);
}

void BattleUnitSprite::playWalkAnimation() {
  playAnimation(AnimationType::WALK, true);
}

void BattleUnitSprite::playAttackAnimation(const std::function<void()>& callback) {
  playAnimation(AnimationType::ATTACK, false, callback);
}

// ===== 方向计算 =====

float BattleUnitSprite::getAngleFromDirection(const Vec2& direction) {
  return CC_RADIANS_TO_DEGREES(atan2f(direction.y, direction.x));
}

void BattleUnitSprite::selectWalkAnimation(const Vec2& direction,
                                           AnimationType& outAnimType,
                                           bool& outFlipX) {
  // 记录移动方向
  _lastMoveDirection = direction;

  float angle = getAngleFromDirection(direction);

  while (angle < 0) angle += 360;
  while (angle >= 360) angle -= 360;

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

  while (angle < 0) angle += 360;
  while (angle >= 360) angle -= 360;

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

// ===== 像素坐标行走 =====

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

// ===== 网格坐标行走 =====

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

  teleportToGrid(startGridX, startGridY);
  walkToGrid(targetGridX, targetGridY, speed, callback);

  CCLOG("BattleUnitSprite: Walking from grid(%d, %d) to grid(%d, %d)",
        startGridX, startGridY, targetGridX, targetGridY);
}

// ===== 方向攻击（站定不动）=====

void BattleUnitSprite::attackInDirection(const Vec2& direction,
                                         const std::function<void()>& callback) {
  Vec2 normalizedDir = direction;

  if (normalizedDir.length() < 0.1f) {
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

// ===== ? 新增：寻路移动 =====

void BattleUnitSprite::moveToTargetWithPathfinding(
    const Vec2& targetWorldPos,
    float speed,
    const std::function<void()>& callback) {

    // 1. 获取当前世界坐标
    Vec2 currentWorldPos = this->getPosition();

    // 2. 使用寻路工具查找路径
    auto pathfinder = FindPathUtil::getInstance();
    std::vector<Vec2> path = pathfinder->findPathInWorld(currentWorldPos, targetWorldPos);

    if (path.empty()) {
        CCLOG("BattleUnitSprite: No path found to target (%.0f, %.0f)",
              targetWorldPos.x, targetWorldPos.y);
        if (callback) callback();
        return;
    }

    CCLOG("BattleUnitSprite: Path found with %lu waypoints", path.size());

    // 3. 沿路径移动
    followPath(path, speed, callback);
}

void BattleUnitSprite::moveToGridWithPathfinding(
    int targetGridX,
    int targetGridY,
    float speed,
    const std::function<void()>& callback) {

    // 1. 检查目标合法性
    if (!GridMapUtils::isValidGridPosition(targetGridX, targetGridY)) {
        CCLOG("BattleUnitSprite: Invalid target grid (%d, %d)", targetGridX, targetGridY);
        if (callback) callback();
        return;
    }

    // 2. 转换为世界坐标
    Vec2 targetWorldPos = GridMapUtils::gridToPixelCenter(targetGridX, targetGridY);

    // 3. 寻路并移动
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

    // 停止当前移动
    this->stopActionByTag(MOVE_TAG);

    // 创建动作序列
    Vector<FiniteTimeAction*> actions;

    Vec2 currentPos = this->getPosition();
    bool hasValidMovement = false;  // ✅ 追踪是否有有效移动

    for (const auto& waypoint : path) {
        // 计算方向和距离
        Vec2 direction = waypoint - currentPos;
        float distance = direction.length();

        if (distance < 0.1f) {
            continue; // 跳过太近的点
        }

        hasValidMovement = true;  // ✅ 标记有有效移动
        direction.normalize();

        // 选择动画
        AnimationType animType;
        bool flipX;
        selectWalkAnimation(direction, animType, flipX);

        // 播放行走动画
        auto playAnim = CallFunc::create([this, animType, flipX]() {
            this->setFlippedX(flipX);
            playAnimation(animType, true);
        });

        // 移动到路径点
        float duration = distance / speed;
        auto moveAction = MoveTo::create(duration, waypoint);

        actions.pushBack(playAnim);
        actions.pushBack(moveAction);

        currentPos = waypoint;
    }

    // ✅ 如果没有有效移动，添加最小延迟防止立即回调导致秒杀
    if (!hasValidMovement) {
        CCLOG("BattleUnitSprite: No valid movement, adding minimum delay");
        actions.pushBack(DelayTime::create(0.1f));
    }

    // 到达后播放待机动画
    auto finishCallback = CallFunc::create([this, callback]() {
        this->setFlippedX(false);
        playIdleAnimation();

        CCLOG("BattleUnitSprite: Path completed");

        if (callback) {
            callback();
        }
    });

    actions.pushBack(finishCallback);

    // 执行动作序列
    auto sequence = Sequence::create(actions);
    sequence->setTag(MOVE_TAG);
    this->runAction(sequence);

    CCLOG("BattleUnitSprite: Following path with %lu waypoints", path.size());
}

// ===== 生命值系统 =====

void BattleUnitSprite::takeDamage(int damage) {
    if (_currentHP <= 0) return;

    _currentHP -= damage;
    if (_currentHP < 0) _currentHP = 0;

    // ✅ 更新血条
    updateHealthBar();

    CCLOG("BattleUnitSprite: %s took %d damage, HP: %d/%d",
          _unitType.c_str(), damage, _currentHP, _maxHP);
}

void BattleUnitSprite::updateHealthBar() {
    // ✅ 惰性创建血条组件
    if (!_healthBar) {
        // 根据兵种大小配置血条
        HealthBarComponent::Config barConfig;

        switch (_unitTypeID) {
            case UnitTypeID::GOBLIN:
            case UnitTypeID::WALL_BREAKER:
                barConfig.width = 30.0f;  // 小型兵种
                break;
            case UnitTypeID::BARBARIAN:
            case UnitTypeID::ARCHER:
                barConfig.width = 40.0f;  // 中型兵种
                break;
            case UnitTypeID::GIANT:
            case UnitTypeID::BALLOON:
                barConfig.width = 60.0f;  // 大型兵种
                break;
            default:
                barConfig.width = 40.0f;
                break;
        }

        barConfig.height = 6.0f;
        barConfig.offset = Vec2(0, 10);  // 兵种头顶上方10像素
        barConfig.highThreshold = 60.0f;
        barConfig.mediumThreshold = 30.0f;
        barConfig.showWhenFull = false;
        barConfig.fadeInDuration = 0.2f;

        // 创建血条组件
        _healthBar = HealthBarComponent::create(barConfig);
        this->addChild(_healthBar, 100);

        // 更新血条位置
        _healthBar->updatePosition(this->getContentSize());
    }

    // 更新血条
    _healthBar->updateHealth(_currentHP, _maxHP);
}

void BattleUnitSprite::playDeathAnimation(const std::function<void()>& callback) {
    // ✅ 第一时间恢复正常颜色
    this->setColor(Color3B::WHITE);
    this->setOpacity(255);

    // ✅ 立即取消建筑锁定状态（触发颜色恢复）
    this->setTargetedByBuilding(false);

    // 隐藏血条
    if (_healthBar) {
        _healthBar->hide();
    }

    CCLOG("BattleUnitSprite: Death animation started, color reset to WHITE, targeting cleared");

    playAnimation(AnimationType::DEATH, false, callback);
}
