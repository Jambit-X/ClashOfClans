#include "DefenseBuildingAnimation.h"
#include <cmath>

USING_NS_CC;

DefenseBuildingAnimation* DefenseBuildingAnimation::create(Node* parentNode, int buildingType) {
    auto anim = new (std::nothrow) DefenseBuildingAnimation();
    if (anim && anim->init(parentNode, buildingType)) {
        anim->autorelease();
        return anim;
    }
    CC_SAFE_DELETE(anim);
    return nullptr;
}

DefenseBuildingAnimation::DefenseBuildingAnimation()
    : _buildingType(0)
    , _parentNode(nullptr)
    , _baseSprite(nullptr)
    , _barrelSprite(nullptr)
    , _muzzleFlashSprite(nullptr)
    , _animationOffset(Vec2::ZERO)     
    , _barrelOffset(Vec2::ZERO) {}     

DefenseBuildingAnimation::~DefenseBuildingAnimation() {
    CCLOG("DefenseBuildingAnimation: Destructor called");
    // Cocos2d-x 会自动管理子节点内存
}

bool DefenseBuildingAnimation::init(Node* parentNode, int buildingType) {
    if (!Node::init()) return false;

    this->setName("DefenseAnim");  // 设置名称便于查找

    _parentNode = parentNode;
    _buildingType = buildingType;

    if (_buildingType == 301) {  // 加农炮
        initCannonSprites(parentNode);
    }
    // 箭塔暂不处理

    return true;
}

void DefenseBuildingAnimation::initCannonSprites(Node* parentNode) {
    // 1. 创建底座（静态）
    auto baseFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName(
        "Animation/defence_architecture/cannon/cannon_stand.png"
    );

    if (!baseFrame) {
        CCLOG("DefenseBuildingAnimation: ERROR - Cannon base frame not found!");
        return;
    }

    _baseSprite = Sprite::createWithSpriteFrame(baseFrame);
    _baseSprite->setAnchorPoint(Vec2(0.5f, 0.0f));

    // 定位到父节点中心 + 整体偏移
    auto parentSize = parentNode->getContentSize();
    Vec2 basePos = Vec2(parentSize.width / 2, 0) + _animationOffset;
    _baseSprite->setPosition(basePos);

    this->addChild(_baseSprite, 0);  // 添加到当前Node，而非parentNode

    // 2. 创建炮管（可旋转）
    auto barrelFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName(
        "Animation/defence_architecture/cannon/cannon01.png"
    );

    if (!barrelFrame) {
        CCLOG("DefenseBuildingAnimation: ERROR - Cannon barrel frame not found!");
        return;
    }

    _barrelSprite = Sprite::createWithSpriteFrame(barrelFrame);
    _barrelSprite->setAnchorPoint(Vec2(0.5f, 0.0f));

    // 将炮管放在底座上方 + 炮管偏移
    auto baseSize = _baseSprite->getContentSize();
    Vec2 barrelPos = Vec2(baseSize.width / 2, baseSize.height * 0.4f) + _barrelOffset;
    _barrelSprite->setPosition(barrelPos);

    _baseSprite->addChild(_barrelSprite, 1);

    CCLOG("DefenseBuildingAnimation: Cannon sprites initialized (offset: %.1f, %.1f)",
          _animationOffset.x, _animationOffset.y);
}

// 设置整体偏移量
void DefenseBuildingAnimation::setAnimationOffset(const Vec2& offset) {
    _animationOffset = offset;

    if (_baseSprite) {
        auto parentSize = _parentNode->getContentSize();
        Vec2 basePos = Vec2(parentSize.width / 2, 0) + _animationOffset;
        _baseSprite->setPosition(basePos);

        CCLOG("DefenseBuildingAnimation: Animation offset updated to (%.1f, %.1f)",
              offset.x, offset.y);
    }
}

// 设置炮管偏移量
void DefenseBuildingAnimation::setBarrelOffset(const Vec2& offset) {
    _barrelOffset = offset;

    if (_barrelSprite && _baseSprite) {
        auto baseSize = _baseSprite->getContentSize();
        Vec2 barrelPos = Vec2(baseSize.width / 2, baseSize.height * 0.4f) + _barrelOffset;
        _barrelSprite->setPosition(barrelPos);

        CCLOG("DefenseBuildingAnimation: Barrel offset updated to (%.1f, %.1f)",
              offset.x, offset.y);
    }
}

void DefenseBuildingAnimation::aimAt(const Vec2& targetWorldPos) {
    if (!_barrelSprite || !_parentNode) return;

    // 计算父节点（建筑）到目标的方向
    Vec2 buildingWorldPos = _parentNode->getPosition();
    Vec2 direction = targetWorldPos - buildingWorldPos;

    // 计算角度（Cocos2d-x：0° = 右，逆时针）
    float angleRadians = atan2(direction.y, direction.x);
    float angleDegrees = CC_RADIANS_TO_DEGREES(angleRadians);

    // 转换为炮管坐标系（0° = 6点钟方向，顺时针）
    // 公式：barrelAngle = 90 - cocos2dAngle
    float barrelAngle = 90.0f - angleDegrees;

    // 归一化到 [0, 360)
    while (barrelAngle < 0) barrelAngle += 360.0f;
    while (barrelAngle >= 360.0f) barrelAngle -= 360.0f;

    // 设置炮管帧
    setBarrelFrame(barrelAngle);
}

void DefenseBuildingAnimation::setBarrelFrame(float angleDegrees) {
    if (!_barrelSprite) return;

    // 36 帧 = 每帧 10°
    // 帧 1 = 0°, 帧 2 = 10°, ..., 帧 36 = 350°
    int frameIndex = (int)(angleDegrees / 10.0f) % 36 + 1;

    // 限制范围 [1, 36]
    if (frameIndex < 1) frameIndex = 1;
    if (frameIndex > 36) frameIndex = 36;

    std::string frameName = StringUtils::format(
        "Animation/defence_architecture/cannon/cannon%02d.png", frameIndex
    );

    auto frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(frameName);
    if (frame) {
        _barrelSprite->setSpriteFrame(frame);
    }
}

void DefenseBuildingAnimation::playFireAnimation(const std::function<void()>& callback) {
    // 播放炮口火焰特效
    playMuzzleFlash();

    // 炮管后坐力动画
    if (_barrelSprite) {
        auto originalPos = _barrelSprite->getPosition();

        // 向后移动一点（模拟后坐力）
        auto recoil = MoveBy::create(0.05f, Vec2(0, -5));
        auto recover = MoveTo::create(0.1f, originalPos);

        // 执行动画序列
        auto sequence = Sequence::create(
            recoil,
            recover,
            CallFunc::create([callback]() {
            if (callback) callback();
        }),
            nullptr
        );

        _barrelSprite->runAction(sequence);
    } else if (callback) {
        callback();
    }
}

void DefenseBuildingAnimation::playMuzzleFlash() {
    // TODO: 添加炮口火焰粒子特效（可选）
    // 简单实现：可以在炮管顶部显示一个闪光精灵，然后淡出

    CCLOG("DefenseBuildingAnimation: Muzzle flash played");
}

void DefenseBuildingAnimation::resetBarrel() {
    // 重置炮管到默认角度（0° = 朝下）
    setBarrelFrame(0.0f);
}

void DefenseBuildingAnimation::setVisible(bool visible) {
    if (_baseSprite) {
        _baseSprite->setVisible(visible);
    }
}
