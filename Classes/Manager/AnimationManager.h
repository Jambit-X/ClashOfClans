#ifndef __ANIMATION_MANAGER_H__
#define __ANIMATION_MANAGER_H__

#include "cocos2d.h"
#include <string>
#include <unordered_map>

USING_NS_CC;

// 动画类型枚举
enum class AnimationType {
  IDLE,              // 待机
  WALK,              // 向右走
  WALK_UP,           // 向右上走
  WALK_DOWN,         // 向右下走
  RUN,
  ATTACK,            // 向右攻击
  ATTACK_UP,         // 向右上攻击
  ATTACK_DOWN,       // 向右下攻击
  DEATH,
  SPAWN,             // 还没有
  VICTORY,
  HURT
};

// 添加移动方向枚举
enum class MoveDirection {
  RIGHT,         // 向右（0度）
  RIGHT_UP,      // 右上（45度）
  UP,            // 向上（90度）
  LEFT_UP,       // 左上（135度）
  LEFT,          // 向左（180度）
  LEFT_DOWN,     // 左下（225度）
  DOWN,          // 向下（270度）
  RIGHT_DOWN     // 右下（315度）
};

// 动画配置数据
struct AnimationConfig {
  std::string framePrefix;
  int startFrame;        // 起始帧号
  int frameCount;        // 帧数量
  float frameDelay;
  bool loop;
};

// 动画管理器（单例）
class AnimationManager {
public:
  static AnimationManager* getInstance();
  static void destroyInstance();

  // 资源加载
  void loadSpriteFrames(const std::string& plistFile);
  void preloadBattleAnimations();
  void unloadSpriteFrames(const std::string& plistFile);

  // 动画创建
  Animation* createAnimation(const std::string& unitType, AnimationType animType);
  RepeatForever* createLoopAnimate(const std::string& unitType, AnimationType animType);
  Animate* createOnceAnimate(const std::string& unitType, AnimationType animType);

  // 配置管理
  void registerAnimationConfig(
    const std::string& unitType,
    AnimationType animType,
    const AnimationConfig& config
  );

  void initializeDefaultConfigs();

  // 辅助方法
  std::string animTypeToString(AnimationType type) const;

private:
  AnimationManager();
  ~AnimationManager();

  static AnimationManager* _instance;

  // 动画配置缓存: <"Barbarian_WALK", AnimationConfig>
  std::unordered_map<std::string, AnimationConfig> _animConfigs;

  // 已加载的 .plist 文件列表
  std::vector<std::string> _loadedPlists;

  std::string getConfigKey(const std::string& unitType, AnimationType animType) const;
};

#endif // __ANIMATION_MANAGER_H__