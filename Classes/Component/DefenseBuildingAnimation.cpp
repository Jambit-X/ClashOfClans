#include "DefenseBuildingAnimation.h"
#include "Sprite/BuildingSprite.h"
#include "Util/GridMapUtils.h"
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
}

bool DefenseBuildingAnimation::init(Node* parentNode, int buildingType) {
    if (!Node::init()) return false;

    this->setName("DefenseAnim");

    _parentNode = parentNode;
    _buildingType = buildingType;

    if (_buildingType == 301) {  // 加农炮
        initCannonSprites(parentNode);
    }

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

    this->addChild(_baseSprite, 0);

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
    _barrelInitialPos = barrelPos;
    _baseSprite->addChild(_barrelSprite, 1);

    CCLOG("DefenseBuildingAnimation: Cannon sprites initialized");
}

void DefenseBuildingAnimation::setAnimationOffset(const Vec2& offset) {
    _animationOffset = offset;

    if (_baseSprite) {
        auto parentSize = _parentNode->getContentSize();
        Vec2 basePos = Vec2(parentSize.width / 2, 0) + _animationOffset;
        _baseSprite->setPosition(basePos);
    }
}

void DefenseBuildingAnimation::setBarrelOffset(const Vec2& offset) {
    _barrelOffset = offset;

    if (_barrelSprite && _baseSprite) {
        auto baseSize = _baseSprite->getContentSize();
        Vec2 barrelPos = Vec2(baseSize.width / 2, baseSize.height * 0.4f) + _barrelOffset;
        _barrelSprite->setPosition(barrelPos);
        _barrelInitialPos = barrelPos;
    }
}

// ========== 核心重构：正确的瞄准逻辑 ==========
void DefenseBuildingAnimation::aimAt(const Vec2& targetWorldPos) {
    if (!_barrelSprite || !_parentNode) {
        CCLOG("DefenseBuildingAnimation::aimAt - NULL pointer!");
        return;
    }

    // ========== 步骤1：获取建筑信息 ==========
    auto buildingSprite = dynamic_cast<BuildingSprite*>(_parentNode);
    if (!buildingSprite) {
        CCLOG("DefenseBuildingAnimation::aimAt - Not a BuildingSprite!");
        return;
    }

    Vec2 gridPos = buildingSprite->getGridPos();
    Size gridSize = buildingSprite->getGridSize();

    CCLOG("========== AIM DEBUG START ==========");
    CCLOG("Building Grid Position: (%.0f, %.0f)", gridPos.x, gridPos.y);
    CCLOG("Building Grid Size: (%.0f, %.0f)", gridSize.width, gridSize.height);

    // ========== 步骤2：计算建筑中心的世界坐标 ==========
    // 关键点：3x3建筑的中心在 (gridX+1.5, gridY+1.5)
    float centerGridX = gridPos.x + gridSize.width * 0.5f;
    float centerGridY = gridPos.y + gridSize.height * 0.5f;

    CCLOG("Building Center Grid: (%.1f, %.1f)", centerGridX, centerGridY);

    // 使用 GridMapUtils 的仿射变换公式
    float buildingCenterX = GridMapUtils::GRID_ORIGIN_X
        + centerGridX * GridMapUtils::GRID_X_UNIT_X
        + centerGridY * GridMapUtils::GRID_Y_UNIT_X;

    float buildingCenterY = GridMapUtils::GRID_ORIGIN_Y
        + centerGridX * GridMapUtils::GRID_X_UNIT_Y
        + centerGridY * GridMapUtils::GRID_Y_UNIT_Y;

    Vec2 buildingCenterWorld(buildingCenterX, buildingCenterY);

    CCLOG("Building Center World: (%.1f, %.1f)", buildingCenterWorld.x, buildingCenterWorld.y);
    CCLOG("Target World: (%.1f, %.1f)", targetWorldPos.x, targetWorldPos.y);

    // ========== 步骤3：计算方向向量 ==========
    Vec2 direction = targetWorldPos - buildingCenterWorld;

    CCLOG("Direction Vector: (%.1f, %.1f)", direction.x, direction.y);
    CCLOG("Direction Length: %.1f", direction.length());

    if (direction.length() < 1.0f) {
        CCLOG("Target too close, skipping aim");
        CCLOG("========== AIM DEBUG END ==========");
        return;
    }

    // ========== 步骤4：计算 Cocos2d-x 角度 ==========
    // Cocos2d-x: 0° = 右(3点钟), 90° = 上(12点钟), 逆时针为正
    float angleRadians = atan2(direction.y, direction.x);
    float angleDegrees = CC_RADIANS_TO_DEGREES(angleRadians);

    // 归一化到 [0, 360)
    while (angleDegrees < 0) angleDegrees += 360.0f;
    while (angleDegrees >= 360.0f) angleDegrees -= 360.0f;

    CCLOG("Cocos2d-x Angle: %.1f°", angleDegrees);
    // ========== 步骤5：Cocos2d角度 -> 炮管帧角度 ==========
    // 炮管帧定义（逆时针）：
    // - cannon01.png (帧1) = 0° = 3点钟(向右) ✅
    // - cannon10.png (帧10) = 90° = 12点钟(向上) ✅
    // - cannon19.png (帧19) = 180° = 9点钟(向左) ✅
    // - cannon28.png (帧28) = 270° = 6点钟(向下) ✅
    //
    // Cocos2d-x 角度（逆时针）：
    // - 0° = 右, 90° = 上, 180° = 左, 270° = 下
    //
    // 完美匹配！直接使用 Cocos2d-x 角度即可

    float barrelAngle = angleDegrees;

    // 归一化到 [0, 360)
    if (barrelAngle >= 360.0f) barrelAngle -= 360.0f;

    CCLOG("Barrel Angle: %.1f°", barrelAngle);

    // ========== 步骤6：设置炮管帧 ==========
    setBarrelFrame(barrelAngle);

    CCLOG("========== AIM DEBUG END ==========");
}

