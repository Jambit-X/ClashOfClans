#include "HealthBarComponent.h"

USING_NS_CC;

// 无参版本（使用默认配置）
HealthBarComponent* HealthBarComponent::create() {
    return create(Config());
}

HealthBarComponent* HealthBarComponent::create(const Config& config) {
    auto component = new (std::nothrow) HealthBarComponent();
    if (component && component->init(config)) {
        component->autorelease();
        return component;
    }
    CC_SAFE_DELETE(component);
    return nullptr;
}

bool HealthBarComponent::init(const Config& config) {
    if (!Node::init()) {
        return false;
    }

    _config = config;

    // 创建血条UI
    createBarComponents();

    // 默认隐藏
    this->setVisible(false);

    return true;
}

void HealthBarComponent::createBarComponents() {
    // 1. 创建背景（深灰色）
    _background = Sprite::create();
    _background->setTextureRect(Rect(0, 0, _config.width, _config.height));
    _background->setColor(Color3B(40, 40, 40));
    _background->setOpacity(200);
    _background->setAnchorPoint(Vec2(0.5f, 0.5f));
    _background->setPosition(Vec2::ZERO);
    this->addChild(_background, 1);

    // 2. 创建进度条（初始绿色）
    auto barSprite = Sprite::create();
    barSprite->setTextureRect(Rect(0, 0, _config.width, _config.height));
    barSprite->setColor(Color3B(50, 205, 50));  // 绿色

    _progressBar = ProgressTimer::create(barSprite);
    _progressBar->setType(ProgressTimer::Type::BAR);
    _progressBar->setMidpoint(Vec2(0, 0.5f));     // 从左边开始
    _progressBar->setBarChangeRate(Vec2(1, 0));   // 水平方向变化
    _progressBar->setPercentage(100.0f);
    _progressBar->setAnchorPoint(Vec2(0.5f, 0.5f));
    _progressBar->setPosition(Vec2::ZERO);
    this->addChild(_progressBar, 2);
}

void HealthBarComponent::updateHealth(int currentHP, int maxHP) {
    if (maxHP <= 0) {
        hide();
        return;
    }

    // 计算百分比
    float percent = (float)currentHP / (float)maxHP * 100.0f;
    percent = std::max(0.0f, std::min(100.0f, percent));

    // 满血时是否隐藏
    if (!_config.showWhenFull && percent >= 100.0f) {
        hide();
        return;
    }

    // 死亡时隐藏
    if (currentHP <= 0) {
        hide();
        return;
    }

    // 显示血条
    if (!_isShowing) {
        show();
    }

    // 更新进度
    if (_progressBar) {
        _progressBar->setPercentage(percent);

        // 更新颜色
        Color3B barColor = getColorForPercent(percent);
        _progressBar->getSprite()->setColor(barColor);
    }
}

void HealthBarComponent::show() {
    if (_isShowing) return;

    _isShowing = true;
    this->setVisible(true);

    // 淡入动画
    if (_config.fadeInDuration > 0) {
        this->setOpacity(0);
        this->runAction(FadeTo::create(_config.fadeInDuration, 255));
    } else {
        this->setOpacity(255);
    }
}

void HealthBarComponent::hide() {
    if (!_isShowing) return;

    _isShowing = false;
    this->setVisible(false);
}

void HealthBarComponent::updatePosition(const Size& parentSize) {
    // 根据父节点大小和配置的偏移量计算位置
    Vec2 position;
    position.x = parentSize.width / 2 + _config.offset.x;
    position.y = parentSize.height + _config.offset.y;

    this->setPosition(position);
}

Color3B HealthBarComponent::getColorForPercent(float percent) const {
    if (percent > _config.highThreshold) {
        // 高血量：绿色
        return Color3B(50, 205, 50);
    } else if (percent > _config.mediumThreshold) {
        // 中血量：黄色
        return Color3B(255, 200, 0);
    } else {
        // 低血量：红色
        return Color3B(220, 50, 50);
    }
}
