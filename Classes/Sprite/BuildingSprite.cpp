#include "BuildingSprite.h"
#include "Model/BuildingConfig.h" // 确保包含头文件

USING_NS_CC;

BuildingSprite* BuildingSprite::create(const BuildingInstance& building) {
    BuildingSprite* sprite = new (std::nothrow) BuildingSprite();
    if (sprite && sprite->init(building)) {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

bool BuildingSprite::init(const BuildingInstance& building) {
    // 获取建筑配置
    auto config = BuildingConfig::getInstance()->getConfig(building.type);
    if (!config) {
        CCLOG("BuildingSprite: Config not found for type %d", building.type);
        return false;
    }

    // 获取精灵路径
    std::string spritePath = BuildingConfig::getInstance()->getSpritePath(
        building.type, building.level);

    // 初始化 Sprite
    if (!Sprite::initWithFile(spritePath)) {
        CCLOG("BuildingSprite: Failed to load sprite: %s", spritePath.c_str());
        return false;
    }

    // 设置锚点为左下角，与网格坐标系统一致
    this->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);

    // 缩放建筑大小到原来的 70%
    _normalScale = 0.7f;
    this->setScale(_normalScale);

    // 存储数据
    _buildingId = building.id;
    _buildingType = building.type;
    _level = building.level;
    _state = building.state;
    _gridPos = Vec2(building.gridX, building.gridY);
    _gridSize = Size(config->gridWidth, config->gridHeight);
    
    // 存储视觉偏移量（用于修正显示位置）
    _visualOffset = config->anchorOffset;

    // 初始化视觉元素
    initVisualElements();

    // 根据状态显示进度
    if (building.state == BuildingInstance::State::CONSTRUCTING) {
        showConstructionProgress(0.0f);  // 初始进度
    }

    CCLOG("BuildingSprite: Created type=%d, level=%d, id=%d at grid(%d,%d)",
        _buildingType, _level, _buildingId, building.gridX, building.gridY);

    return true;
}

void BuildingSprite::initVisualElements() {
    // 1. 创建阴影
    createShadow();

    // 2. 创建等级标签
    createLevelLabel();

    // 3. 创建进度条（默认隐藏）
    createProgressBar();

    // 4. 创建选中框（默认隐藏）
    createSelectionBox();
}

void BuildingSprite::createShadow() {
    _shadowSprite = Sprite::create("UI/Common/shadow.png");
    if (_shadowSprite) {
        _shadowSprite->setPosition(Vec2(this->getContentSize().width / 2, 0));
        _shadowSprite->setOpacity(128);  // 半透明
        this->addChild(_shadowSprite, -1);  // 在建筑下层
    }
}

void BuildingSprite::createLevelLabel() {
    _levelLabel = Label::createWithSystemFont(
        StringUtils::format("Lv.%d", _level),
        "Arial",
        18
    );

    if (_levelLabel) {
        _levelLabel->setPosition(Vec2(30, this->getContentSize().height - 20));
        _levelLabel->setColor(Color3B::YELLOW);
        _levelLabel->enableOutline(Color4B::BLACK, 2);
        this->addChild(_levelLabel, 10);
    }
}

void BuildingSprite::createProgressBar() {
    // 进度条背景
    _progressBg = Sprite::create("UI/Common/progress_bg.png");
    if (_progressBg) {
        _progressBg->setPosition(Vec2(
            this->getContentSize().width / 2,
            this->getContentSize().height + 20
        ));
        _progressBg->setVisible(false);
        this->addChild(_progressBg, 15);

        // 进度条前景
        auto progressSprite = Sprite::create("UI/Common/progress_fill.png");
        _progressBar = ProgressTimer::create(progressSprite);
        _progressBar->setType(ProgressTimer::Type::BAR);
        _progressBar->setMidpoint(Vec2(0, 0.5f));
        _progressBar->setBarChangeRate(Vec2(1, 0));
        _progressBar->setPercentage(0);
        _progressBar->setPosition(_progressBg->getContentSize() / 2);
        _progressBg->addChild(_progressBar);
    }
}

void BuildingSprite::createSelectionBox() {
    _selectionBox = Sprite::create("UI/Common/selection_box.png");
    if (_selectionBox) {
        _selectionBox->setPosition(this->getContentSize() / 2);
        _selectionBox->setVisible(false);
        this->addChild(_selectionBox, 5);
    }
}

// ========== 更新方法 ==========

void BuildingSprite::updateBuilding(const BuildingInstance& building) {
    // 更新等级
    if (building.level != _level) {
        updateLevel(building.level);
    }

    // 更新状态
    if (building.state != _state) {
        updateState(building.state);
    }

    // 更新位置
    _gridPos = Vec2(building.gridX, building.gridY);
}

void BuildingSprite::updateLevel(int newLevel) {
    _level = newLevel;

    // 更新纹理
    updateSpriteTexture();

    // 更新等级标签
    if (_levelLabel) {
        _levelLabel->setString(StringUtils::format("Lv.%d", _level));
    }

    CCLOG("BuildingSprite: Level updated to %d", _level);
}

void BuildingSprite::updateState(BuildingInstance::State newState) {
    _state = newState;

    switch (newState) {
    case BuildingInstance::State::PLACING:
        setPlacementPreview(true);
        break;

    case BuildingInstance::State::CONSTRUCTING:
        showConstructionProgress(0.0f);
        break;

    case BuildingInstance::State::BUILT:
        hideConstructionProgress();
        setOpacity(255);  // 完全不透明
        break;
    }
}

void BuildingSprite::updateSpriteTexture() {
    std::string newPath = BuildingConfig::getInstance()->getSpritePath(
        _buildingType, _level);

    auto texture = Director::getInstance()->getTextureCache()->addImage(newPath);
    if (texture) {
        this->setTexture(texture);
    }
}

// ========== 显示模式 ==========

void BuildingSprite::setDraggingMode(bool dragging) {
    if (dragging) {
        this->setOpacity(180);  // 半透明
        this->setScale(_normalScale * 1.1f);   // 在原始基础上放大 10%
    }
    else {
        this->setOpacity(255);
        this->setScale(_normalScale);   // 恢复到原始 scale
    }
}

void BuildingSprite::setPlacementPreview(bool valid) {
    // 绿色表示可放置，红色表示不可放置
    if (valid) {
        this->setColor(Color3B(100, 255, 100));  // 浅绿色
        this->setOpacity(200);
    }
    else {
        this->setColor(Color3B(255, 100, 100));  // 浅红色
        this->setOpacity(200);
    }
}

void BuildingSprite::setSelected(bool selected) {
    if (_selectionBox) {
        _selectionBox->setVisible(selected);

        // 添加脉冲动画
        if (selected) {
            auto scaleUp = ScaleTo::create(0.5f, 1.1f);
            auto scaleDown = ScaleTo::create(0.5f, 1.0f);
            auto seq = Sequence::create(scaleUp, scaleDown, nullptr);
            _selectionBox->runAction(RepeatForever::create(seq));
        }
        else {
            _selectionBox->stopAllActions();
            _selectionBox->setScale(1.0f);
        }
    }
}

// ========== 进度显示 ==========

void BuildingSprite::showConstructionProgress(float progress) {
    if (_progressBg && _progressBar) {
        _progressBg->setVisible(true);
        _progressBar->setPercentage(progress * 100.0f);
    }
}

void BuildingSprite::hideConstructionProgress() {
    if (_progressBg) {
        _progressBg->setVisible(false);
    }
}

void BuildingSprite::showCountdown(int seconds) {
    if (!_countdownLabel) {
        _countdownLabel = Label::createWithSystemFont("", "Arial", 16);
        _countdownLabel->setPosition(Vec2(
            this->getContentSize().width / 2,
            this->getContentSize().height + 40
        ));
        this->addChild(_countdownLabel, 20);
    }

    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;

    std::string timeStr = StringUtils::format("%02d:%02d:%02d", hours, minutes, secs);
    _countdownLabel->setString(timeStr);
    _countdownLabel->setVisible(true);
}

// ========== 访问器 ==========

void BuildingSprite::setGridPos(const Vec2& pos) {
    _gridPos = pos;
}

// ========== 碰撞检测 ==========

bool BuildingSprite::containsPoint(const Vec2& point) const {
    return getBoundingBox().containsPoint(point);
}

Rect BuildingSprite::getBoundingBox() const {
    return Sprite::getBoundingBox();
}
