#include "Projectile.h"

USING_NS_CC;

Projectile::Projectile()
    : _type(Type::CANNON_BULLET)
    , _rotateWithDirection(false)
    , _onHitCallback(nullptr) {}

Projectile::~Projectile() {
    CCLOG("Projectile: Destroyed");
}

Projectile* Projectile::create(Type type) {
    Projectile* ret = new (std::nothrow) Projectile();
    if (ret && ret->init(type)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool Projectile::init(Type type) {
    _type = type;
    std::string texturePath = getTexturePathForType(type);

    if (!Sprite::initWithFile(texturePath)) {
        CCLOG("Projectile: Failed to load texture: %s", texturePath.c_str());
        return false;
    }

    // 根据类型设置默认属性
    switch (_type) {
        case Type::CANNON_BULLET:
            // 炮弹：圆形,不需要旋转
            setAnchorPoint(Vec2(0.5f, 0.5f));
            _rotateWithDirection = false;
            CCLOG("Projectile: Created CANNON_BULLET");
            break;

        case Type::ARROW:
            // 箭矢：竖直朝下,需要旋转指向飞行方向
            // 锚点设在箭尾(底部中心),方便旋转
            setAnchorPoint(Vec2(0.5f, 0.0f));
            _rotateWithDirection = true;
            // 初始旋转90度(因为资源是竖直朝下,我们需要横向飞行)
            setRotation(-90.0f);
            CCLOG("Projectile: Created ARROW");
            break;

        case Type::MORTAR_BULLET:
            // 迫击炮弹：类似炮弹,但飞行轨迹是抛物线
            setAnchorPoint(Vec2(0.5f, 0.5f));
            _rotateWithDirection = false;
            CCLOG("Projectile: Created MORTAR_BULLET");
            break;
    }

    return true;
}

std::string Projectile::getTexturePathForType(Type type) const {
    switch (type) {
        case Type::CANNON_BULLET:
            return "Animation/projectiles/cannon_bullet/cannon_bullet.png";
        case Type::ARROW:
            return "Animation/projectiles/arrow/arrow.png";
        case Type::MORTAR_BULLET:
            return "Animation/projectiles/mortar_bullet/mortar_bullet.png";
        default:
            return "";
    }
}

void Projectile::updateRotationForDirection(const Vec2& direction) {
    if (!_rotateWithDirection) return;

    // 计算飞行方向的角度(相对于X轴正方向)
    float angleRadians = atan2(direction.y, direction.x);
    float angleDegrees = CC_RADIANS_TO_DEGREES(angleRadians);

    // Cocos2d-x的旋转：逆时针为正
    // 箭矢初始朝右(0度),需要调整到实际方向
    setRotation(-angleDegrees - 90.0f);
}

void Projectile::flyTo(const Vec2& target, float duration, const std::function<void()>& onHit) {
    _onHitCallback = onHit;

    Vec2 startPos = getPosition();
    Vec2 direction = target - startPos;

    // 如果是箭矢,旋转到正确方向
    updateRotationForDirection(direction);

    // 创建移动动作
    auto move = MoveTo::create(duration, target);

    // 创建回调动作
    auto callback = CallFunc::create([this]() {
        if (_onHitCallback) {
            _onHitCallback();
        }
        // 播放命中特效(可选)
        playHitEffect();
        // 自动移除
        removeFromParent();
    });

    // 组合动作序列
    auto sequence = Sequence::create(move, callback, nullptr);
    runAction(sequence);

    CCLOG("Projectile: Flying to (%.1f, %.1f) in %.2fs", target.x, target.y, duration);
}

void Projectile::arcTo(const Vec2& target, float duration, float height, const std::function<void()>& onHit) {
    _onHitCallback = onHit;

    // 使用 JumpTo 实现抛物线效果
    // jumps=1 表示一个完整的抛物线
    auto jump = JumpTo::create(duration, target, height, 1);

    // 可选：添加旋转动画(模拟炮弹滚动)
    auto rotate = RotateBy::create(duration, 360.0f * 2);  // 飞行过程中旋转2圈
    auto spawn = Spawn::create(jump, rotate, nullptr);

    auto callback = CallFunc::create([this]() {
        if (_onHitCallback) {
            _onHitCallback();
        }
        playHitEffect();
        removeFromParent();
    });

    auto sequence = Sequence::create(spawn, callback, nullptr);
    runAction(sequence);

    CCLOG("Projectile: Arc to (%.1f, %.1f) in %.2fs, height=%.1f",
          target.x, target.y, duration, height);
}

void Projectile::playHitEffect() {
    // TODO: 添加命中特效
    // 例如：爆炸粒子效果、闪光等

    switch (_type) {
        case Type::CANNON_BULLET:
            CCLOG("Projectile: BOOM! (cannon hit effect)");
            // 可以播放爆炸粒子
            break;

        case Type::ARROW:
            CCLOG("Projectile: Thud! (arrow hit effect)");
            // 可以播放箭矢插入特效
            break;

        case Type::MORTAR_BULLET:
            CCLOG("Projectile: KABOOM! (mortar explosion effect)");
            // 可以播放大范围爆炸特效
            break;
    }
}
