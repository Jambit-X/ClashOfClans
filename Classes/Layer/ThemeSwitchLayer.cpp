#pragma execution_character_set("utf-8")
#include "Layer/ThemeSwitchLayer.h"
#include "Manager/VillageDataManager.h"
#include "Layer/VillageLayer.h"
#include "Scene/VillageScene.h"

USING_NS_CC;
using namespace ui;

bool ThemeSwitchLayer::init() {
    if (!Layer::init()) {
        return false;
    }

    // 1. 半透明遮罩
    auto shieldLayer = LayerColor::create(Color4B(0, 0, 0, 180));
    this->addChild(shieldLayer);

    // 设置触摸吞噬
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch* t, Event* e) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 2. 加载场景数据
    loadThemeData();

    // 3. 初始化UI组件
    initBackground();
    initScrollView();
    initBottomButton();

    // 4. 获取当前选中的场景
    auto dataManager = VillageDataManager::getInstance();
    _currentSelectedTheme = dataManager->getCurrentThemeId();

    // 5. 更新底部按钮
    updateBottomButton();

    return true;
}

void ThemeSwitchLayer::initBackground() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 主面板容器
    _panelNode = Node::create();
    _panelNode->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    this->addChild(_panelNode);

    // ========== 修改：使用图片背景 ==========
    float panelW = 800;
    float panelH = 500;

    // 1. 尝试加载背景图片
    auto panelBg = Sprite::create("UI/Village/change_bg_card.png");
    if (panelBg) {
        // 图片加载成功，设置锚点和缩放
        panelBg->setAnchorPoint(Vec2(0.5f, 0.5f));
        panelBg->setPosition(Vec2(0, 0));  // 相对于 _panelNode 中心

        // 如果需要缩放以适应尺寸
        Size imgSize = panelBg->getContentSize();
        float scaleX = panelW / imgSize.width;
        float scaleY = panelH / imgSize.height;
        panelBg->setScale(std::max(scaleX, scaleY));  // 保持比例缩放

        _panelNode->addChild(panelBg, -1);  // ZOrder=-1 确保在最底层

        CCLOG("ThemeSwitchLayer: Background image loaded successfully");
    } else {
        // 图片加载失败，使用颜色层作为后备方案
        CCLOG("ThemeSwitchLayer: WARNING - Failed to load background image, using color fallback");
        auto panelBg = LayerColor::create(Color4B(40, 30, 20, 255), panelW, panelH);
        panelBg->ignoreAnchorPointForPosition(false);
        panelBg->setAnchorPoint(Vec2(0.5f, 0.5f));
        _panelNode->addChild(panelBg, -1);
    }
    // ==========================================

    // 关闭按钮
    auto closeBtn = Button::create("ImageElements/close_btn.png");
    if (closeBtn) {
        closeBtn->setScale(0.8f);
        closeBtn->setPosition(Vec2(panelW / 2 - 40, panelH / 2 - 20));
        closeBtn->addClickEventListener([this](Ref*) {
            onCloseClicked();
        });
        _panelNode->addChild(closeBtn);
    }
}

void ThemeSwitchLayer::initScrollView() {
    float scrollW = 740;
    float scrollH = 280;

    _scrollView = ScrollView::create();
    _scrollView->setDirection(ScrollView::Direction::HORIZONTAL);
    _scrollView->setContentSize(Size(scrollW, scrollH));
    _scrollView->setAnchorPoint(Vec2(0.5f, 0.5f));
    _scrollView->setPosition(Vec2(0, 20));  // 在面板中心偏上
    _scrollView->setScrollBarEnabled(false);
    _scrollView->setBounceEnabled(true);
    _panelNode->addChild(_scrollView);

    // 动态添加场景卡片
    float startX = 20;
    float padding = 20;
    float cardWidth = 200;

    for (const auto& pair : _themes) {
        createThemeCard(pair.second, startX);
        startX += cardWidth + padding;
    }

    _scrollView->setInnerContainerSize(Size(startX, scrollH));
}

