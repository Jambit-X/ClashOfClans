#pragma execution_character_set("utf-8")
#include "BattleResultLayer.h"
#include "../Scene/BattleScene.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace ui;

const std::string FONT_PATH = "fonts/simhei.ttf";

bool BattleResultLayer::init() {
    // 半透明黑色背景
    if (!LayerColor::initWithColor(Color4B(0, 0, 0, 200))) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 标题
    auto titleLabel = Label::createWithTTF("战斗结束", FONT_PATH, 48);
    titleLabel->setPosition(visibleSize.width / 2, visibleSize.height / 2 + 100);
    titleLabel->setColor(Color3B::YELLOW);
    this->addChild(titleLabel);

    // 模拟结算数据
    auto infoLabel = Label::createWithTTF("获得战利品:\n金币: 0\n圣水: 0", FONT_PATH, 30);
    infoLabel->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    this->addChild(infoLabel);

    // [回营] 按钮 - 替换为图片 back.png
    auto btnReturn = Button::create("UI/battle/battle-prepare/back.png");

    // 稍微缩小一点，防止原图太大
    btnReturn->setScale(0.4f);

    // 放在屏幕下方中间位置
    btnReturn->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 150));

    btnReturn->addClickEventListener([this](Ref*) {
        auto scene = dynamic_cast<BattleScene*>(this->getScene());
        if (scene) scene->onReturnHomeClicked();
        });
    this->addChild(btnReturn);

    // 吞噬触摸，防止点到后面的地图
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch*, Event*) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}