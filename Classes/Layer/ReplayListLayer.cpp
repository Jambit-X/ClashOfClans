// Layer/ReplayListLayer.cpp
#pragma execution_character_set("utf-8")  // ✅ 新增：强制UTF-8编码

#include "ReplayListLayer.h"
#include "Manager/ReplayManager.h"
#include "Scene/BattleScene.h"

USING_NS_CC;

// ✅ 新增：统一字体路径
const std::string FONT_PATH = "fonts/simhei.ttf";

bool ReplayListLayer::init() {
    if (!Layer::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // 1. 半透明背景遮罩（✅ 修复：添加触摸事件吞噬）
    auto bgMask = LayerColor::create(Color4B(0, 0, 0, 180));
    this->addChild(bgMask, -1);  // ✅ 放在最底层

    // ✅ 新增：背景遮罩吞噬触摸事件，防止穿透到下层
    auto bgListener = EventListenerTouchOneByOne::create();
    bgListener->setSwallowTouches(true);
    bgListener->onTouchBegan = [](Touch*, Event*) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(bgListener, bgMask);

    // 2. 主面板背景（使用您的 UI 资源）
    auto panel = ui::Scale9Sprite::create("UI/replay/replay_list_bg.png");
    panel->setPosition(Vec2(visibleSize.width / 2 + origin.x,
                       visibleSize.height / 2 + origin.y));
    this->addChild(panel);

    // ✅ 获取面板实际尺寸（原始图片尺寸 801×448）
    Size panelSize = panel->getContentSize();
    CCLOG("ReplayListLayer: Panel size = %.0f x %.0f", panelSize.width, panelSize.height);

    // 4. 关闭按钮
    auto closeBtn = ui::Button::create("UI/replay/close_btn.png");
    closeBtn->setPosition(Vec2(panelSize.width - 60, panelSize.height - 40));
    closeBtn->setScale(0.8f);
    closeBtn->addClickEventListener([this](Ref*) {
        CCLOG("ReplayListLayer: Close button clicked!");
        onCloseClicked();
    });
    panel->addChild(closeBtn, 100);

    CCLOG("ReplayListLayer: Close button created at (%.0f, %.0f)",
          closeBtn->getPosition().x, closeBtn->getPosition().y);

    // ✅ 5. 创建滚动视图（精确放置在指定矩形区域）
    // 矩形区域定义：左下角(32, 40)，右上角(944, 475)
    float scrollX = 32.0f;
    float scrollY = 40.0f;
    float scrollWidth = 944.0f - 32.0f;   // = 912
    float scrollHeight = 475.0f - 40.0f;  // = 435

    _scrollView = ui::ScrollView::create();
    _scrollView->setDirection(ui::ScrollView::Direction::VERTICAL);
    _scrollView->setContentSize(Size(scrollWidth, scrollHeight));
    _scrollView->setPosition(Vec2(scrollX, scrollY));  // ✅ 左下角锚点
    _scrollView->setBounceEnabled(true);
    _scrollView->setScrollBarEnabled(false);
    panel->addChild(_scrollView);

    CCLOG("ReplayListLayer: ScrollView created at (%.0f, %.0f) with size (%.0f x %.0f)",
          scrollX, scrollY, scrollWidth, scrollHeight);

    // 6. 创建内容容器
    _contentNode = Node::create();
    _scrollView->addChild(_contentNode);

    // 7. 加载回放列表
    loadReplayList();

    return true;
}

void ReplayListLayer::loadReplayList() {
    auto replayList = ReplayManager::getInstance()->getReplayList();

    if (replayList.empty()) {
        // 空状态提示（✅ 改用 TTF）
        auto emptyLabel = Label::createWithTTF(
            "暂无战斗回放\n去打一场战斗吧！",
            FONT_PATH, 36
        );
        emptyLabel->setPosition(_scrollView->getContentSize() / 2);
        emptyLabel->setColor(Color3B::GRAY);
        emptyLabel->setAlignment(TextHAlignment::CENTER);
        _scrollView->addChild(emptyLabel);
        return;
    }

    // 倒序显示（最新的在上面）
    std::reverse(replayList.begin(), replayList.end());

    // ✅ 计算总高度（卡片数量 * 单卡高度 + 间距）
    float totalHeight = replayList.size() * (CARD_HEIGHT + CARD_SPACING);
    _contentNode->setContentSize(Size(_scrollView->getContentSize().width, totalHeight));
    _scrollView->setInnerContainerSize(Size(_scrollView->getContentSize().width, totalHeight));

    CCLOG("ReplayListLayer: Total content height = %.0f for %zu replays",
          totalHeight, replayList.size());

    // ✅ 创建每张卡片（从顶部开始排列）
    float yPos = totalHeight - CARD_HEIGHT / 2 - 10;  // 从顶部开始，留10像素边距
    for (const auto& replay : replayList) {
        createReplayCard(replay, yPos);
        yPos -= (CARD_HEIGHT + CARD_SPACING);
    }
}

void ReplayListLayer::createReplayCard(const ReplayMetadata& replay, float yPosition) {
    // ✅ 获取滚动视图的宽度，卡片自动适配
    float scrollWidth = _scrollView->getContentSize().width;
    float cardWidth = scrollWidth - 20;  // 左右各留10像素边距

    // 1. 卡片背景（横向长方形）
    auto cardBg = ui::Scale9Sprite::create("UI/replay/card_bg.png");
    cardBg->setContentSize(Size(cardWidth, CARD_HEIGHT));
    cardBg->setPosition(Vec2(scrollWidth / 2, yPosition));  // ✅ 居中对齐
    _contentNode->addChild(cardBg);

    // ========== 左侧：星星和摧毁率 ==========
    float leftX = 60;  // ✅ 调整位置适应新宽度
    float topY = CARD_HEIGHT - 30;

    // 星星图标（3个，亮/暗）
    for (int i = 0; i < 3; ++i) {
        std::string starIcon;
        if (i < replay.finalStars) {
            starIcon = "UI/battle/battle-prepare/victory_star.png";
        } else {
            starIcon = "UI/battle/battle-prepare/victory_star_bg.png";
        }

        auto star = Sprite::create(starIcon);
        star->setScale(0.3f);  // ✅ 缩小星星适应新宽度
        star->setPosition(Vec2(leftX + i * 40, topY));
        cardBg->addChild(star);
    }

    // 摧毁率
    auto destructionLabel = Label::createWithTTF(
        StringUtils::format("%d%%", replay.destructionPercentage),
        FONT_PATH, 28
    );
    destructionLabel->setPosition(Vec2(leftX + 80, topY - 50));
    destructionLabel->setColor(Color3B::WHITE);
    cardBg->addChild(destructionLabel);

    // ========== 中部：资源和时间 ==========
    float midX = 280;  // ✅ 调整位置

    // 金币图标
    auto goldIcon = Sprite::create("ImageElements/coin_icon.png");
    goldIcon->setScale(0.5f);
    goldIcon->setPosition(Vec2(midX, topY));
    cardBg->addChild(goldIcon);

    // 金币数量
    auto goldLabel = Label::createWithTTF(
        StringUtils::format("%d", replay.lootedGold),
        FONT_PATH, 24
    );
    goldLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
    goldLabel->setPosition(Vec2(midX + 25, topY));
    goldLabel->setColor(Color3B::WHITE);
    cardBg->addChild(goldLabel);

    // 圣水图标
    auto elixirIcon = Sprite::create("ImageElements/elixir_icon.png");
    elixirIcon->setScale(0.5f);
    elixirIcon->setPosition(Vec2(midX + 140, topY));
    cardBg->addChild(elixirIcon);

    // 圣水数量
    auto elixirLabel = Label::createWithTTF(
        StringUtils::format("%d", replay.lootedElixir),
        FONT_PATH, 24
    );
    elixirLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
    elixirLabel->setPosition(Vec2(midX + 165, topY));
    elixirLabel->setColor(Color3B::WHITE);
    cardBg->addChild(elixirLabel);

    // 奖杯图标（固定为0）
    auto trophyIcon = Sprite::create("ImageElements/trophy_icon.png");
    trophyIcon->setScale(0.5f);
    trophyIcon->setPosition(Vec2(midX + 280, topY));
    cardBg->addChild(trophyIcon);

    // 奖杯数量
    auto trophyLabel = Label::createWithTTF("0", FONT_PATH, 24);
    trophyLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
    trophyLabel->setPosition(Vec2(midX + 305, topY));
    trophyLabel->setColor(Color3B::WHITE);
    cardBg->addChild(trophyLabel);

    // 时间戳
    auto timeLabel = Label::createWithTTF(
        getTimeAgo(replay.timestamp),
        FONT_PATH, 20
    );
    timeLabel->setPosition(Vec2(midX + 150, topY - 50));
    timeLabel->setColor(Color3B::GRAY);
    cardBg->addChild(timeLabel);

    // ========== 底部：兵种消耗列表 ==========
    float troopStartX = 60;
    float troopY = 30;
    int troopIndex = 0;

    for (const auto& pair : replay.usedTroops) {
        int troopId = pair.first;
        int count = pair.second;

        if (count <= 0) continue;

        // 兵种图标
        std::string iconPath = getTroopIconPath(troopId);
        auto troopIcon = Sprite::create(iconPath);
        if (troopIcon) {
            troopIcon->setScale(0.4f);  // ✅ 缩小图标
            troopIcon->setPosition(Vec2(troopStartX + troopIndex * 80, troopY));
            cardBg->addChild(troopIcon);

            // 数量标签
            auto countLabel = Label::createWithTTF(
                StringUtils::format("x%d", count),
                FONT_PATH, 18
            );
            countLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
            countLabel->setPosition(Vec2(troopStartX + troopIndex * 80 + 25, troopY));
            countLabel->setColor(Color3B::WHITE);
            cardBg->addChild(countLabel);

            troopIndex++;
        }
    }

    // ========== 右侧：回放按钮 ==========
    auto replayBtn = ui::Button::create("UI/replay/replay_btn.png");
    replayBtn->setScale(0.6f);  // ✅ 缩小按钮
    replayBtn->setPosition(Vec2(cardWidth - 80, CARD_HEIGHT / 2));
    replayBtn->addClickEventListener([this, replay](Ref*) {
        onWatchClicked(replay.replayId);
    });
    cardBg->addChild(replayBtn);

    // ========== 右上角：删除按钮 ==========
    auto deleteBtn = ui::Button::create("UI/replay/close_btn.png");
    deleteBtn->setScale(0.4f);  // ✅ 缩小删除按钮
    deleteBtn->setPosition(Vec2(cardWidth - 30, CARD_HEIGHT - 25));
    deleteBtn->addClickEventListener([this, replay](Ref*) {
        onDeleteClicked(replay.replayId);
    });
    cardBg->addChild(deleteBtn);
}

// ========== 事件处理 ==========

void ReplayListLayer::onWatchClicked(int replayId) {
    CCLOG("ReplayListLayer: Watching replay #%d", replayId);

    // 加载回放数据
    auto replayData = ReplayManager::getInstance()->loadReplay(replayId);

    if (replayData.troopEvents.empty()) {
        CCLOG("ReplayListLayer: ERROR - Failed to load replay data");
        return;
    }

    // 创建回放场景
    auto replayScene = BattleScene::createReplayScene(replayData);
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, replayScene));
}