void ThemeSwitchLayer::createThemeCard(const MapTheme& theme, float startX) {
    float cardW = 200;
    float cardH = 220;

    // 卡片容器
    auto card = Sprite::create();
    card->setTextureRect(Rect(0, 0, cardW, cardH));
    card->setColor(Color3B(60, 60, 60));
    card->setPosition(Vec2(startX + cardW / 2, 140));
    _scrollView->addChild(card);

    _themeCards[theme.id] = card;  // 保存引用用于更新边框

    // 场景预览图（缩小版）
    auto preview = Sprite::create(theme.mapImage);
    if (preview) {
        float scale = std::min(180.0f / preview->getContentSize().width,
                               150.0f / preview->getContentSize().height);
        preview->setScale(scale);
        preview->setPosition(cardW / 2, cardH / 2 + 20);
        card->addChild(preview);
    }

    // 场景名称
    auto nameLabel = Label::createWithTTF(theme.name, FONT_PATH, 20);
    nameLabel->setPosition(cardW / 2, 25);
    nameLabel->enableOutline(Color4B::BLACK, 2);
    card->addChild(nameLabel);

    // 选中边框（默认隐藏）
    auto border = DrawNode::create();
    border->drawRect(Vec2(0, 0), Vec2(cardW, cardH), Color4F::YELLOW);
    border->setLineWidth(4);
    border->setVisible(theme.id == _currentSelectedTheme);
    border->setTag(999);
    card->addChild(border);

    // 点击事件
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(true);
    touchListener->onTouchBegan = [this, card, theme](Touch* touch, Event* event) {
        Vec2 locationInNode = card->convertToNodeSpace(touch->getLocation());
        Rect rect = Rect(0, 0, card->getContentSize().width, card->getContentSize().height);

        if (rect.containsPoint(locationInNode)) {
            onThemeSelected(theme.id);
            return true;
        }
        return false;
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, card);
}

void ThemeSwitchLayer::initBottomButton() {
    // ✅ 关键修复：使用一个1x1的透明占位图创建按钮，或者使用 Scale9Sprite
    _bottomButton = Button::create();
    _bottomButton->setScale9Enabled(true);  // ✅ 启用九宫格，让 setContentSize 生效
    _bottomButton->setCapInsets(Rect(0, 0, 0, 0));  // ✅ 设置九宫格边距

    _bottomButton->setTitleFontSize(24);
    _bottomButton->setTitleColor(Color3B::WHITE);
    _bottomButton->setPosition(Vec2(0, -210));
    _bottomButton->setContentSize(Size(200, 60));

    // ✅ 添加一个透明背景确保触摸区域存在
    auto touchArea = LayerColor::create(Color4B(0, 0, 0, 1), 200, 60);  // 几乎透明
    touchArea->ignoreAnchorPointForPosition(false);
    touchArea->setAnchorPoint(Vec2::ZERO);
    touchArea->setPosition(Vec2::ZERO);
    _bottomButton->addChild(touchArea, -100);  // 最底层

    _bottomButton->addClickEventListener([this](Ref*) {
        CCLOG("ThemeSwitchLayer: Bottom button clicked!");

        auto dataManager = VillageDataManager::getInstance();
        auto theme = _themes[_currentSelectedTheme];

        if (!theme.isPurchased) {
            CCLOG("ThemeSwitchLayer: Calling onPurchaseClicked()");
            onPurchaseClicked();
        } else {
            CCLOG("ThemeSwitchLayer: Calling onConfirmClicked()");
            onConfirmClicked();
        }
    });

    _panelNode->addChild(_bottomButton, 10);

    CCLOG("ThemeSwitchLayer: Bottom button created at position (%.1f, %.1f)",
          _bottomButton->getPosition().x, _bottomButton->getPosition().y);
}
void ThemeSwitchLayer::loadThemeData() {
    auto dataManager = VillageDataManager::getInstance();

    // 场景1：经典（默认拥有）
    MapTheme classic;
    classic.id = 1;
    classic.name = "经典";
    classic.previewImage = "Scene/VillageScene.png";
    classic.mapImage = "Scene/VillageScene.png";
    classic.unlockTownHallLevel = 0;
    classic.gemCost = 0;
    classic.isPurchased = true;  // 默认拥有
    classic.hasParticleEffect = false;
    _themes[1] = classic;

    // 场景2：冬天（带雪花效果）
    MapTheme winter;
    winter.id = 2;
    winter.name = "冬天";
    winter.previewImage = "Scene/Map_Classic_Winter.png";
    winter.mapImage = "Scene/Map_Classic_Winter.png";
    winter.unlockTownHallLevel = 0;
    winter.gemCost = 0;
    winter.isPurchased = dataManager->isThemePurchased(2);
    winter.hasParticleEffect = true;
    _themes[2] = winter;

    // 场景3：Royale（大本营3级解锁）
    MapTheme royale;
    royale.id = 3;
    royale.name = "皇室竞技场";
    royale.previewImage = "Scene/Map_Royale.png";
    royale.mapImage = "Scene/Map_Royale.png";
    royale.unlockTownHallLevel = 3;
    royale.gemCost = 0;
    royale.isPurchased = dataManager->getTownHallLevel() >= 3;
    royale.hasParticleEffect = false;
    _themes[3] = royale;

    // 场景4：Crossover（花100宝石购买）
    MapTheme crossover;
    crossover.id = 4;
    crossover.name = "联动欢乐谷";
    crossover.previewImage = "Scene/Map_Crossover.png";
    crossover.mapImage = "Scene/Map_Crossover.png";
    crossover.unlockTownHallLevel = 0;
    crossover.gemCost = 100;
    crossover.isPurchased = dataManager->isThemePurchased(4);
    crossover.hasParticleEffect = false;
    _themes[4] = crossover;
}

