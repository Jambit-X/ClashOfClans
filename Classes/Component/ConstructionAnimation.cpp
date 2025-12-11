#pragma execution_character_set("utf-8")
#include "ConstructionAnimation.h"
#include "Sprite/BuildingSprite.h"

USING_NS_CC;

ConstructionAnimation::ConstructionAnimation(BuildingSprite* targetSprite)
  : _targetSprite(targetSprite)
  , _progressLabel(nullptr)
  , _dotCount(0)
  , _isRunning(false) {}

ConstructionAnimation::~ConstructionAnimation() {
  stop();
}

void ConstructionAnimation::start() {
  if (_isRunning) return;

  _isRunning = true;
  createAnimationLabel();

  // 建筑变黑
  if (_targetSprite) {
    _targetSprite->runAction(TintTo::create(0.3f, 60, 60, 60));
  }

  // 启动省略号动画
  auto scheduler = Director::getInstance()->getScheduler();
  scheduler->schedule(
    [this](float dt) { this->updateDots(dt); },
    this,
    0.5f,  // 每0.5秒更新一次省略号
    CC_REPEAT_FOREVER,
    0.0f,
    false,
    "construction_dots_animation"
  );

  CCLOG("ConstructionAnimation: Started");
}

void ConstructionAnimation::stop() {
  if (!_isRunning) return;

  _isRunning = false;

  // 恢复建筑颜色
  if (_targetSprite) {
    _targetSprite->stopAllActions();
    _targetSprite->runAction(TintTo::create(0.3f, 255, 255, 255));
  }

  // 移除标签
  if (_progressLabel) {
    _progressLabel->removeFromParent();
    _progressLabel = nullptr;
  }

  // 停止定时器
  auto scheduler = Director::getInstance()->getScheduler();
  scheduler->unschedule("construction_dots_animation", this);

  CCLOG("ConstructionAnimation: Stopped");
}

void ConstructionAnimation::updateProgress(float progress) {
  if (!_progressLabel || !_isRunning) return;

  // 更新进度文字
  int percentage = static_cast<int>(progress * 100);
  std::string dotsStr = "";
  for (int i = 0; i < _dotCount; ++i) {
    dotsStr += ".";
  }

  _progressLabel->setString("建造中" + dotsStr + " " + std::to_string(percentage) + "%");
}

void ConstructionAnimation::createAnimationLabel() {
  if (!_targetSprite) return;

  _progressLabel = Label::createWithTTF("建造中...", "fonts/simhei.ttf", 20);
  _progressLabel->setColor(Color3B::YELLOW);
  _progressLabel->enableOutline(Color4B::BLACK, 2);

  // 放在建筑上方
  auto spriteSize = _targetSprite->getContentSize();
  _progressLabel->setPosition(Vec2(spriteSize.width / 2, spriteSize.height + 20));

  _targetSprite->addChild(_progressLabel, 10);
}

void ConstructionAnimation::updateDots(float dt) {
  _dotCount = (_dotCount + 1) % 4;  // 0, 1, 2, 3 循环
  updateProgress(0.0f);  // 触发更新，实际进度由外部设置
}