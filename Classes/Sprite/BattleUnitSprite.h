// BattleUnitSprite.h
#ifndef __BATTLE_UNIT_SPRITE_H__
#define __BATTLE_UNIT_SPRITE_H__

#include "cocos2d.h"
#include "Manager/AnimationManager.h"

USING_NS_CC;

class BattleUnitSprite : public Sprite {
public:
  static BattleUnitSprite* create(const std::string& unitType);
  virtual bool init(const std::string& unitType);

  // 基础动画控制
  void playAnimation(AnimationType animType, bool loop = false,
                     const std::function<void()>& callback = nullptr);
  
  void stopCurrentAnimation();
  void playIdleAnimation();
  void playWalkAnimation();
  void playAttackAnimation(const std::function<void()>& callback = nullptr);
  void playDeathAnimation(const std::function<void()>& callback = nullptr);

  // ===== 像素坐标行走 =====
  
  // 行走到指定像素位置
  void walkToPosition(const Vec2& targetPos, float duration = 2.0f,
                      const std::function<void()>& callback = nullptr);
  
  // 沿向量偏移行走
  void walkByOffset(const Vec2& offset, float duration = 2.0f,
                    const std::function<void()>& callback = nullptr);

  // ===== 网格坐标行走 =====
  
  // 从当前网格位置行走到目标网格位置
  void walkToGrid(int targetGridX, int targetGridY, float speed = 100.0f,
                  const std::function<void()>& callback = nullptr);
  
  // 从指定网格行走到目标网格
  void walkFromGridToGrid(int startGridX, int startGridY, 
                         int targetGridX, int targetGridY,
                         float speed = 100.0f,
                         const std::function<void()>& callback = nullptr);

  // ===== 方向攻击 =====
  
  // 向指定方向攻击（只播放动画，不移动）
  void attackInDirection(const Vec2& direction, 
                        const std::function<void()>& callback = nullptr);
  
  // 攻击指定位置（计算方向，站定攻击）
  void attackTowardPosition(const Vec2& targetPos, 
                           const std::function<void()>& callback = nullptr);
  
  // 攻击指定网格（计算方向，站定攻击）
  void attackTowardGrid(int targetGridX, int targetGridY,
                       const std::function<void()>& callback = nullptr);

  // ===== 网格位置管理 =====
  
  // 设置单位当前网格位置
  void setGridPosition(int gridX, int gridY);
  
  // 获取单位当前网格位置
  Vec2 getGridPosition() const { return _currentGridPos; }
  
  // 将单位瞬移到网格位置（不播放动画）
  void teleportToGrid(int gridX, int gridY);

  // ===== 寻路移动 =====
  
  /**
   * @brief 使用寻路算法移动到指定世界坐标
   * @param targetWorldPos 目标世界坐标
   * @param speed 移动速度（像素/秒）
   * @param callback 完成后回调
   */
  void moveToTargetWithPathfinding(
      const Vec2& targetWorldPos, 
      float speed = 100.0f,
      const std::function<void()>& callback = nullptr);
  
  /**
   * @brief 使用寻路算法移动到指定网格（根据VillageDataManager的建筑占用表避开障碍）
   * @param targetGridX 目标网格X坐标
   * @param targetGridY 目标网格Y坐标
   * @param speed 移动速度（像素/秒）
   * @param callback 完成后回调
   */
  void moveToGridWithPathfinding(
      int targetGridX, 
      int targetGridY, 
      float speed = 100.0f,
      const std::function<void()>& callback = nullptr);
  
  /**
   * @brief 沿指定路径列表移动
   * @param path 路径点列表（世界坐标）
   * @param speed 移动速度（像素/秒）
   * @param callback 完成后回调
   */
  void followPath(
      const std::vector<Vec2>& path, 
      float speed = 100.0f,
      const std::function<void()>& callback = nullptr);

  // ===== 属性访问 =====
  
  std::string getUnitType() const { return _unitType; }
  AnimationType getCurrentAnimation() const { return _currentAnimation; }
  bool isAnimating() const { return _isAnimating; }

protected:
  std::string _unitType;
  AnimationType _currentAnimation;
  bool _isAnimating;
  Vec2 _currentGridPos;  // 当前网格坐标
  
  static const int ANIMATION_TAG = 1000;
  static const int MOVE_TAG = 1001;

  // 根据移动方向选择动画类型和是否需要翻转
  void selectWalkAnimation(const Vec2& direction, AnimationType& outAnimType, bool& outFlipX);
  void selectAttackAnimation(const Vec2& direction, AnimationType& outAnimType, bool& outFlipX);
  
  // 计算角度（0度=右，逆时针）
  float getAngleFromDirection(const Vec2& direction);
};

#endif // __BATTLE_UNIT_SPRITE_H__