void ThemeSwitchLayer::onThemeSelected(int themeId) {
    CCLOG("ThemeSwitchLayer: Selected theme ID=%d", themeId);

    // 更新选中状态
    for (const auto& pair : _themeCards) {
        auto border = pair.second->getChildByTag(999);
        if (border) {
            border->setVisible(pair.first == themeId);
        }
    }

    _currentSelectedTheme = themeId;
    updateBottomButton();
}

void ThemeSwitchLayer::updateBottomButton() {
    auto dataManager = VillageDataManager::getInstance();
    auto theme = _themes[_currentSelectedTheme];

    CCLOG("ThemeSwitchLayer: updateBottomButton called for theme %d (isPurchased=%s)",
          theme.id, theme.isPurchased ? "YES" : "NO");

    // ✅ 只移除非固定子节点（保留 tag=-100 的触摸区域）
    auto children = _bottomButton->getChildren();
    Vector<Node*> toRemove;
    for (auto child : children) {
        if (child->getTag() != -100) {  // 不移除触摸区域
            toRemove.pushBack(child);
        }
    }
    for (auto child : toRemove) {
        child->removeFromParent();
    }

    _bottomButton->setTitleText("");

    // 获取按钮尺寸
    Size btnSize = _bottomButton->getContentSize();
    Vec2 btnCenter = Vec2(btnSize.width / 2, btnSize.height / 2);

    if (!theme.isPurchased) {
        // ========== 未购买/未解锁 ==========
        if (theme.unlockTownHallLevel > dataManager->getTownHallLevel()) {
            // 未解锁 - 灰色按钮 + 文字提示
            _bottomButton->setTitleText("大本营" + std::to_string(theme.unlockTownHallLevel) + "级解锁");
            _bottomButton->setEnabled(false);
            _bottomButton->setBright(false);

            auto btnBg = LayerColor::create(Color4B(80, 80, 80, 255), btnSize.width, btnSize.height);
            btnBg->ignoreAnchorPointForPosition(false);
            btnBg->setAnchorPoint(Vec2::ZERO);
            btnBg->setPosition(Vec2::ZERO);
            _bottomButton->addChild(btnBg, -1);

            CCLOG("ThemeSwitchLayer: Button state - LOCKED");
        } else {
            // 可购买 - 使用 100 宝石购买按钮图片
            CCLOG("ThemeSwitchLayer: Loading purchase button image...");

            auto purchaseBtn = Sprite::create("UI/Village/spend_100_gem_btn.png");
            if (purchaseBtn) {
                purchaseBtn->setScale(1.0f);
                purchaseBtn->setAnchorPoint(Vec2(0.5f, 0.5f));
                purchaseBtn->setPosition(btnSize.width / 2, btnSize.height / 3);

                // ✅ 不缩放，使用原始大小（确保图片可见）
                CCLOG("ThemeSwitchLayer: Purchase button loaded, size=(%.0f, %.0f)",
                      purchaseBtn->getContentSize().width,
                      purchaseBtn->getContentSize().height);

                _bottomButton->addChild(purchaseBtn, 1);  // ✅ 使用正数 ZOrder

                // ✅ 调整按钮大小以匹配图片
                Size imgSize = purchaseBtn->getContentSize();
                _bottomButton->setContentSize(imgSize);

                // ✅ 重新设置触摸区域大小
                auto touchArea = _bottomButton->getChildByTag(-100);
                if (touchArea) {
                    auto layerColor = dynamic_cast<LayerColor*>(touchArea);
                    if (layerColor) {
                        layerColor->setContentSize(imgSize);
                    }
                }

            } else {
                CCLOG("ThemeSwitchLayer: ERROR - Failed to load purchase button image!");
                _bottomButton->setTitleText("购买 " + std::to_string(theme.gemCost) + " 宝石");

                // 添加可见背景
                auto btnBg = LayerColor::create(Color4B(200, 100, 0, 255), btnSize.width, btnSize.height);
                btnBg->ignoreAnchorPointForPosition(false);
                btnBg->setAnchorPoint(Vec2::ZERO);
                btnBg->setPosition(Vec2::ZERO);
                _bottomButton->addChild(btnBg, -1);
            }

            bool canAfford = dataManager->getGem() >= theme.gemCost;
            _bottomButton->setEnabled(canAfford);
            _bottomButton->setBright(canAfford);

            CCLOG("ThemeSwitchLayer: Can afford: %s (have %d, need %d)",
                  canAfford ? "YES" : "NO", dataManager->getGem(), theme.gemCost);

            if (!canAfford && purchaseBtn) {
                purchaseBtn->setColor(Color3B(100, 100, 100));
            }
        }
    } else {
        // ========== 已解锁 - 显示勾勾"确认选择"按钮 ==========
        CCLOG("ThemeSwitchLayer: Theme is purchased, showing check icon");

        auto checkIcon = Sprite::create("ImageElements/right.png");
        if (checkIcon) {
            checkIcon->setScale(0.5f);  // ✅ 放大图标使其更明显
            checkIcon->setAnchorPoint(Vec2(0.5f, 0.5f));
            checkIcon->setPosition(btnSize.width / 2, btnSize.height / 4);

            _bottomButton->addChild(checkIcon, 1);  // ✅ 使用正数 ZOrder

            CCLOG("ThemeSwitchLayer: Check icon added, size=(%.0f, %.0f)",
                  checkIcon->getContentSize().width * checkIcon->getScale(),
                  checkIcon->getContentSize().height * checkIcon->getScale());
        } else {
            CCLOG("ThemeSwitchLayer: ERROR - Failed to load check icon!");
            _bottomButton->setTitleText("确认选择");

            // 添加可见背景
            auto btnBg = LayerColor::create(Color4B(0, 200, 0, 255), btnSize.width, btnSize.height);
            btnBg->ignoreAnchorPointForPosition(false);
            btnBg->setAnchorPoint(Vec2::ZERO);
            btnBg->setPosition(Vec2::ZERO);
            _bottomButton->addChild(btnBg, -1);
        }

        _bottomButton->setEnabled(true);
        _bottomButton->setBright(true);
        _bottomButton->setTouchEnabled(true);
    }
}

