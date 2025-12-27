// AnimationManager.cpp
// 动画管理器实现，负责加载和创建战斗单位的动画

#include "AnimationManager.h"

USING_NS_CC;

AnimationManager* AnimationManager::_instance = nullptr;

AnimationManager::AnimationManager() {
    CCLOG("AnimationManager: Initialized");
}

AnimationManager::~AnimationManager() {
    for (const auto& plist : _loadedPlists) {
        SpriteFrameCache::getInstance()->removeSpriteFramesFromFile(plist);
    }
    _loadedPlists.clear();
    _animConfigs.clear();
    CCLOG("AnimationManager: Destroyed");
}

AnimationManager* AnimationManager::getInstance() {
    if (!_instance) {
        _instance = new AnimationManager();
    }
    return _instance;
}

void AnimationManager::destroyInstance() {
    if (_instance) {
        delete _instance;
        _instance = nullptr;
    }
}

void AnimationManager::loadSpriteFrames(const std::string& plistFile) {
    auto cache = SpriteFrameCache::getInstance();
    cache->addSpriteFramesWithFile(plistFile);
    _loadedPlists.push_back(plistFile);
    CCLOG("AnimationManager: Loaded sprite frames from %s", plistFile.c_str());
}

void AnimationManager::preloadBattleAnimations() {
    CCLOG("AnimationManager: Preloading battle animations...");

    // 预加载野蛮人动画
    loadSpriteFrames("Animation/troop/barbarian/barbarian.plist");

    // 预加载弓箭手动画
    loadSpriteFrames("Animation/troop/archer/archer.plist");

    // 预加载巨人动画
    loadSpriteFrames("Animation/troop/giant/giant.plist");

    // 预加载哥布林动画
    loadSpriteFrames("Animation/troop/goblin/goblin.plist");

    // 预加载炸弹兵动画
    loadSpriteFrames("Animation/troop/wall_breaker/wall_breaker.plist");

    // 预加载加农炮动画
    loadSpriteFrames("Animation/defence_architecture/cannon/atlas.plist");

    CCLOG("AnimationManager: All battle animations preloaded");
}

void AnimationManager::unloadSpriteFrames(const std::string& plistFile) {
    SpriteFrameCache::getInstance()->removeSpriteFramesFromFile(plistFile);
    auto it = std::find(_loadedPlists.begin(), _loadedPlists.end(), plistFile);
    if (it != _loadedPlists.end()) {
        _loadedPlists.erase(it);
        CCLOG("AnimationManager: Unloaded sprite frames from %s", plistFile.c_str());
    }
}

Animation* AnimationManager::createAnimation(const std::string& unitType, AnimationType animType) {
    std::string key = getConfigKey(unitType, animType);
    auto it = _animConfigs.find(key);

    if (it == _animConfigs.end()) {
        CCLOG("AnimationManager: Config not found for %s", key.c_str());
        return nullptr;
    }

    const AnimationConfig& config = it->second;

    // 创建帧序列
    Vector<SpriteFrame*> frames;
    auto cache = SpriteFrameCache::getInstance();

    // 支持两种模式
    if (config.isNonContinuous()) {
        // 模式2: 非连续帧
        CCLOG("AnimationManager: Creating animation '%s' with non-continuous frames", key.c_str());

        for (int frameIndex : config.frameIndices) {
            std::string frameName = config.framePrefix + StringUtils::format("%d.0.png", frameIndex);
            SpriteFrame* frame = cache->getSpriteFrameByName(frameName);

            if (frame) {
                frames.pushBack(frame);
            } else {
                CCLOG("AnimationManager: Frame not found: %s", frameName.c_str());
            }
        }

        CCLOG("AnimationManager: Loaded %d non-continuous frames", (int)frames.size());
    } else {
        // 模式1: 连续帧
        for (int i = 0; i < config.frameCount; ++i) {
            int frameIndex = config.startFrame + i;
            std::string frameName = config.framePrefix + StringUtils::format("%d.0.png", frameIndex);

            SpriteFrame* frame = cache->getSpriteFrameByName(frameName);

            if (frame) {
                frames.pushBack(frame);
            } else {
                CCLOG("AnimationManager: Frame not found: %s", frameName.c_str());
            }
        }

        CCLOG("AnimationManager: Created animation '%s' with %d frames (frames %d-%d)",
              key.c_str(), (int)frames.size(), config.startFrame, config.startFrame + config.frameCount - 1);
    }

    if (frames.empty()) {
        CCLOG("AnimationManager: No frames loaded for config: %s", key.c_str());
        return nullptr;
    }

    Animation* animation = Animation::createWithSpriteFrames(frames, config.frameDelay);
    return animation;
}

RepeatForever* AnimationManager::createLoopAnimate(const std::string& unitType, AnimationType animType) {
    Animation* animation = createAnimation(unitType, animType);
    if (!animation) {
        CCLOG("AnimationManager: Failed to create loop animation for %s_%s",
              unitType.c_str(), animTypeToString(animType).c_str());
        return nullptr;
    }

    Animate* animate = Animate::create(animation);
    return RepeatForever::create(animate);
}

