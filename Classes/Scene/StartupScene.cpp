#include "StartupScene.h"
#include "VillageScene.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

// ��������
Scene* StartupScene::createScene() {
  auto scene = Scene::create();
  auto layer = StartupScene::create();
  scene->addChild(layer);
  return scene;
}

// ��ʼ��
bool StartupScene::init() {
  if (!Scene::init()) return false;

  auto screenSize = Director::getInstance()->getVisibleSize();

  // ����
  auto bg = Sprite::create("Scene/StartupScene.png");
  bg->setPosition(screenSize / 2);
  bg->setScaleX(screenSize.width / bg->getContentSize().width);
  bg->setScaleY(screenSize.height / bg->getContentSize().height);
  this->addChild(bg);

  // ������
  auto progressBar = ui::LoadingBar::create("ImageElements/loading_bar.png");
  progressBar->setPosition(Vec2(screenSize.width / 2, screenSize.height / 5));
  progressBar->setPercent(0);
  this->addChild(progressBar);

  // ���ȶ���
  float duration = 2.0f;
  for (int i = 0; i <= 100; ++i) {
    this->scheduleOnce([progressBar, i](float) {
      progressBar->setPercent(i);
    }, duration * i / 100.0f, "loading" + std::to_string(i));
  }

  // ������ɺ��л�����ׯ����
  this->scheduleOnce([](float) {
    auto scene = VillageScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(1.0f, scene));
  }, duration + 1.0f, "toVillage");

  return true;
}