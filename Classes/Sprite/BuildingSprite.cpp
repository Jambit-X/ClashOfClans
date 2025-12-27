// BuildingSprite.cpp
// 建筑精灵类实现，管理建筑的渲染、状态、动画和交互效果

#include "BuildingSprite.h"
#include "../Model/BuildingConfig.h"
#include "Component/HealthBarComponent.h"

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
  _gridPos = Vec2(building.gridX, building.gridY);

  _constructionUIContainer = nullptr;
  _progressBg = nullptr;
  _progressBar = nullptr;
  _countdownLabel = nullptr;
  _percentLabel = nullptr;

  _selectionGlow = nullptr;
  _isSelected = false;

  loadSprite(_buildingType, _buildingLevel);

  initConstructionUI();

  updateVisuals();

  if (_buildingState == BuildingInstance::State::CONSTRUCTING) {
    startConstruction();
  }

  return true;
}

void BuildingSprite::initConstructionUI() {
  _constructionUIContainer = Node::create();
  _constructionUIContainer->setVisible(false);
  this->addChild(_constructionUIContainer, 100);

  auto spriteSize = this->getContentSize();

  // 创建灰色进度条背景
  _progressBg = Sprite::create();
  _progressBg->setTextureRect(Rect(0, 0, 100, 12));
  _progressBg->setColor(Color3B(50, 50, 50));
  _progressBg->setOpacity(200);
  _progressBg->setPosition(Vec2(spriteSize.width / 2, spriteSize.height + 30));
  _constructionUIContainer->addChild(_progressBg, 1);

  // 创建绿色进度条前景
  auto progressSprite = Sprite::create();
  progressSprite->setTextureRect(Rect(0, 0, 100, 12));
  progressSprite->setColor(Color3B(50, 205, 50));

  _progressBar = ProgressTimer::create(progressSprite);
  _progressBar->setType(ProgressTimer::Type::BAR);
  _progressBar->setMidpoint(Vec2(0, 0.5f));
  _progressBar->setBarChangeRate(Vec2(1, 0));
  _progressBar->setPosition(Vec2(spriteSize.width / 2, spriteSize.height + 30));
  _constructionUIContainer->addChild(_progressBar, 2);

  // 创建倒计时文本标签
  _countdownLabel = Label::createWithTTF("", "fonts/simhei.ttf", 18);
  _countdownLabel->setColor(Color3B::WHITE);
  _countdownLabel->enableOutline(Color4B::BLACK, 2);
  _countdownLabel->setPosition(Vec2(spriteSize.width / 2, spriteSize.height + 50));
  _constructionUIContainer->addChild(_countdownLabel, 3);

  // 创建百分比文本标签
  _percentLabel = Label::createWithTTF("", "fonts/simhei.ttf", 20);
  _percentLabel->setColor(Color3B::YELLOW);
  _percentLabel->enableOutline(Color4B::BLACK, 2);
  _percentLabel->setPosition(Vec2(spriteSize.width / 2, spriteSize.height + 70));
  _constructionUIContainer->addChild(_percentLabel, 4);

  CCLOG("BuildingSprite: Construction UI container initialized (ID=%d)", _buildingId);
}

void BuildingSprite::showConstructionUI() {
  if (_constructionUIContainer) {
    _constructionUIContainer->setVisible(true);
  }
}

void BuildingSprite::hideConstructionUI() {
  if (_constructionUIContainer) {
    _constructionUIContainer->setVisible(false);
  }
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

    if (_buildingState == BuildingInstance::State::CONSTRUCTING) {
      startConstruction();
    } else if (_buildingState == BuildingInstance::State::BUILT) {
      finishConstruction();
    }
  }
}

