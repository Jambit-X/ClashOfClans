#include "BuildingSprite.h"
#include "../Model/BuildingConfig.h"
#include "../Component/ConstructionAnimation.h"

USING_NS_CC;

BuildingSprite* BuildingSprite::create(const BuildingInstance& building) {
    auto sprite = new BuildingSprite();
    if (sprite && sprite->init(building)) {
        sprite->autorelease();
        return sprite;
    }
    delete sprite;
    return nullptr;
}

bool BuildingSprite::init(const BuildingInstance& building) {
    _buildingId = building.id;
    _buildingType = building.type;
    _buildingLevel = building.level;
    _buildingState = building.state;
    _visualOffset = Vec2::ZERO;
    _progressBar = nullptr;
    _countdownLabel = nullptr;
    _constructionAnim = nullptr;  // 初始化
    _gridPos = Vec2(building.gridX, building.gridY);
    
    loadSprite(_buildingType, _buildingLevel);
    updateVisuals();
    
    // 如果是建造中状态，启动建造动画
    if (_buildingState == BuildingInstance::State::CONSTRUCTING) {
        startConstruction();
    }
    
    return true;
}

void BuildingSprite::updateBuilding(const BuildingInstance& building) {
    bool levelChanged = (_buildingLevel != building.level);
    bool stateChanged = (_buildingState != building.state);
    
    _buildingLevel = building.level;
    _buildingState = building.state;
    
    if (levelChanged) {
        loadSprite(_buildingType, _buildingLevel);
    }
    
    if (stateChanged) {
        updateVisuals();
        
        // 状态切换处理
        if (_buildingState == BuildingInstance::State::CONSTRUCTING) {
            startConstruction();
        } else if (_buildingState == BuildingInstance::State::BUILT) {
            finishConstruction();
        }
    }
}

void BuildingSprite::updateState(BuildingInstance::State state) {
    if (_buildingState == state) return;
    
    _buildingState = state;
    updateVisuals();
    
    if (_buildingState == BuildingInstance::State::CONSTRUCTING) {
        startConstruction();
    } else if (_buildingState == BuildingInstance::State::BUILT) {
        finishConstruction();
    }
}

void BuildingSprite::updateLevel(int level) {
  if (_buildingLevel == level) return;

  _buildingLevel = level;

  // 重新加载对应等级的精灵图
  loadSprite(_buildingType, _buildingLevel);

  CCLOG("BuildingSprite: Updated to level %d", level);
}

void BuildingSprite::startConstruction() {
    CCLOG("BuildingSprite: Starting construction animation for building ID=%d", _buildingId);
    
    if (!_constructionAnim) {
        _constructionAnim = new ConstructionAnimation(this);
    }
    _constructionAnim->start();
}

void BuildingSprite::updateConstructionProgress(float progress) {
    if (_constructionAnim) {
        _constructionAnim->updateProgress(progress);
    }
}

void BuildingSprite::finishConstruction() {
    CCLOG("BuildingSprite: Finishing construction for building ID=%d", _buildingId);
    
    if (_constructionAnim) {
        _constructionAnim->stop();
        delete _constructionAnim;
        _constructionAnim = nullptr;
    }
    
    hideConstructionProgress();
}

void BuildingSprite::showConstructionProgress(float progress) {
  if (!_progressBar) {
    // 创建一个 1x1 白色像素精灵作为进度条基础
    auto whitePixel = Sprite::create();
    whitePixel->setTextureRect(Rect(0, 0, 100, 12));
    whitePixel->setColor(Color3B(50, 205, 50)); // 绿色

    _progressBar = ProgressTimer::create(whitePixel);
    _progressBar->setType(ProgressTimer::Type::BAR);
    _progressBar->setMidpoint(Vec2(0, 0.5f));
    _progressBar->setBarChangeRate(Vec2(1, 0));

    auto spriteSize = this->getContentSize();
    _progressBar->setPosition(Vec2(spriteSize.width / 2, spriteSize.height + 30));

    // 添加背景条（使用 Tag 888 标识）
    auto bgBar = Sprite::create();
    bgBar->setTextureRect(Rect(0, 0, 100, 12));
    bgBar->setColor(Color3B(50, 50, 50));
    bgBar->setOpacity(200);
    bgBar->setPosition(Vec2(spriteSize.width / 2, spriteSize.height + 30));
    bgBar->setTag(888); // 添加 Tag 用于后续删除
    this->addChild(bgBar, 9);

    this->addChild(_progressBar, 10);

    CCLOG("BuildingSprite: Created simple solid progress bar");
  }

  _progressBar->setPercentage(progress * 100);
}

