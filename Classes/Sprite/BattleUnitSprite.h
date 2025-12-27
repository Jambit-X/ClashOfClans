// BattleUnitSprite.h
// 战斗单位精灵，支持动画、移动、攻击和生命值系统

#ifndef __BATTLE_UNIT_SPRITE_H__
#define __BATTLE_UNIT_SPRITE_H__

#include "cocos2d.h"
#include "Manager/AnimationManager.h"
#include "../Util/GridMapUtils.h"
#include "Component/HealthBarComponent.h"

USING_NS_CC;

// 单位类型枚举
enum class UnitTypeID {
    UNKNOWN = 0,
    BARBARIAN = 1001,
    ARCHER = 1002,
    GOBLIN = 1003,
    GIANT = 1004,
    WALL_BREAKER = 1005,
    BALLOON = 1006
};

class BattleUnitSprite : public Sprite {
public:
  static BattleUnitSprite* create(const std::string& unitType);
  virtual bool init(const std::string& unitType);
  virtual void update(float dt) override;

  // 基础动画控制
  void playAnimation(AnimationType animType, bool loop = false,
                     const std::function<void()>& callback = nullptr);
  
  void stopCurrentAnimation();
  void playIdleAnimation();
  void playWalkAnimation();
  void playAttackAnimation(const std::function<void()>& callback = nullptr);
  void playDeathAnimation(const std::function<void()>& callback = nullptr);

  // 像素坐标行走
  void walkToPosition(const Vec2& targetPos, float duration = 2.0f,
                      const std::function<void()>& callback = nullptr);
  
  void walkByOffset(const Vec2& offset, float duration = 2.0f,
                    const std::function<void()>& callback = nullptr);

  // 网格坐标行走
  void walkToGrid(int targetGridX, int targetGridY, float speed = 100.0f,
                  const std::function<void()>& callback = nullptr);
  
  void walkFromGridToGrid(int startGridX, int startGridY, 
                         int targetGridX, int targetGridY,
                         float speed = 100.0f,
                         const std::function<void()>& callback = nullptr);

  // 方向攻击
  void attackInDirection(const Vec2& direction, 
                        const std::function<void()>& callback = nullptr);
  
  void attackTowardPosition(const Vec2& targetPos, 
                           const std::function<void()>& callback = nullptr);
  
  void attackTowardGrid(int targetGridX, int targetGridY,
                       const std::function<void()>& callback = nullptr);

  // 网格位置管理
  void setGridPosition(int gridX, int gridY);
  Vec2 getGridPosition() const { return _currentGridPos; }
  void teleportToGrid(int gridX, int gridY);

  // 寻路移动
  void moveToTargetWithPathfinding(
      const Vec2& targetWorldPos, 
      float speed = 100.0f,
      const std::function<void()>& callback = nullptr);
  
  void moveToGridWithPathfinding(
      int targetGridX, 
      int targetGridY, 
      float speed = 100.0f,
      const std::function<void()>& callback = nullptr);
  
  void followPath(
      const std::vector<Vec2>& path, 
      float speed = 100.0f,
      const std::function<void()>& callback = nullptr);

  // 生命值系统
  void takeDamage(int damage);
  int getCurrentHP() const { return _currentHP; }
  int getMaxHP() const { return _maxHP; }
  bool isDead() const { return _currentHP <= 0; }

  // 属性访问
  std::string getUnitType() const { return _unitType; }
  UnitTypeID getUnitTypeID() const { return _unitTypeID; }
  AnimationType getCurrentAnimation() const { return _currentAnimation; }
  bool isAnimating() const { return _isAnimating; }
  
  // 状态标志
  bool isChangingTarget() const { return _isChangingTarget; }
  void setChangingTarget(bool changing) { _isChangingTarget = changing; }
  
  // 建筑锁定状态
  bool isTargetedByBuilding() const { return _isTargetedByBuilding; }
  void setTargetedByBuilding(bool targeted);
  void updateHealthBar();

protected:
  std::string _unitType;
  UnitTypeID _unitTypeID = UnitTypeID::UNKNOWN;
  AnimationType _currentAnimation;
  bool _isAnimating;
  Vec2 _currentGridPos;
  int _lastGridX = -999;
  int _lastGridY = -999;
  bool _isChangingTarget = false;
  bool _isTargetedByBuilding = false;

  Vec2 _lastMoveDirection = Vec2::ZERO;
  
  int _currentHP = 0;
  int _maxHP = 0;
  
  static const int ANIMATION_TAG = 1000;
  static const int MOVE_TAG = 1001;

  HealthBarComponent* _healthBar = nullptr;

  void selectWalkAnimation(const Vec2& direction, AnimationType& outAnimType, bool& outFlipX);
  void selectAttackAnimation(const Vec2& direction, AnimationType& outAnimType, bool& outFlipX);
  
  float getAngleFromDirection(const Vec2& direction);
  
  static UnitTypeID parseUnitType(const std::string& unitType);
  static float getScaleForUnitType(UnitTypeID typeID);
};

#endif // __BATTLE_UNIT_SPRITE_H__
