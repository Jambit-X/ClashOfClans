#pragma execution_character_set("utf-8")

#include "BattleProgressUI.h"

USING_NS_CC;

BattleProgressUI* BattleProgressUI::create() {
    auto ret = new (std::nothrow) BattleProgressUI();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool BattleProgressUI::init() {
    if (!Node::init()) {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // 创建容器
    _container = Node::create();
    this->addChild(_container);

    // 初始化状态
    _currentStars = 0;
    _currentProgress = 0.0f;
    _isShown = false;

    // 初始化UI元素
    initBackground();
    initStars();
    initProgressLabel();

    // ✅ 修复：考虑背景宽度，让整个UI框完全显示在屏幕内
    float rightPadding = 20.0f;   // 右边距（从屏幕右边缘往左）
    float bottomPadding = 100.0f;  // 底部边距
    float backgroundWidth = 250.0f; // 背景宽度

    _container->setPosition(Vec2(
        origin.x + visibleSize.width - backgroundWidth - rightPadding,  // ✅ 关键：减去背景宽度
        origin.y + bottomPadding
    ));

    // ✅ 初始隐藏
    _container->setOpacity(0);
    _container->setVisible(false);

    CCLOG("BattleProgressUI: Initialized at position (%.1f, %.1f)",
          _container->getPositionX(), _container->getPositionY());

    return true;
}

void BattleProgressUI::initBackground() {
    // 半透明黑色背景
    _background = LayerColor::create(Color4B(0, 0, 0, 180), 250, 90);

    // ✅ 最简单：锚点左下角，位置(0, 0)
    _background->setAnchorPoint(Vec2(0.0f, 0.0f));
    _background->setPosition(Vec2(0, 0));

    _container->addChild(_background, -1);
}

void BattleProgressUI::initStars() {
    float starSize = 35.0f;
    float starSpacing = 50.0f;

    // ✅ 关键：把星星添加到背景上，使用背景的坐标系
    // 背景宽度 250，3个星星总宽度约 135
    // 居中：(250 - 135) / 2 = 57.5 起始
    float startX = 70.0f;   // 左边距
    float starY = 71.0f;    // 上方位置

    for (int i = 0; i < 3; i++) {
        _starSprites[i] = Sprite::create("UI/battle/battle-prepare/victory_star_bg.png");

        if (!_starSprites[i]) {
            _starSprites[i] = Sprite::create();
            _starSprites[i]->setTextureRect(Rect(0, 0, starSize, starSize));
            _starSprites[i]->setColor(Color3B(50, 50, 50));
        }

        _starSprites[i]->setScale(starSize / _starSprites[i]->getContentSize().width);
        _starSprites[i]->setPosition(Vec2(startX + i * starSpacing, starY));

        // ✅ 添加到背景上，不是容器
        _background->addChild(_starSprites[i]);
    }
}

void BattleProgressUI::initProgressLabel() {
    // ✅ 标签和文字也添加到背景上

    // "摧毁进度:"标签
    auto progressTitle = Label::createWithSystemFont("摧毁进度:", "Arial", 20);
    progressTitle->setPosition(Vec2(60, 30));  // 背景坐标系：左下角
    progressTitle->setColor(Color3B::WHITE);
    progressTitle->enableOutline(Color4B::BLACK, 2);
    progressTitle->setAnchorPoint(Vec2(0.0f, 0.5f));  // 左对齐
    _background->addChild(progressTitle);  // ✅ 添加到背景

    // 百分比数字
    _progressLabel = Label::createWithSystemFont("0%", "Arial", 28);
    _progressLabel->setPosition(Vec2(160, 30));  // 标签右侧
    _progressLabel->setColor(Color3B(255, 215, 0));
    _progressLabel->enableOutline(Color4B::BLACK, 2);
    _progressLabel->enableShadow(Color4B(0, 0, 0, 150), Size(2, -2));
    _progressLabel->setAnchorPoint(Vec2(0.0f, 0.5f));  // 左对齐
    _background->addChild(_progressLabel);  // ✅ 添加到背景

    _progressLabel->setString("0%");
}

void BattleProgressUI::updateStars(int starCount) {
    if (starCount < 0 || starCount > 3) return;

    // 只更新新获得的星星
    for (int i = _currentStars; i < starCount; i++) {
        playStarAnimation(i);
    }

    _currentStars = starCount;
}

void BattleProgressUI::updateProgress(float progress) {
    _currentProgress = progress;

    // 更新文字
    std::string progressText = StringUtils::format("%.0f%%", progress);
    _progressLabel->setString(progressText);
}

void BattleProgressUI::show() {
    if (_isShown) return;

    _isShown = true;
    _container->setVisible(true);

    // 淡入动画
    _container->runAction(Sequence::create(
        FadeIn::create(0.3f),
        nullptr
    ));

    CCLOG("BattleProgressUI: Showing UI");
}

void BattleProgressUI::playStarAnimation(int starIndex) {
    if (starIndex < 0 || starIndex >= 3 || !_starSprites[starIndex]) {
        return;
    }

    auto star = _starSprites[starIndex];
    float originalScale = star->getScale();

    // 1. 切换到金色星星
    auto goldTexture = Director::getInstance()->getTextureCache()->addImage(
        "UI/battle/battle-prepare/victory_star.png"
    );

    if (goldTexture) {
        star->setTexture(goldTexture);
        auto rect = Rect(0, 0, goldTexture->getContentSize().width,
                         goldTexture->getContentSize().height);
        star->setTextureRect(rect);
        star->setScale(originalScale);
    } else {
        star->setColor(Color3B(255, 215, 0));  // 金色
    }

    // 2. 播放弹跳动画
    star->setScale(0.1f);

    auto scaleUp = ScaleTo::create(0.2f, originalScale * 1.3f);
    auto scaleNormal = ScaleTo::create(0.1f, originalScale);
    auto bounce = EaseBackOut::create(scaleUp);

    star->runAction(Sequence::create(
        bounce,
        scaleNormal,
        nullptr
    ));

    CCLOG("BattleProgressUI: Star %d animation played", starIndex);
}

void BattleProgressUI::playResultAnimation(const std::function<void()>& onComplete) {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    float backgroundWidth = 250.0f;
    // 目标位置：屏幕中间偏上
    Vec2 targetPos(
        origin.x + (visibleSize.width - backgroundWidth) / 2.0f - 60.0f,
        origin.y + visibleSize.height * 0.8f  // 80%高度位置
    );

    // 目标缩放：放大到1.5倍
    float targetScale = 1.5f;

    CCLOG("BattleProgressUI: Playing result animation to (%.1f, %.1f)",
          targetPos.x, targetPos.y);

    // 组合动画
    auto moveAction = MoveTo::create(0.5f, targetPos);
    auto scaleAction = ScaleTo::create(0.5f, targetScale);

    auto spawn = Spawn::create(
        EaseOut::create(moveAction, 2.0f),
        EaseOut::create(scaleAction, 2.0f),
        nullptr
    );

    // 执行动画
    _container->runAction(Sequence::create(
        spawn,
        CallFunc::create([onComplete]() {
        if (onComplete) {
            onComplete();
        }
    }),
        nullptr
    ));
}

void BattleProgressUI::reset() {
    // 重置状态
    _currentStars = 0;
    _currentProgress = 0.0f;
    _isShown = false;

    // 重置星星为暗色
    for (int i = 0; i < 3; i++) {
        if (_starSprites[i]) {
            auto texture = Director::getInstance()->getTextureCache()->addImage(
                "UI/battle/battle-prepare/victory_star_bg.png"
            );
            if (texture) {
                _starSprites[i]->setTexture(texture);
            } else {
                _starSprites[i]->setColor(Color3B(50, 50, 50));
            }
        }
    }

    // 重置进度
    _progressLabel->setString("0%");

    // 重置位置和缩放
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    _container->setPosition(Vec2(
        origin.x + visibleSize.width - 100,
        origin.y + 100
    ));
    _container->setScale(1.0f);
    _container->setOpacity(0);
    _container->setVisible(false);

    CCLOG("BattleProgressUI: Reset to initial state");
}