void DefenseBuildingAnimation::setBarrelFrame(float angleDegrees) {
    if (!_barrelSprite) {
        CCLOG("DefenseBuildingAnimation::setBarrelFrame - _barrelSprite is NULL!");
        return;
    }

    // 36 帧 = 每帧 10°
    // 帧 1 = 0°(右), 帧 2 = 10°, ..., 帧 36 = 350°
    int frameIndex = static_cast<int>(angleDegrees / 10.0f) + 1;

    // 处理边界情况
    if (frameIndex < 1) frameIndex = 1;
    if (frameIndex > 36) frameIndex = 36;

    std::string frameName = StringUtils::format(
        "Animation/defence_architecture/cannon/cannon%02d.png", frameIndex
    );

    // ✅ 增强的日志输出
    CCLOG("🎯 [BARREL ROTATION] 正在播放炮管动画:");
    CCLOG("   ├─ 帧序号: %d/36", frameIndex);
    CCLOG("   ├─ 角度: %.1f°", angleDegrees);
    CCLOG("   └─ 帧文件: %s", frameName.c_str());

    auto frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(frameName);
    if (frame) {
        _barrelSprite->setSpriteFrame(frame);
        CCLOG("   ✅ 炮管帧切换成功!");
    } else {
        CCLOG("   ❌ ERROR - 炮管帧 '%s' 未找到!", frameName.c_str());
    }
}

void DefenseBuildingAnimation::playFireAnimation(const std::function<void()>& callback) {
    playMuzzleFlash();

    if (_barrelSprite) {
        // 停止所有旧动画
        _barrelSprite->stopAllActions();

        // 重置到初始位置
        _barrelSprite->setPosition(_barrelInitialPos);

        // 后坐力动画
        float recoilDistance = -5.0f;
        float recoilTime = 0.04f;
        float recoverTime = 0.08f;

        auto recoil = MoveBy::create(recoilTime, Vec2(0, recoilDistance));
        auto recover = MoveTo::create(recoverTime, _barrelInitialPos);

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
    // TODO: 添加炮口火焰特效
    CCLOG("DefenseBuildingAnimation: Muzzle flash played");
}

void DefenseBuildingAnimation::resetBarrel() {
    setBarrelFrame(0.0f);
}

void DefenseBuildingAnimation::setVisible(bool visible) {
    if (_baseSprite) {
        _baseSprite->setVisible(visible);
    }
}

void DefenseBuildingAnimation::playAttackAnimation(const Vec2& targetWorldPos, const std::function<void()>& callback) {
    if (_buildingType == 301) {  // 加农炮
        // 1. 瞄准目标
        aimAt(targetWorldPos);

        // 2. 延迟后开火
        this->runAction(Sequence::create(
            DelayTime::create(0.15f),
            CallFunc::create([this, callback]() {
            playFireAnimation(callback);
        }),
            nullptr
        ));

        CCLOG("DefenseBuildingAnimation: Cannon attack animation started");
    } else if (_buildingType == 302) {  // 箭塔
        playFireAnimation(callback);
        CCLOG("DefenseBuildingAnimation: Archer Tower attack animation started");
    } else {
        playFireAnimation(callback);
    }
}

void DefenseBuildingAnimation::playAttackAnimation() {
    playFireAnimation(nullptr);
}

// ✅✅✅ 测试函数：按顺序显示所有炮管帧
void DefenseBuildingAnimation::testAllBarrelFrames() {
    if (!_barrelSprite) {
        CCLOG("DefenseBuildingAnimation::testAllBarrelFrames - _barrelSprite is NULL!");
        return;
    }

    CCLOG("========================================");
    CCLOG("开始测试炮管36帧动画");
    CCLOG("每帧显示1秒，请观察炮管朝向");
    CCLOG("========================================");

    // 创建动作序列
    Vector<FiniteTimeAction*> actions;

    for (int frameIndex = 1; frameIndex <= 36; frameIndex++) {
        // 计算角度（帧1=0°, 帧2=10°, ..., 帧36=350°）
        float angle = (frameIndex - 1) * 10.0f;

        // 切换帧
        auto switchFrame = CallFunc::create([this, frameIndex, angle]() {
            std::string frameName = StringUtils::format(
                "Animation/defence_architecture/cannon/cannon%02d.png", frameIndex
            );

            auto frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(frameName);
            if (frame) {
                _barrelSprite->setSpriteFrame(frame);

                // 输出当前帧信息
                CCLOG("========================================");
                CCLOG("当前帧: cannon%02d.png", frameIndex);
                CCLOG("帧编号: %d/36", frameIndex);
                CCLOG("定义角度: %.1f°", angle);
                CCLOG("请观察炮管朝向！");
                CCLOG("========================================");
            } else {
                CCLOG("❌ 帧 %d 加载失败: %s", frameIndex, frameName.c_str());
            }
        });

        // 停留1秒
        auto delay = DelayTime::create(1.0f);

        actions.pushBack(switchFrame);
        actions.pushBack(delay);
    }

    // 测试完成后的回调
    auto finish = CallFunc::create([]() {
        CCLOG("========================================");
        CCLOG("✅ 测试完成！共36帧已全部显示");
        CCLOG("========================================");
    });

    actions.pushBack(finish);

    // 执行动作序列
    auto sequence = Sequence::create(actions);
    this->runAction(sequence);
}