void BuildingSprite::updateState(BuildingInstance::State state) {
  if (_buildingState == state) return;

  BuildingInstance::State oldState = _buildingState;
  _buildingState = state;

  if (oldState == BuildingInstance::State::CONSTRUCTING) {
    hideConstructionUI();
    this->setColor(Color3B::WHITE);
  }

  updateVisuals();

  if (_buildingState == BuildingInstance::State::CONSTRUCTING) {
    startConstruction();
  } else if (_buildingState == BuildingInstance::State::BUILT) {
    finishConstruction();
  }

  CCLOG("BuildingSprite: State changed %d → %d (ID=%d)",
        (int)oldState, (int)_buildingState, _buildingId);
}

void BuildingSprite::updateLevel(int level) {
  if (_buildingLevel == level) return;

  _buildingLevel = level;
  loadSprite(_buildingType, _buildingLevel);

  CCLOG("BuildingSprite: Updated to level %d", level);
}

void BuildingSprite::startConstruction() {
  CCLOG("BuildingSprite: Starting construction (ID=%d)", _buildingId);

  showConstructionUI();

  this->setColor(Color3B(100, 100, 100));
}

void BuildingSprite::updateConstructionProgress(float progress) {
  if (_progressBar) {
    _progressBar->setPercentage(progress * 100);
  }

  if (_percentLabel) {
    std::string text = StringUtils::format("建造中...%.0f%%", progress * 100);
    _percentLabel->setString(text);
  }
}

void BuildingSprite::finishConstruction() {
  CCLOG("BuildingSprite: Finishing construction (ID=%d)", _buildingId);

  hideConstructionUI();

  this->setColor(Color3B::WHITE);
}

void BuildingSprite::showConstructionProgress(float progress) {
  showConstructionUI();
  updateConstructionProgress(progress);
}

void BuildingSprite::hideConstructionProgress() {
  hideConstructionUI();
  CCLOG("BuildingSprite: Construction UI hidden (ID=%d)", _buildingId);
}

void BuildingSprite::showCountdown(int seconds) {
  if (!_countdownLabel) return;

  // 将秒数转换为时分秒格式
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
  // 根据建筑状态更新视觉效果
  switch (_buildingState) {
    case BuildingInstance::State::PLACING:
      this->setOpacity(180);
      break;
    case BuildingInstance::State::CONSTRUCTING:
      this->setOpacity(255);
      break;
    case BuildingInstance::State::BUILT:
      this->setOpacity(255);
      this->setColor(Color3B::WHITE);
      break;
  }
}

void BuildingSprite::clearPlacementPreview() {
  this->setColor(Color3B::WHITE);
  this->setOpacity(255);
}

cocos2d::Size BuildingSprite::getGridSize() const {
  auto config = BuildingConfig::getInstance()->getConfig(_buildingType);
  if (config) {
    return cocos2d::Size(config->gridWidth, config->gridHeight);
  }
  return cocos2d::Size(3, 3);
}

void BuildingSprite::setDraggingMode(bool isDragging) {
  if (isDragging) {
    this->setOpacity(220);
  } else {
    this->setScale(1.0f);
    this->setOpacity(255);
    this->setColor(Color3B::WHITE);
  }
}

void BuildingSprite::setPlacementPreview(bool isValid) {
  if (isValid) {
    // 绿色表示可放置
    this->setColor(cocos2d::Color3B(100, 255, 100));
    this->setOpacity(220);
  } else {
    // 红色表示不可放置
    this->setColor(cocos2d::Color3B(255, 100, 100));
    this->setOpacity(220);
  }
}

