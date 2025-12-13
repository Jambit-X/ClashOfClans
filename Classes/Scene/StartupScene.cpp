#include "StartupScene.h"
#include "VillageScene.h"

USING_NS_CC;
using namespace cocos2d::ui;

Scene* StartupScene::createScene() {
    return StartupScene::create();
}

bool StartupScene::init() {
    if (!Scene::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // ==========================================
    // 1. 预先初始化"加载阶段"的元素 (先设为不可见)
    // ==========================================

    // --- 背景图 ---
    _loadingBg = Sprite::create("Scene/StartupScene.png");
    _loadingBg->setPosition(visibleSize / 2);
    float scaleX = visibleSize.width / _loadingBg->getContentSize().width;
    float scaleY = visibleSize.height / _loadingBg->getContentSize().height;
    _loadingBg->setScale(std::max(scaleX, scaleY));
    _loadingBg->setOpacity(0);
    this->addChild(_loadingBg, 0);

    // --- 进度条 ---
    _progressBar = LoadingBar::create("ImageElements/loading_bar.png");
    _progressBar->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.2f));
    _progressBar->setPercent(0);
    _progressBar->setOpacity(0);
    this->addChild(_progressBar, 1);

    // ==========================================
    // 2. 初始化"Supercell Splash"阶段
    // ==========================================
    showSplashPhase();

    return true;
}

void StartupScene::showSplashPhase() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 1. 黑色背景层
    _splashLayer = LayerColor::create(Color4B::BLACK);
    this->addChild(_splashLayer, 10);

    // 2. Supercell Logo - 调整为更小尺寸
    _logo = Sprite::create("Scene/supercell_logo.png");
    _logo->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.6f));

    float targetScale = 0.6f; // 进一步缩小到0.6倍
    if (_logo->getContentSize().width * targetScale > visibleSize.width * 0.25f) {
        // 如果放大后太宽，调整到屏幕宽度的25%
        targetScale = (visibleSize.width * 0.25f) / _logo->getContentSize().width;
    }
    targetScale = std::max(targetScale, 0.4f); // 确保最小缩放不小于0.4倍

    // 初始状态：完全透明，从小开始
    _logo->setOpacity(0);
    _logo->setScale(targetScale * 0.7f);
    _splashLayer->addChild(_logo);

    // Logo简化动画：淡入 + 缩放到目标大小
    auto fadeIn = FadeIn::create(0.8f);
    auto scaleUp = ScaleTo::create(0.8f, targetScale);

    // 同时执行淡入和缩放动画
    auto logoAnimation = Spawn::create(fadeIn, scaleUp, nullptr);
    _logo->runAction(logoAnimation);

    // 3. 法律文本 - 直接显示，无动画
    std::string textContent = u8"本公司积极履行《网络游戏行业防沉迷自律公约》\n根据国家新闻出版署《关于防止未成年人沉迷网络游戏的通知》\n所有用户均需使用有效身份信息完成账号实名注册后方可进入游戏";

    _legalTextLabel = Label::createWithSystemFont(textContent, "Microsoft YaHei UI", 22);
    _legalTextLabel->setAlignment(TextHAlignment::CENTER);
    _legalTextLabel->setDimensions(visibleSize.width * 0.85f, 0);
    _legalTextLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.2f));
    _legalTextLabel->setTextColor(Color4B::WHITE);
    _legalTextLabel->enableShadow(Color4B(0, 0, 0, 100), Size(1, -1));
    _legalTextLabel->setLineSpacing(4);

    // 法律文本直接显示，无淡入动画
    _splashLayer->addChild(_legalTextLabel);

    // 4. 定时调度
    this->scheduleOnce(CC_SCHEDULE_SELECTOR(StartupScene::showLoadingPhase), 0.3f);
}

void StartupScene::showLoadingPhase(float dt) {
    // 让logo和文本也渐渐淡出，而不是突然消失
    if (_logo) {
        _logo->runAction(FadeOut::create(0.5f));
    }
    if (_legalTextLabel) {
        _legalTextLabel->runAction(FadeOut::create(0.5f));
    }

    // splash层背景也淡出，然后移除
    if (_splashLayer) {
        _splashLayer->runAction(Sequence::create(
            FadeOut::create(0.5f),
            RemoveSelf::create(),
            nullptr
        ));
    }

    // 立即开始loading元素的淡入，与splash淡出同步
    if (_loadingBg) _loadingBg->runAction(FadeIn::create(0.5f));
    if (_progressBar) _progressBar->runAction(FadeIn::create(0.5f));

    // 延迟0.5秒后开始进度条更新，等待淡入完成
    this->scheduleOnce([this](float) {
        this->schedule(CC_SCHEDULE_SELECTOR(StartupScene::updateLoadingBar), 0.05f);
        }, 0.05f, "start_progress");
}

void StartupScene::updateLoadingBar(float dt) {
    float currentPercent = _progressBar->getPercent();
    float speed = (currentPercent < 50) ? 3.0f : 5.0f;
    float newPercent = currentPercent + speed;

    if (newPercent >= 100) {
        _progressBar->setPercent(100);
        this->unschedule(CC_SCHEDULE_SELECTOR(StartupScene::updateLoadingBar));
        this->scheduleOnce(CC_SCHEDULE_SELECTOR(StartupScene::goToVillageScene), 0.05f);
    }
    else {
        _progressBar->setPercent(newPercent);
    }
}

void StartupScene::goToVillageScene(float dt) {
    auto scene = VillageScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}