void ReplayListLayer::onDeleteClicked(int replayId) {
    CCLOG("ReplayListLayer: Deleting replay #%d", replayId);

    // 创建确认弹窗
    auto confirmBg = LayerColor::create(Color4B(0, 0, 0, 200));

    // 确认提示
    auto confirmLabel = Label::createWithTTF(
        "确定要删除这场回放吗？",
        FONT_PATH, 36
    );
    confirmLabel->setPosition(Director::getInstance()->getVisibleSize() / 2 + Size(0, 60));
    confirmBg->addChild(confirmLabel);

    // 确认按钮
    auto yesBtn = ui::Button::create("UI/common/btn_yes.png");
    yesBtn->setPosition(Director::getInstance()->getVisibleSize() / 2 + Size(-100, -40));
    yesBtn->addClickEventListener([this, replayId, confirmBg](Ref*) {
        // 删除回放
        ReplayManager::getInstance()->deleteReplay(replayId);

        // 关闭确认框
        confirmBg->removeFromParent();

        // 重新加载列表
        _contentNode->removeAllChildren();
        loadReplayList();
    });
    confirmBg->addChild(yesBtn);

    // 取消按钮
    auto noBtn = ui::Button::create("UI/common/btn_no.png");
    noBtn->setPosition(Director::getInstance()->getVisibleSize() / 2 + Size(100, -40));
    noBtn->addClickEventListener([confirmBg](Ref*) {
        confirmBg->removeFromParent();
    });
    confirmBg->addChild(noBtn);

    this->addChild(confirmBg, 10);
}

void ReplayListLayer::onCloseClicked() {
    CCLOG("ReplayListLayer: onCloseClicked() called, removing layer");
    this->removeFromParent();
}

// ========== 辅助方法 ==========

std::string ReplayListLayer::getTimeAgo(time_t timestamp) {
    time_t now = time(nullptr);
    int diff = static_cast<int>(now - timestamp);

    if (diff < 60) return "刚刚";
    if (diff < 3600) return StringUtils::format("%d分钟前", diff / 60);
    if (diff < 86400) return StringUtils::format("%d小时前", diff / 3600);
    return StringUtils::format("%d天前", diff / 86400);
}

std::string ReplayListLayer::getTroopIconPath(int troopId) {
    // 兵种ID到图标路径的映射
    switch (troopId) {
        case 1001: return "UI/troop/barbarian_icon.png";
        case 1002: return "UI/troop/archer_icon.png";
        case 1003: return "UI/troop/goblin_icon.png";
        case 1004: return "UI/troop/giant_icon.png";
        case 1005: return "UI/troop/wall_breaker_icon.png";
        case 1006: return "UI/troop/balloon_icon.png";
        default:   return "UI/troop/unknown_icon.png";
    }
}
