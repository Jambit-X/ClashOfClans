// ConstructionAnimation.cpp
// 建筑建造动画组件实现，负责显示建造进度和视觉效果

#pragma execution_character_set("utf-8")
#include "ConstructionAnimation.h"
#include "Sprite/BuildingSprite.h"

USING_NS_CC;

ConstructionAnimation::ConstructionAnimation(BuildingSprite* sprite)
  : _buildingSprite(sprite)
  , _progressLabel(nullptr)
  , _isRunning(false) {}

ConstructionAnimation::~ConstructionAnimation() {
  stop();
}

void ConstructionAnimation::start() {
  if (_isRunning) return;

  _isRunning = true;

  // 创建进度标签
  if (!_progressLabel) {
    _progressLabel = Label::createWithTTF("建造中...0%", "fonts/simhei.ttf", 20);
    _progressLabel->setColor(Color3B::YELLOW);
    _progressLabel->enableOutline(Color4B::BLACK, 2);

    auto spriteSize = _buildingSprite->getContentSize();
    _progressLabel->setPosition(Vec2(spriteSize.width / 2, spriteSize.height + 70));
    _buildingSprite->addChild(_progressLabel, 12);
  }

  // 建筑变暗效果
  _buildingSprite->setColor(Color3B(100, 100, 100));

  CCLOG("ConstructionAnimation: Started");
}

void ConstructionAnimation::stop() {
  if (!_isRunning) return;

  _isRunning = false;

  // 移除进度标签
  if (_progressLabel) {
    _progressLabel->removeFromParent();
    _progressLabel = nullptr;
  }

  // 恢复建筑颜色
  _buildingSprite->setColor(Color3B::WHITE);

  CCLOG("ConstructionAnimation: Stopped");
}

void ConstructionAnimation::updateProgress(float progress) {
  if (!_isRunning) return;

  // 更新进度文字
  if (_progressLabel) {
    std::string progressText = StringUtils::format("建造中...%.0f%%", progress * 100);
    _progressLabel->setString(progressText);
  }
}
