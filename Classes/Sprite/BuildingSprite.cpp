#include "BuildingSprite.h"
#include "../Model/BuildingConfig.h"
// 不再需要 ConstructionAnimation
// #include "../Component/ConstructionAnimation.h"

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

  // 初始化容器和子元素为 nullptr
  _constructionUIContainer = nullptr;
  _progressBg = nullptr;
  _progressBar = nullptr;
  _countdownLabel = nullptr;
  _percentLabel = nullptr;

  loadSprite(_buildingType, _buildingLevel);

  // 预先创建 UI 容器（只创建一次）
  initConstructionUI();

  updateVisuals();

  // 如果是建造中状态，显示 UI
  if (_buildingState == BuildingInstance::State::CONSTRUCTING) {
    startConstruction();
  }

  return true;
}

// 新增：创建 UI 容器（核心方法）
void BuildingSprite::initConstructionUI() {
  // 创建容器
  _constructionUIContainer = Node::create();
  _constructionUIContainer->setVisible(false);  // 默认隐藏
  this->addChild(_constructionUIContainer, 100);  // 高 ZOrder

  auto spriteSize = this->getContentSize();

  // 1. 创建进度条背景（灰色）
  _progressBg = Sprite::create();
  _progressBg->setTextureRect(Rect(0, 0, 100, 12));
  _progressBg->setColor(Color3B(50, 50, 50));
  _progressBg->setOpacity(200);
  _progressBg->setPosition(Vec2(spriteSize.width / 2, spriteSize.height + 30));
  _constructionUIContainer->addChild(_progressBg, 1);

  // 2. 创建进度条（绿色）
  auto progressSprite = Sprite::create();
  progressSprite->setTextureRect(Rect(0, 0, 100, 12));
  progressSprite->setColor(Color3B(50, 205, 50));

  _progressBar = ProgressTimer::create(progressSprite);
  _progressBar->setType(ProgressTimer::Type::BAR);
  _progressBar->setMidpoint(Vec2(0, 0.5f));
  _progressBar->setBarChangeRate(Vec2(1, 0));
  _progressBar->setPosition(Vec2(spriteSize.width / 2, spriteSize.height + 30));
  _constructionUIContainer->addChild(_progressBar, 2);

  // 3. 创建倒计时标签
  _countdownLabel = Label::createWithTTF("", "fonts/simhei.ttf", 18);
  _countdownLabel->setColor(Color3B::WHITE);
  _countdownLabel->enableOutline(Color4B::BLACK, 2);
  _countdownLabel->setPosition(Vec2(spriteSize.width / 2, spriteSize.height + 50));
  _constructionUIContainer->addChild(_countdownLabel, 3);

  // 4. 创建百分比标签（原 ConstructionAnimation 的）
  _percentLabel = Label::createWithTTF("", "fonts/simhei.ttf", 20);
  _percentLabel->setColor(Color3B::YELLOW);
  _percentLabel->enableOutline(Color4B::BLACK, 2);
  _percentLabel->setPosition(Vec2(spriteSize.width / 2, spriteSize.height + 70));
  _constructionUIContainer->addChild(_percentLabel, 4);

  CCLOG("BuildingSprite: Construction UI container initialized (ID=%d)", _buildingId);
}

// 新增：显示建造 UI
void BuildingSprite::showConstructionUI() {
  if (_constructionUIContainer) {
    _constructionUIContainer->setVisible(true);
  }
}

// 新增：隐藏建造 UI
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

  // 清理旧状态的 UI
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

  // 显示 UI 容器
  showConstructionUI();

  // 设置建筑变暗
  this->setColor(Color3B(100, 100, 100));
}

void BuildingSprite::updateConstructionProgress(float progress) {
  // 更新进度条
  if (_progressBar) {
    _progressBar->setPercentage(progress * 100);
  }

  // 更新百分比文字
  if (_percentLabel) {
    std::string text = StringUtils::format("建造中...%.0f%%", progress * 100);
    _percentLabel->setString(text);
  }
}

void BuildingSprite::finishConstruction() {
  CCLOG("BuildingSprite: Finishing construction (ID=%d)", _buildingId);

  // 隐藏 UI 容器（不删除，可复用）
  hideConstructionUI();

  // 恢复建筑颜色
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
  switch (_buildingState) {
    case BuildingInstance::State::PLACING:
      this->setOpacity(180);
      break;
    case BuildingInstance::State::CONSTRUCTING:
      this->setOpacity(255);
      // 颜色由 startConstruction() 设置
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
    this->setColor(cocos2d::Color3B(100, 255, 100));
    this->setOpacity(220);
  } else {
    this->setColor(cocos2d::Color3B(255, 100, 100));
    this->setOpacity(220);
  }
}