// BattleUnitSprite.cpp
#include "BattleUnitSprite.h"
#include "Util/GridMapUtils.h"
#include "Util/FindPathUtil.h"  // ? 添加寻路工具
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
  _currentAnimation = AnimationType::IDLE;
  _isAnimating = false;
  _currentGridPos = Vec2::ZERO;

  std::string unitTypeLower = unitType;
  std::transform(unitTypeLower.begin(), unitTypeLower.end(),
                 unitTypeLower.begin(), ::tolower);

  std::string firstFrameName = unitTypeLower + "1.0.png";

  bool success = Sprite::initWithSpriteFrameName(firstFrameName);

  if (!success) {
    CCLOG("BattleUnitSprite: Failed to load first frame: %s", firstFrameName.c_str());
    return false;
  }

  this->setAnchorPoint(Vec2(0.5f, 0.0f));
  CCLOG("BattleUnitSprite: Created %s", unitType.c_str());
  return true;
}

// ===== 基础动画控制 =====

void BattleUnitSprite::playAnimation(
  AnimationType animType,
  bool loop,
  const std::function<void()>& callback
) {
  stopCurrentAnimation();

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
  playAnimation(AnimationType::IDLE, true);
}

void BattleUnitSprite::playWalkAnimation() {
  playAnimation(AnimationType::WALK, true);
}

void BattleUnitSprite::playAttackAnimation(const std::function<void()>& callback) {
  playAnimation(AnimationType::ATTACK, false, callback);
}

void BattleUnitSprite::playDeathAnimation(const std::function<void()>& callback) {
  playAnimation(AnimationType::DEATH, false, callback);
}

// ===== 方向计算 =====

float BattleUnitSprite::getAngleFromDirection(const Vec2& direction) {
  return CC_RADIANS_TO_DEGREES(atan2f(direction.y, direction.x));
}

void BattleUnitSprite::selectWalkAnimation(const Vec2& direction,
                                           AnimationType& outAnimType,
                                           bool& outFlipX) {
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

    for (const auto& waypoint : path) {
        // 计算方向和距离
        Vec2 direction = waypoint - currentPos;
        float distance = direction.length();

        if (distance < 0.1f) {
            continue; // 跳过太近的点
        }

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