Animate* AnimationManager::createOnceAnimate(const std::string& unitType, AnimationType animType) {
    Animation* animation = createAnimation(unitType, animType);
    if (!animation) {
        CCLOG("AnimationManager: Failed to create once animation for %s_%s",
              unitType.c_str(), animTypeToString(animType).c_str());
        return nullptr;
    }

    return Animate::create(animation);
}

void AnimationManager::registerAnimationConfig(
    const std::string& unitType,
    AnimationType animType,
    const AnimationConfig& config
) {
    std::string key = getConfigKey(unitType, animType);
    _animConfigs[key] = config;

    if (config.isNonContinuous()) {
        CCLOG("AnimationManager: Registered config: %s (non-continuous frames: %zu frames)",
              key.c_str(), config.frameIndices.size());
    } else {
        CCLOG("AnimationManager: Registered config: %s (frames: %d-%d)",
              key.c_str(), config.startFrame, config.startFrame + config.frameCount - 1);
    }
}

void AnimationManager::initializeDefaultConfigs() {
    CCLOG("AnimationManager: Initializing default animation configs...");

    // 野蛮人动画配置
    registerAnimationConfig("Barbarian", AnimationType::IDLE, {
        "barbarian", {26, 29}, 0.3f, false
    });

    registerAnimationConfig("Barbarian", AnimationType::IDLE_UP, {
        "barbarian", {27, 30} , 0.3f, true
    });

    registerAnimationConfig("Barbarian", AnimationType::IDLE_DOWN, {
        "barbarian", {25, 28}, 0.3f, true
    });

    registerAnimationConfig("Barbarian", AnimationType::WALK, {
        "barbarian", 9, 8, 0.1f, true
    });

    registerAnimationConfig("Barbarian", AnimationType::WALK_UP, {
        "barbarian", 17, 8, 0.1f, true
    });

    registerAnimationConfig("Barbarian", AnimationType::WALK_DOWN, {
        "barbarian", 1, 8, 0.1f, true
    });

    registerAnimationConfig("Barbarian", AnimationType::ATTACK, {
        "barbarian", 66, 10, 0.12f, false
    });

    registerAnimationConfig("Barbarian", AnimationType::ATTACK_UP, {
        "barbarian", 164, 11, 0.12f, false
    });

    registerAnimationConfig("Barbarian", AnimationType::ATTACK_DOWN, {
        "barbarian", 142, 11, 0.12f, false
    });

    registerAnimationConfig("Barbarian", AnimationType::DEATH, {
        "barbarian", 176, 1, 0.5f, false
    });

    // 弓箭手动画配置
    registerAnimationConfig("Archer", AnimationType::IDLE, {
        "archer", 27, 3, 0.3f, true
    });

    registerAnimationConfig("Archer", AnimationType::IDLE_UP, {
        "archer", 30, 3, 0.3f, true
    });

    registerAnimationConfig("Archer", AnimationType::IDLE_DOWN, {
        "archer", 33, 3, 0.3f, true
    });

    registerAnimationConfig("Archer", AnimationType::WALK, {
        "archer", 9, 8, 0.1f, true
    });

    registerAnimationConfig("Archer", AnimationType::WALK_UP, {
        "archer", 17, 8, 0.1f, true
    });

    registerAnimationConfig("Archer", AnimationType::WALK_DOWN, {
        "archer", 1, 8, 0.1f, true
    });

    registerAnimationConfig("Archer", AnimationType::ATTACK, {
        "archer", {35, 36, 37, 45, 46, 47, 48}, 0.08f, false
    });

    registerAnimationConfig("Archer", AnimationType::ATTACK_UP, {
        "archer", {38, 39, 40, 49, 50, 51, 52}, 0.08f, false
    });

    registerAnimationConfig("Archer", AnimationType::ATTACK_DOWN, {
        "archer", {32, 33, 34, 41, 42, 43, 44}, 0.08f, false
    });

    registerAnimationConfig("Archer", AnimationType::DEATH, {
        "archer", 54, 1, 0.5f, false
    });

    // 巨人动画配置
    registerAnimationConfig("Giant", AnimationType::IDLE, {
        "giant", {38, 42, 43}, 0.4f, true
    });

    registerAnimationConfig("Giant", AnimationType::IDLE_UP, {
        "giant", {39, 44, 45}, 0.4f, true
    });

    registerAnimationConfig("Giant", AnimationType::IDLE_DOWN, {
        "giant", {37, 40, 41}, 0.4f, true
    });

    registerAnimationConfig("Giant", AnimationType::WALK, {
        "giant", 13, 12, 0.15f, true
    });

    registerAnimationConfig("Giant", AnimationType::WALK_UP, {
        "giant", 25, 12, 0.15f, true
    });

    registerAnimationConfig("Giant", AnimationType::WALK_DOWN, {
        "giant", 1, 12, 0.15f, true
    });

    registerAnimationConfig("Giant", AnimationType::ATTACK, {
        "giant", 55, 9, 0.15f, false
    });

    registerAnimationConfig("Giant", AnimationType::ATTACK_UP, {
        "giant", 64, 8, 0.15f, false
    });

    registerAnimationConfig("Giant", AnimationType::ATTACK_DOWN, {
        "giant", 46, 9, 0.15f, false
    });

    registerAnimationConfig("Giant", AnimationType::DEATH, {
        "giant", 100, 1, 0.5f, false
    });

    // 哥布林动画配置
    registerAnimationConfig("Goblin", AnimationType::IDLE, {
        "goblin", 26, 1, 0.2f, true
    });

    registerAnimationConfig("Goblin", AnimationType::IDLE_UP, {
        "goblin", 27, 1, 0.2f, true
    });

    registerAnimationConfig("Goblin", AnimationType::IDLE_DOWN, {
        "goblin", 25, 1, 0.2f, true
    });

    registerAnimationConfig("Goblin", AnimationType::WALK, {
        "goblin", 9, 8, 0.08f, true
    });

    registerAnimationConfig("Goblin", AnimationType::WALK_UP, {
        "goblin", 17, 8, 0.08f, true
    });

    registerAnimationConfig("Goblin", AnimationType::WALK_DOWN, {
        "goblin", 1, 8, 0.08f, true
    });

    registerAnimationConfig("Goblin", AnimationType::ATTACK, {
        "goblin", 32, 5, 0.1f, false
    });

    registerAnimationConfig("Goblin", AnimationType::ATTACK_UP, {
        "goblin", 37, 5, 0.1f, false
    });

    registerAnimationConfig("Goblin", AnimationType::ATTACK_DOWN, {
        "goblin", 28, 5, 0.1f, false
    });

    registerAnimationConfig("Goblin", AnimationType::DEATH, {
        "goblin", 42, 1, 0.5f, false
    });

    // 炸弹兵动画配置
    registerAnimationConfig("Wall_Breaker", AnimationType::DEATH, {
        "wall_breaker", 1, 1, 1.0f, false
    });

    registerAnimationConfig("Wall_Breaker", AnimationType::IDLE, {
        "wall_breaker", 7, 4, 0.15f, true
    });

    registerAnimationConfig("Wall_Breaker", AnimationType::IDLE_UP, {
        "wall_breaker", 3, 4, 0.15f, true
    });

    registerAnimationConfig("Wall_Breaker", AnimationType::IDLE_DOWN, {
        "wall_breaker", 11, 4, 0.15f, true
    });

    registerAnimationConfig("Wall_Breaker", AnimationType::WALK, {
        "wall_breaker", 41, 8, 0.08f, true
    });

    registerAnimationConfig("Wall_Breaker", AnimationType::WALK_UP, {
        "wall_breaker", 33, 8, 0.08f, true
    });

    registerAnimationConfig("Wall_Breaker", AnimationType::WALK_DOWN, {
        "wall_breaker", 49, 8, 0.08f, true
    });

    registerAnimationConfig("Wall_Breaker", AnimationType::ATTACK, {
        "wall_breaker", 41, 8, 0.08f, false
    });

    registerAnimationConfig("Wall_Breaker", AnimationType::ATTACK_UP, {
        "wall_breaker", 33, 8, 0.08f, false
    });

    registerAnimationConfig("Wall_Breaker", AnimationType::ATTACK_DOWN, {
        "wall_breaker", 49, 8, 0.08f, false
    });

    CCLOG("AnimationManager: Default configs initialized (including Wall_Breaker)");
}

std::string AnimationManager::getConfigKey(const std::string& unitType, AnimationType animType) const {
    return unitType + "_" + animTypeToString(animType);
}

std::string AnimationManager::animTypeToString(AnimationType type) const {
    switch (type) {
        case AnimationType::IDLE:        return "IDLE";
        case AnimationType::IDLE_UP:     return "IDLE_UP";        
        case AnimationType::IDLE_DOWN:   return "IDLE_DOWN";      
        case AnimationType::WALK:        return "WALK";
        case AnimationType::WALK_UP:     return "WALK_UP";
        case AnimationType::WALK_DOWN:   return "WALK_DOWN";
        case AnimationType::RUN:         return "RUN";
        case AnimationType::ATTACK:      return "ATTACK";
        case AnimationType::ATTACK_UP:   return "ATTACK_UP";
        case AnimationType::ATTACK_DOWN: return "ATTACK_DOWN";
        case AnimationType::DEATH:       return "DEATH";
        case AnimationType::SPAWN:       return "SPAWN";
        case AnimationType::VICTORY:     return "VICTORY";
        case AnimationType::HURT:        return "HURT";
        default: return "UNKNOWN";
    }
}