void BuildingSprite::createSelectionGlow() {
  if (_selectionGlow) return;

  _selectionGlow = DrawNode::create();
  
  auto config = BuildingConfig::getInstance()->getConfig(_buildingType);
  if (!config) return;
  
  const float GRID_UNIT = 28.0f;
  float ellipseWidth = config->gridWidth * GRID_UNIT * 1.4f;
  float ellipseHeight = config->gridHeight * GRID_UNIT * 0.7f;
  
  // 绘制外层光晕椭圆
  const int segments = 48;
  std::vector<Vec2> outerPoints;
  float outerWidth = ellipseWidth * 1.3f;
  float outerHeight = ellipseHeight * 1.3f;
  for (int i = 0; i < segments; ++i) {
    float angle = (float)i / segments * 2.0f * M_PI;
    float x = outerWidth * 0.5f * cosf(angle);
    float y = outerHeight * 0.5f * sinf(angle);
    outerPoints.push_back(Vec2(x, y));
  }
  
  _selectionGlow->drawPolygon(
    outerPoints.data(),
    segments,
    Color4F(1.0f, 0.7f, 0.1f, 0.15f),
    3.0f,
    Color4F(1.0f, 0.8f, 0.2f, 0.4f)
  );
  
  // 绘制内层光圈椭圆
  std::vector<Vec2> innerPoints;
  for (int i = 0; i < segments; ++i) {
    float angle = (float)i / segments * 2.0f * M_PI;
    float x = ellipseWidth * 0.5f * cosf(angle);
    float y = ellipseHeight * 0.5f * sinf(angle);
    innerPoints.push_back(Vec2(x, y));
  }
  
  _selectionGlow->drawPolygon(
    innerPoints.data(),
    segments,
    Color4F(1.0f, 0.8f, 0.2f, 0.45f),
    4.0f,
    Color4F(1.0f, 0.9f, 0.3f, 0.9f)
  );
  
  auto spriteSize = this->getContentSize();
  _selectionGlow->setPosition(Vec2(spriteSize.width / 2, ellipseHeight * 0.4f));
  
  this->addChild(_selectionGlow, -1);
  _selectionGlow->setVisible(false);
  
  CCLOG("BuildingSprite: Selection glow created (ID=%d, size=%.0fx%.0f)", 
        _buildingId, ellipseWidth, ellipseHeight);
}

void BuildingSprite::showSelectionEffect() {
  if (_isSelected) return;
  _isSelected = true;
  
  // 播放弹跳缩放动画
  this->stopActionByTag(100);
  auto bounceUp = ScaleTo::create(0.1f, 1.08f);
  auto bounceDown = ScaleTo::create(0.1f, 1.0f);
  auto bounce = Sequence::create(bounceUp, bounceDown, nullptr);
  bounce->setTag(100);
  this->runAction(bounce);
  
  if (!_selectionGlow) {
    createSelectionGlow();
  }
  
  if (_selectionGlow) {
    _selectionGlow->setVisible(true);
    _selectionGlow->setOpacity(255);
    
    // 播放光圈呼吸脉冲动画
    _selectionGlow->stopActionByTag(101);
    auto fadeOut = FadeTo::create(0.6f, 150);
    auto fadeIn = FadeTo::create(0.6f, 255);
    auto pulse = RepeatForever::create(Sequence::create(fadeOut, fadeIn, nullptr));
    pulse->setTag(101);
    _selectionGlow->runAction(pulse);
  }
  
  CCLOG("BuildingSprite: Selection effect shown (ID=%d)", _buildingId);
}

void BuildingSprite::hideSelectionEffect() {
  if (!_isSelected) return;
  _isSelected = false;
  
  this->stopActionByTag(100);
  this->setScale(1.0f);
  
  if (_selectionGlow) {
    _selectionGlow->stopActionByTag(101);
    _selectionGlow->setVisible(false);
  }
  
  CCLOG("BuildingSprite: Selection effect hidden (ID=%d)", _buildingId);
}

void BuildingSprite::updateHealthBar(int currentHP, int maxHP) {
    if (!_healthBar) {
        // 首次创建血条，根据建筑网格宽度计算血条宽度
        auto config = BuildingConfig::getInstance()->getConfig(_buildingType);
        int gridWidth = config ? config->gridWidth : 2;

        HealthBarComponent::Config barConfig;
        barConfig.width = std::max(40.0f, std::min(120.0f, gridWidth * 30.0f));
        barConfig.height = 8.0f;
        barConfig.offset = Vec2(0, 15);
        barConfig.highThreshold = 50.0f;
        barConfig.mediumThreshold = 25.0f;
        barConfig.showWhenFull = false;

        _healthBar = HealthBarComponent::create(barConfig);
        this->addChild(_healthBar, 99);

        _healthBar->updatePosition(this->getContentSize());
    }

    _healthBar->updateHealth(currentHP, maxHP);
}

