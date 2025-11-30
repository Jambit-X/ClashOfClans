#include "StartupScene.h"
#include "VillageScene.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

// 创建场景
Scene* StartupScene::createScene() {
  auto scene = Scene::create();
  auto layer = StartupScene::create();
  scene->addChild(layer);
  return scene;
}

// 初始化
bool StartupScene::init() {
  if (!Scene::init()) return false;

  auto screenSize = Director::getInstance()->getVisibleSize();

  // 背景
  auto bg = Sprite::create("Scene/StartupScene.png");
  bg->setPosition(screenSize / 2);
  bg->setScaleX(screenSize.width / bg->getContentSize().width);
  bg->setScaleY(screenSize.height / bg->getContentSize().height);
  this->addChild(bg);

  // 进度条
  auto progressBar = ui::LoadingBar::create("ImageElements/loading_bar.png");
  progressBar->setPosition(Vec2(screenSize.width / 2, screenSize.height / 5));
  progressBar->setPercent(0);
  this->addChild(progressBar);

  // 进度动画
  float duration = 2.0f;
  for (int i = 0; i <= 100; ++i) {
    this->scheduleOnce([progressBar, i](float) {
      progressBar->setPercent(i);
    }, duration * i / 100.0f, "loading" + std::to_string(i));
  }

  // 加载完成后切换到村庄界面
  this->scheduleOnce([](float) {
    auto scene = VillageScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(1.0f, scene));
  }, duration + 1.0f, "toVillage");

  return true;
}