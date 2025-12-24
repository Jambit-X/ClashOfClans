#include "StartupScene.h"
#include "VillageScene.h"
#include "Manager/AudioManager.h"

USING_NS_CC;
using namespace cocos2d::ui;

Scene* StartupScene::createScene() {
    return StartupScene::create();
}

bool StartupScene::init() {
    if (!Scene::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    
    // ========== 初始化音频 ID ==========
    _supercellJingleID = -1;
    _startupJingleID = -1;

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
    
    // ========== 播放 Supercell Jingle (1.5秒) ==========
    auto audioManager = AudioManager::getInstance();
    _supercellJingleID = audioManager->playEffect("Audios/supercell_jingle.mp3");

    // 1. 黑色背景层
    _splashLayer = LayerColor::create(Color4B::BLACK);
    this->addChild(_splashLayer, 10);

    // 2. Supercell Logo - 调整为更小尺寸
    _logo = Sprite::create("Scene/supercell_logo.png");
    _logo->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.6f));

    float targetScale = 0.6f;
    if (_logo->getContentSize().width * targetScale > visibleSize.width * 0.25f) {
        targetScale = (visibleSize.width * 0.25f) / _logo->getContentSize().width;
    }
    targetScale = std::max(targetScale, 0.4f);

    // 初始状态：完全透明，从小开始
    _logo->setOpacity(0);
    _logo->setScale(targetScale * 0.7f);
    _splashLayer->addChild(_logo);

    // ========== Logo 动画：0.6 秒（适配 1.5 秒音频） ==========
    auto fadeIn = FadeIn::create(0.6f);
    auto scaleUp = ScaleTo::create(0.6f, targetScale);

    auto logoAnimation = Spawn::create(fadeIn, scaleUp, nullptr);
    _logo->runAction(logoAnimation);

    // 3. 法律文本 - 快速淡入
    std::string textContent = u8"本公司积极履行《网络游戏行业防沉迷自律公约》\n根据国家新闻出版署《关于防止未成年人沉迷网络游戏的通知》\n所有用户均需使用有效身份信息完成账号实名注册后方可进入游戏";

    _legalTextLabel = Label::createWithSystemFont(textContent, "Microsoft YaHei UI", 22);
    _legalTextLabel->setAlignment(TextHAlignment::CENTER);
    _legalTextLabel->setDimensions(visibleSize.width * 0.85f, 0);
    _legalTextLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.2f));
    _legalTextLabel->setTextColor(Color4B::WHITE);
    _legalTextLabel->enableShadow(Color4B(0, 0, 0, 100), Size(1, -1));
    _legalTextLabel->setLineSpacing(4);
    
    // 法律文本淡入（0.3 秒延迟 + 0.5 秒淡入）
    _legalTextLabel->setOpacity(0);
    _legalTextLabel->runAction(Sequence::create(
        DelayTime::create(0.3f),
        FadeIn::create(0.5f),
        nullptr
    ));
    
    _splashLayer->addChild(_legalTextLabel);

    // ========== 4. 定时调度：1.5 秒后切换（精确匹配音频时长） ==========
    this->scheduleOnce(CC_SCHEDULE_SELECTOR(StartupScene::showLoadingPhase), 1.5f);
}

void StartupScene::showLoadingPhase(float dt) {
    // ========== 播放 Loading Screen Jingle (3.5秒) ==========
    auto audioManager = AudioManager::getInstance();
    _startupJingleID = audioManager->playEffect("Audios/loading_screen_jingle.mp3");
    
    // 让 logo 和文本快速淡出（0.3 秒）
    if (_logo) {
        _logo->runAction(FadeOut::create(0.3f));
    }
    if (_legalTextLabel) {
        _legalTextLabel->runAction(FadeOut::create(0.3f));
    }

    // splash 层背景也淡出，然后移除
    if (_splashLayer) {
        _splashLayer->runAction(Sequence::create(
            FadeOut::create(0.3f),
            RemoveSelf::create(),
            nullptr
        ));
    }

    // 立即开始 loading 元素的淡入（0.3 秒）
    if (_loadingBg) _loadingBg->runAction(FadeIn::create(0.3f));
    if (_progressBar) _progressBar->runAction(FadeIn::create(0.3f));

    // ========== 延迟 0.4 秒后开始进度条更新 ==========
    this->scheduleOnce([this](float) {
        this->schedule(CC_SCHEDULE_SELECTOR(StartupScene::updateLoadingBar), 0.05f);
    }, 0.4f, "start_progress");
}

void StartupScene::updateLoadingBar(float dt) {
    float currentPercent = _progressBar->getPercent();
    
    // ========== 调整速度让进度条在 2.6 秒内完成（0.4s延迟 + 2.6s进度 + 0.5s停留 = 3.5s） ==========
    // 2.6秒 / 0.05秒 = 52次更新
    // 100% / 52 = 1.92% 每次
    float speed = 1.92f;
    
    float newPercent = currentPercent + speed;

    if (newPercent >= 100) {
        _progressBar->setPercent(100);
        this->unschedule(CC_SCHEDULE_SELECTOR(StartupScene::updateLoadingBar));
        
        // ========== 进度条满后延迟 0.5 秒再跳转（让音频播完） ==========
        this->scheduleOnce(CC_SCHEDULE_SELECTOR(StartupScene::goToVillageScene), 0.5f);
    }
    else {
        _progressBar->setPercent(newPercent);
    }
}

void StartupScene::goToVillageScene(float dt) {
    // ========== 切换场景前停止所有音频 ==========
    auto audioManager = AudioManager::getInstance();
    if (_supercellJingleID != -1) {
        audioManager->stopAudio(_supercellJingleID);
    }
    if (_startupJingleID != -1) {
        audioManager->stopAudio(_startupJingleID);
    }
    
    auto scene = VillageScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}