void ThemeSwitchLayer::onPurchaseClicked() {
    auto dataManager = VillageDataManager::getInstance();
    auto theme = _themes[_currentSelectedTheme];

    if (dataManager->getGem() < theme.gemCost) {
        CCLOG("ThemeSwitchLayer: Not enough gems");
        return;
    }

    // 扣除宝石
    dataManager->spendGem(theme.gemCost);

    // 标记为已购买
    dataManager->purchaseTheme(_currentSelectedTheme);
    theme.isPurchased = true;
    _themes[_currentSelectedTheme] = theme;

    // 更新按钮
    updateBottomButton();

    CCLOG("ThemeSwitchLayer: Theme %d purchased", _currentSelectedTheme);
}

void ThemeSwitchLayer::onConfirmClicked() {
    auto dataManager = VillageDataManager::getInstance();
    auto theme = _themes[_currentSelectedTheme];

    // 保存选择
    dataManager->setCurrentTheme(_currentSelectedTheme);

    CCLOG("ThemeSwitchLayer: Switched to theme %d", _currentSelectedTheme);

    // 通知 VillageLayer 切换地图
    auto scene = Director::getInstance()->getRunningScene();
    auto villageScene = dynamic_cast<VillageScene*>(scene);
    if (villageScene) {
        auto villageLayer = dynamic_cast<VillageLayer*>(villageScene->getChildByTag(1));
        if (villageLayer) {
            villageLayer->switchMapBackground(_currentSelectedTheme);
        }
    }

    // 关闭面板
    onCloseClicked();
}

void ThemeSwitchLayer::onCloseClicked() {
    this->runAction(Sequence::create(
        ScaleTo::create(0.1f, 0.1f),
        RemoveSelf::create(),
        nullptr
    ));
}
