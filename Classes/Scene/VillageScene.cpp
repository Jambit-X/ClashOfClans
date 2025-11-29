#include "VillageScene.h"

USING_NS_CC;

// ��������
Scene* VillageScene::createScene() {
  auto scene = Scene::create();
  auto layer = VillageScene::create();
  scene->addChild(layer);
  return scene;
}

// ��ʼ��
bool VillageScene::init() {
  if (!Scene::init()) return false;

  auto screenSize = Director::getInstance()->getVisibleSize();

  // ��ׯ����
  auto bg = Sprite::create("Scene/VillageScene.png");
  bg->setPosition(screenSize / 2);
  bg->Sprite::setScale(0.5);
  this->addChild(bg);

  //// �򵥽���
  //auto townHall = Sprite::create("townhall.png");
  //townHall->setPosition(screenSize.width / 2, screenSize.height / 2);
  //this->addChild(townHall);


  auto label = Label::createWithSystemFont("man! what can i say!", "Arial", 36);
  label->setPosition(Vec2(screenSize.width / 2, screenSize.height - 80));
  this->addChild(label);

  return true;
}