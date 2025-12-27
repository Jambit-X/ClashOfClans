// BattleProgressUI.cpp
// 战斗进度UI实现，管理星数和摧毁率的显示与动画

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

    _container = Node::create();
    this->addChild(_container);

    _currentStars = 0;
    _currentProgress = 0.0f;
    _isShown = false;

    initBackground();
    initStars();
    initProgressLabel();

    // 计算右下角位置，确保UI完全显示在屏幕内
    float rightPadding = 20.0f;
    float bottomPadding = 100.0f;
    float backgroundWidth = 250.0f;

    _container->setPosition(Vec2(
        origin.x + visibleSize.width - backgroundWidth - rightPadding,
        origin.y + bottomPadding
    ));

    // 初始隐藏状态
    _container->setOpacity(0);
    _container->setVisible(false);

    CCLOG("BattleProgressUI: Initialized at position (%.1f, %.1f)",
          _container->getPositionX(), _container->getPositionY());

    return true;
}

void BattleProgressUI::initBackground() {
    // 创建半透明黑色背景
    _background = LayerColor::create(Color4B(0, 0, 0, 180), 250, 90);
    _background->setAnchorPoint(Vec2(0.0f, 0.0f));
    _background->setPosition(Vec2(0, 0));
    _container->addChild(_background, -1);
}

void BattleProgressUI::initStars() {
    float starSize = 35.0f;
    float starSpacing = 50.0f;

    // 星星在背景上居中排列
    float startX = 70.0f;
    float starY = 71.0f;

    for (int i = 0; i < 3; i++) {
        _starSprites[i] = Sprite::create("UI/battle/battle-prepare/victory_star_bg.png");

        if (!_starSprites[i]) {
            // 加载失败时创建灰色占位符
            _starSprites[i] = Sprite::create();
            _starSprites[i]->setTextureRect(Rect(0, 0, starSize, starSize));
            _starSprites[i]->setColor(Color3B(50, 50, 50));
        }

        _starSprites[i]->setScale(starSize / _starSprites[i]->getContentSize().width);
        _starSprites[i]->setPosition(Vec2(startX + i * starSpacing, starY));

        _background->addChild(_starSprites[i]);
    }
}

void BattleProgressUI::initProgressLabel() {
    // 创建"摧毁进度:"标题标签
    auto progressTitle = Label::createWithSystemFont("摧毁进度:", "Arial", 20);
    progressTitle->setPosition(Vec2(60, 30));
    progressTitle->setColor(Color3B::WHITE);
    progressTitle->enableOutline(Color4B::BLACK, 2);
    progressTitle->setAnchorPoint(Vec2(0.0f, 0.5f));
    _background->addChild(progressTitle);

    // 创建百分比数字标签
    _progressLabel = Label::createWithSystemFont("0%", "Arial", 28);
    _progressLabel->setPosition(Vec2(160, 30));
    _progressLabel->setColor(Color3B(255, 215, 0));
    _progressLabel->enableOutline(Color4B::BLACK, 2);
    _progressLabel->enableShadow(Color4B(0, 0, 0, 150), Size(2, -2));
    _progressLabel->setAnchorPoint(Vec2(0.0f, 0.5f));
    _background->addChild(_progressLabel);

    _progressLabel->setString("0%");
}

void BattleProgressUI::updateStars(int starCount) {
    if (starCount < 0 || starCount > 3) return;

    // 只播放新获得星星的动画
    for (int i = _currentStars; i < starCount; i++) {
        playStarAnimation(i);
    }

    _currentStars = starCount;
}

void BattleProgressUI::updateProgress(float progress) {
    _currentProgress = progress;

    std::string progressText = StringUtils::format("%.0f%%", progress);
    _progressLabel->setString(progressText);
}

void BattleProgressUI::show() {
    if (_isShown) return;

    _isShown = true;
    _container->setVisible(true);

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

    // 切换到金色星星纹理
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
        star->setColor(Color3B(255, 215, 0));
    }

    // 播放弹跳缩放动画
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
    
    // 目标位置为屏幕中间偏上
    Vec2 targetPos(
        origin.x + (visibleSize.width - backgroundWidth) / 2.0f - 60.0f,
        origin.y + visibleSize.height * 0.8f
    );

    float targetScale = 1.5f;

    CCLOG("BattleProgressUI: Playing result animation to (%.1f, %.1f)",
          targetPos.x, targetPos.y);

    // 移动和缩放动画同时播放
    auto moveAction = MoveTo::create(0.5f, targetPos);
    auto scaleAction = ScaleTo::create(0.5f, targetScale);

    auto spawn = Spawn::create(
        EaseOut::create(moveAction, 2.0f),
        EaseOut::create(scaleAction, 2.0f),
        nullptr
    );

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
    _currentStars = 0;
    _currentProgress = 0.0f;
    _isShown = false;

    // 重置所有星星为暗色背景
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

    _progressLabel->setString("0%");

    // 重置位置和显示状态
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