void BuildingSprite::hideConstructionProgress() {
  // 移除进度条
  if (_progressBar) {
    _progressBar->removeFromParent();
    _progressBar = nullptr;
  }

  // 移除背景条（通过 Tag）
  this->removeChildByTag(888);

  // 移除倒计时标签
  if (_countdownLabel) {
    _countdownLabel->removeFromParent();
    _countdownLabel = nullptr;
  }

  CCLOG("BuildingSprite: Progress bar, background and countdown removed");
}

void BuildingSprite::showCountdown(int seconds) {
    if (!_countdownLabel) {
        _countdownLabel = Label::createWithTTF("", "fonts/simhei.ttf", 18);
        _countdownLabel->setColor(Color3B::WHITE);
        _countdownLabel->enableOutline(Color4B::BLACK, 2);
        
        auto spriteSize = this->getContentSize();
        _countdownLabel->setPosition(Vec2(spriteSize.width / 2, spriteSize.height + 50));
        this->addChild(_countdownLabel, 11);
    }
    
    // 格式化时间显示
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;
    
    std::string timeStr;
    if (hours > 0) {
        timeStr = StringUtils::format("%02d:%02d:%02d", hours, minutes, secs);
    } else {
        timeStr = StringUtils::format("%02d:%02d", minutes, secs);
    }
    
    _countdownLabel->setString(timeStr);
}

void BuildingSprite::loadSprite(int type, int level) {
    auto config = BuildingConfig::getInstance();
    std::string spritePath = config->getSpritePath(type, level);
    
    if (spritePath.empty()) {
        CCLOG("BuildingSprite: ERROR - Empty sprite path for type=%d, level=%d", type, level);
        return;
    }
    
    auto texture = Director::getInstance()->getTextureCache()->addImage(spritePath);
    if (texture) {
        this->setTexture(texture);
        auto rect = Rect(0, 0, texture->getContentSize().width, texture->getContentSize().height);
        this->setTextureRect(rect);
        
        auto configData = config->getConfig(type);
        if (configData) {
            _visualOffset = configData->anchorOffset;
        }
        
        CCLOG("BuildingSprite: Loaded sprite for type=%d, level=%d", type, level);
    } else {
        CCLOG("BuildingSprite: ERROR - Failed to load texture: %s", spritePath.c_str());
    }
}

void BuildingSprite::updateVisuals() {
    // 根据状态调整透明度
    switch (_buildingState) {
        case BuildingInstance::State::PLACING:
            this->setOpacity(180);  // 半透明
            break;
        case BuildingInstance::State::CONSTRUCTING:
            this->setOpacity(255);
            // 建造动画会处理变黑效果
            break;
        case BuildingInstance::State::BUILT:
            this->setOpacity(255);
            this->setColor(Color3B::WHITE);
            break;
    }
}
// ========== 网格和拖动相关实现 ==========

cocos2d::Size BuildingSprite::getGridSize() const {
  auto config = BuildingConfig::getInstance()->getConfig(_buildingType);
  if (config) {
    return cocos2d::Size(config->gridWidth, config->gridHeight);
  }
  CCLOG("BuildingSprite::getGridSize - Config not found for type=%d, using default 3x3", _buildingType);
  return cocos2d::Size(3, 3);  // 默认大小
}

void BuildingSprite::setDraggingMode(bool isDragging) {
  if (isDragging) {
    // 拖动时：放大 + 半透明
    // this->setScale(1.1f);
    this->setOpacity(220);
    CCLOG("BuildingSprite: Entering dragging mode");
  } else {
    // 恢复正常
    this->setScale(1.0f);
    this->setOpacity(255);
    this->setColor(cocos2d::Color3B::WHITE);
    CCLOG("BuildingSprite: Exiting dragging mode");
  }
}

void BuildingSprite::setPlacementPreview(bool isValid) {
  if (isValid) {
    // 可放置：绿色高亮
    this->setColor(cocos2d::Color3B(100, 255, 100));
    this->setOpacity(220);
  } else {
    // 不可放置：红色高亮
    this->setColor(cocos2d::Color3B(255, 100, 100));
    this->setOpacity(220);
  }
}