void BuildingSprite::showDestroyedRubble() {
    if (_isShowingRubble) return;
    _isShowingRubble = true;

    // 获取建筑网格尺寸
    auto config = BuildingConfig::getInstance()->getConfig(_buildingType);
    int gridWidth = config ? config->gridWidth : 2;
    int gridHeight = config ? config->gridHeight : 2;
    
    int maxSize = std::max(gridWidth, gridHeight);
    
    // 根据建筑大小选择废墟贴图
    std::string rubblePath;
    if (maxSize <= 1) {
        rubblePath = "buildings/broken/broken1x1.png";
    } else if (maxSize <= 2) {
        rubblePath = "buildings/broken/broken2x2.png";
    } else {
        rubblePath = "buildings/broken/broken3x3.png";
    }
    
    this->setVisible(true);
    this->setOpacity(255);
    
    // 加载并显示废墟纹理
    auto texture = Director::getInstance()->getTextureCache()->addImage(rubblePath);
    if (texture) {
        this->setTexture(texture);
        auto rect = Rect(0, 0, texture->getContentSize().width, texture->getContentSize().height);
        this->setTextureRect(rect);
        
        this->setColor(Color3B::WHITE);
        
        CCLOG("BuildingSprite: Showing rubble for building ID=%d, size=%dx%d, path=%s", 
              _buildingId, gridWidth, gridHeight, rubblePath.c_str());
    } else {
        // 加载失败时显示红色占位符
        this->setColor(Color3B::RED);
        this->setOpacity(200);
        CCLOG("BuildingSprite: Failed to load rubble texture: %s", rubblePath.c_str());
    }
    
    // 隐藏防御动画
    auto defenseAnim = this->getChildByName("DefenseAnim");
    if (defenseAnim) {
        defenseAnim->setVisible(false);
        CCLOG("BuildingSprite: Defense animation hidden (ID=%d)", _buildingId);
    }
    
    if (_healthBar) {
        _healthBar->hide();
    }
}

void BuildingSprite::showTargetBeacon() {
  if (!_targetBeacon) {
    // 创建目标指示信标精灵
    _targetBeacon = Sprite::create("UI/battle/beacon/beacon.png");
    if (_targetBeacon) {
      auto size = this->getContentSize();
      float beaconY = size.height * 0.5f + _visualOffset.y;
      _targetBeacon->setPosition(Vec2(size.width / 2, beaconY)); 
      this->addChild(_targetBeacon, 101);
    }
  }

  if (_targetBeacon) {
    _targetBeacon->setVisible(true);
    _targetBeacon->stopAllActions();
    _targetBeacon->setOpacity(255);
    _targetBeacon->setScale(0.1f);

    // 播放信标缩放弹出动画后自动隐藏
    auto scaleUp = ScaleTo::create(0.2f, 1.2f);
    auto scaleNormal = ScaleTo::create(0.1f, 1.0f);
    
    auto sequence = Sequence::create(
        scaleUp, 
        scaleNormal,
        DelayTime::create(3.0f),
        FadeOut::create(0.5f),
        Hide::create(),
        nullptr
    );
    
    _targetBeacon->runAction(sequence);
    
    CCLOG("BuildingSprite: Showing target beacon for ID=%d", _buildingId);
  } else {
    CCLOG("BuildingSprite: Failed to create beacon sprite");
  }
}

void BuildingSprite::setMainTextureVisible(bool visible) {
    if (visible) {
        this->setVisible(true);
        this->setOpacity(255);
    } else {
        // 隐藏主纹理但保留其他子节点可见
        this->setOpacity(0);
        this->setTextureRect(Rect(0, 0, 0, 0));
    }

    CCLOG("BuildingSprite: Main texture visibility set to %s (ID=%d)",
          visible ? "VISIBLE" : "HIDDEN", _buildingId);
}
