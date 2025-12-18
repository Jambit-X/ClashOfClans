#include "BuildingSprite.h"
#include "../Model/BuildingConfig.h"


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

  // 初始化选中效果成员
  _selectionGlow = nullptr;
  _isSelected = false;

  // 初始化血条成员
  _healthBarContainer = nullptr;
  _healthBarBg = nullptr;
  _healthBar = nullptr;
  _cachedMaxHP = 0;

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

// ========== 选中效果实现 ==========

void BuildingSprite::createSelectionGlow() {
  if (_selectionGlow) return;  // 已存在则不重复创建

  _selectionGlow = DrawNode::create();
  
  // 获取建筑网格尺寸
  auto config = BuildingConfig::getInstance()->getConfig(_buildingType);
  if (!config) return;
  
  // [B] 放大尺寸：系数从 1.1 改为 1.4
  const float GRID_UNIT = 28.0f;
  float ellipseWidth = config->gridWidth * GRID_UNIT * 1.4f;
  float ellipseHeight = config->gridHeight * GRID_UNIT * 0.7f;
  
  // ========== [D] 外层光晕（更大、更透明）==========
  const int segments = 48;  // 更多分段，更平滑
  std::vector<Vec2> outerPoints;
  float outerWidth = ellipseWidth * 1.3f;   // 外层比内层大30%
  float outerHeight = ellipseHeight * 1.3f;
  for (int i = 0; i < segments; ++i) {
    float angle = (float)i / segments * 2.0f * M_PI;
    float x = outerWidth * 0.5f * cosf(angle);
    float y = outerHeight * 0.5f * sinf(angle);
    outerPoints.push_back(Vec2(x, y));
  }
  
  // 外层：淡金色，更透明
  _selectionGlow->drawPolygon(
    outerPoints.data(),
    segments,
    Color4F(1.0f, 0.7f, 0.1f, 0.15f),  // 很淡的填充
    3.0f,                               // 较粗边框
    Color4F(1.0f, 0.8f, 0.2f, 0.4f)    // 淡边框
  );
  
  // ========== 内层光圈（主体）==========
  std::vector<Vec2> innerPoints;
  for (int i = 0; i < segments; ++i) {
    float angle = (float)i / segments * 2.0f * M_PI;
    float x = ellipseWidth * 0.5f * cosf(angle);
    float y = ellipseHeight * 0.5f * sinf(angle);
    innerPoints.push_back(Vec2(x, y));
  }
  
  // [A] 提高不透明度 + [C] 加粗边框
  _selectionGlow->drawPolygon(
    innerPoints.data(),
    segments,
    Color4F(1.0f, 0.8f, 0.2f, 0.45f),   // 填充色（更实心）
    4.0f,                                // 边框宽度（加粗）
    Color4F(1.0f, 0.9f, 0.3f, 0.9f)     // 边框色（更亮）
  );
  
  // 定位光圈到建筑底部中心
  auto spriteSize = this->getContentSize();
  _selectionGlow->setPosition(Vec2(spriteSize.width / 2, ellipseHeight * 0.4f));
  
  // 层级低于建筑精灵本体
  this->addChild(_selectionGlow, -1);
  _selectionGlow->setVisible(false);
  
  CCLOG("BuildingSprite: Selection glow created (ID=%d, size=%.0fx%.0f)", 
        _buildingId, ellipseWidth, ellipseHeight);
}

void BuildingSprite::showSelectionEffect() {
  if (_isSelected) return;  // 避免重复触发
  _isSelected = true;
  
  // 1. 弹跳动画
  this->stopActionByTag(100);  // 停止之前的弹跳
  auto bounceUp = ScaleTo::create(0.1f, 1.08f);
  auto bounceDown = ScaleTo::create(0.1f, 1.0f);
  auto bounce = Sequence::create(bounceUp, bounceDown, nullptr);
  bounce->setTag(100);
  this->runAction(bounce);
  
  // 2. 显示光圈（如果不存在则创建）
  if (!_selectionGlow) {
    createSelectionGlow();
  }
  
  if (_selectionGlow) {
    _selectionGlow->setVisible(true);
    _selectionGlow->setOpacity(255);
    
    // 脉冲呼吸动画
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
  
  // 停止弹跳动画
  this->stopActionByTag(100);
  this->setScale(1.0f);
  
  // 隐藏光圈
  if (_selectionGlow) {
    _selectionGlow->stopActionByTag(101);
    _selectionGlow->setVisible(false);
  }
  
  CCLOG("BuildingSprite: Selection effect hidden (ID=%d)", _buildingId);
}

// ========== 血条显示实现 ==========

void BuildingSprite::initHealthBar() {
  if (_healthBarContainer) return;  // 已存在则不重复创建

  // 获取建筑配置，根据网格宽度计算血条长度
  auto config = BuildingConfig::getInstance()->getConfig(_buildingType);
  int gridWidth = config ? config->gridWidth : 2;
  
  // 血条宽度：根据建筑格子数动态计算（最小40，最大120）
  float barWidth = std::max(40.0f, std::min(120.0f, gridWidth * 30.0f));
  const float barHeight = 8.0f;

  // 创建容器
  _healthBarContainer = Node::create();
  _healthBarContainer->setVisible(false);  // 默认隐藏
  this->addChild(_healthBarContainer, 99);  // 高 ZOrder

  auto spriteSize = this->getContentSize();

  // 1. 创建血条背景（深灰色）
  _healthBarBg = Sprite::create();
  _healthBarBg->setTextureRect(Rect(0, 0, barWidth, barHeight));
  _healthBarBg->setColor(Color3B(40, 40, 40));
  _healthBarBg->setOpacity(200);
  _healthBarBg->setPosition(Vec2(spriteSize.width / 2, spriteSize.height + 15));
  _healthBarContainer->addChild(_healthBarBg, 1);

  // 2. 创建血条本体（初始绿色）
  auto healthSprite = Sprite::create();
  healthSprite->setTextureRect(Rect(0, 0, barWidth, barHeight));
  healthSprite->setColor(Color3B(50, 205, 50));  // 绿色

  _healthBar = ProgressTimer::create(healthSprite);
  _healthBar->setType(ProgressTimer::Type::BAR);
  _healthBar->setMidpoint(Vec2(0, 0.5f));       // 从左边开始
  _healthBar->setBarChangeRate(Vec2(1, 0));     // 水平方向变化
  _healthBar->setPercentage(100.0f);
  _healthBar->setPosition(Vec2(spriteSize.width / 2, spriteSize.height + 15));
  _healthBarContainer->addChild(_healthBar, 2);

  CCLOG("BuildingSprite: Health bar initialized (ID=%d, width=%.0f)", _buildingId, barWidth);
}

void BuildingSprite::updateHealthBar(int currentHP, int maxHP) {
  // 满血或无效数据时隐藏血条
  if (currentHP >= maxHP || maxHP <= 0 || currentHP < 0) {
    hideHealthBar();
    return;
  }

  // 惰性创建血条 UI
  if (!_healthBarContainer) {
    initHealthBar();
  }

  // 显示血条
  showHealthBar();

  // 计算百分比
  float percent = (float)currentHP / (float)maxHP * 100.0f;
  percent = std::max(0.0f, std::min(100.0f, percent));

  // 更新进度
  if (_healthBar) {
    _healthBar->setPercentage(percent);
    
    // 根据血量比例改变颜色
    Color3B barColor;
    if (percent > 50.0f) {
      // 绿色 (50-100%)
      barColor = Color3B(50, 205, 50);
    } else if (percent > 25.0f) {
      // 黄色 (25-50%)
      barColor = Color3B(255, 200, 0);
    } else {
      // 红色 (0-25%)
      barColor = Color3B(220, 50, 50);
    }
    _healthBar->getSprite()->setColor(barColor);
  }
}

void BuildingSprite::showHealthBar() {
  if (_healthBarContainer) {
    _healthBarContainer->setVisible(true);
  }
}

void BuildingSprite::hideHealthBar() {
  if (_healthBarContainer) {
    _healthBarContainer->setVisible(false);
  }
}

// ========== 摧毁效果实现 ==========

void BuildingSprite::showDestroyedRubble() {
    if (_isShowingRubble) return;
    _isShowingRubble = true;

    // 获取建筑的网格尺寸
    auto config = BuildingConfig::getInstance()->getConfig(_buildingType);
    int gridWidth = config ? config->gridWidth : 2;
    int gridHeight = config ? config->gridHeight : 2;
    
    int maxSize = std::max(gridWidth, gridHeight);
    
    std::string rubblePath;
    if (maxSize <= 1) {
        rubblePath = "buildings/broken/broken1x1.png";
    } else if (maxSize <= 2) {
        rubblePath = "buildings/broken/broken2x2.png";
    } else {
        rubblePath = "buildings/broken/broken3x3.png";
    }
    
    // 1. 显示主贴图（用于加载废墟纹理）
    this->setVisible(true);
    this->setOpacity(255);
    
    // 2. 加载废墟贴图
    auto texture = Director::getInstance()->getTextureCache()->addImage(rubblePath);
    if (texture) {
        this->setTexture(texture);
        auto rect = Rect(0, 0, texture->getContentSize().width, texture->getContentSize().height);
        this->setTextureRect(rect);
        
        this->setColor(Color3B::WHITE);
        
        CCLOG("BuildingSprite: Showing rubble for building ID=%d, size=%dx%d, path=%s", 
              _buildingId, gridWidth, gridHeight, rubblePath.c_str());
    } else {
        this->setColor(Color3B::RED);
        this->setOpacity(200);
        CCLOG("BuildingSprite: Failed to load rubble texture: %s", rubblePath.c_str());
    }
    
    // 3. 【新增】隐藏防御动画（如果存在）
    auto defenseAnim = this->getChildByName("DefenseAnim");
    if (defenseAnim) {
        defenseAnim->setVisible(false);
        CCLOG("BuildingSprite: Defense animation hidden (ID=%d)", _buildingId);
    }
    
    // 4. 隐藏血条
    hideHealthBar();
}

// ========== 目标标记实现 ==========

void BuildingSprite::showTargetBeacon() {
  if (!_targetBeacon) {
    _targetBeacon = Sprite::create("UI/battle/beacon/beacon.png");
    if (_targetBeacon) {
      auto size = this->getContentSize();
      // 【修复】beacon 显示在建筑的视觉中心附近
      // visualOffset.y 通常是负值，所以加上它可以让 beacon 向下移动到正确位置
      float beaconY = size.height * 0.5f + _visualOffset.y;  // 显示在建筑中心偏上
      _targetBeacon->setPosition(Vec2(size.width / 2, beaconY)); 
      this->addChild(_targetBeacon, 101); // 确保在最上层
    }
  }

  if (_targetBeacon) {
    _targetBeacon->setVisible(true);
    _targetBeacon->stopAllActions();
    _targetBeacon->setOpacity(255);
    _targetBeacon->setScale(0.1f); // 初始很小

    // 弹跳出现效果
    auto scaleUp = ScaleTo::create(0.2f, 1.2f);
    auto scaleNormal = ScaleTo::create(0.1f, 1.0f);
    
    // 持续一段时间后淡出，模拟"Ping"的效果
    auto sequence = Sequence::create(
        scaleUp, 
        scaleNormal,
        DelayTime::create(3.0f),  // 显示 3 秒
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
        // 显示主贴图
        this->setVisible(true);
        this->setOpacity(255);
    } else {
        // 完全隐藏主贴图的渲染
        // 方法1：设置为完全透明
        this->setOpacity(0);

        // 方法2：禁用纹理绘制（更彻底）
        this->setTextureRect(Rect(0, 0, 0, 0));
    }

    CCLOG("BuildingSprite: Main texture visibility set to %s (ID=%d)",
          visible ? "VISIBLE" : "HIDDEN", _buildingId);
}
