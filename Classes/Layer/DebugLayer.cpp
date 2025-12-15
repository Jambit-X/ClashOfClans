#pragma execution_character_set("utf-8")
#include "DebugLayer.h"
#include "../Util/DebugHelper.h"
#include "../Manager/VillageDataManager.h"
#include "../Layer/VillageLayer.h"

USING_NS_CC;
using namespace ui;

bool DebugLayer::init() {
    if (!Layer::init()) return false;

    // 吞噬触摸事件，防止点击穿透到游戏层
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch* touch, Event* event) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    initBackground();
    initPanel();

    return true;
}

void DebugLayer::initBackground() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    
    // 半透明黑色背景
    auto bg = LayerColor::create(Color4B(0, 0, 0, 150));
    this->addChild(bg);
}

void DebugLayer::initPanel() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    
    // 面板背景
    _panel = LayerColor::create(Color4B(40, 40, 40, 240)); // 深灰色背景
    _panel->setContentSize(Size(600, 500));
    _panel->setIgnoreAnchorPointForPosition(false);
    _panel->setAnchorPoint(Vec2(0.5, 0.5));
    _panel->setPosition(visibleSize / 2);
    this->addChild(_panel);

    // 标题栏
    auto title = Label::createWithSystemFont("🔧 调试模式", "Arial", 24);
    title->setPosition(Vec2(300, 470));
    _panel->addChild(title);

    // 关闭按钮 [X]
    auto closeBtn = Button::create();
    closeBtn->setTitleText("X");
    closeBtn->setTitleFontSize(20);
    closeBtn->setTitleColor(Color3B::RED);
    closeBtn->setPosition(Vec2(570, 470));
    closeBtn->addClickEventListener([this](Ref*) { this->close(); });
    _panel->addChild(closeBtn);

    initResourceSection();
    initBuildingSection();
    initSaveSection();
}

void DebugLayer::initResourceSection() {
    // 资源区域标题
    auto label = Label::createWithSystemFont("资源修改", "Arial", 18);
    label->setAnchorPoint(Vec2(0, 0.5));
    label->setPosition(Vec2(20, 440));
    label->setColor(Color3B::YELLOW);
    _panel->addChild(label);

    auto dataManager = VillageDataManager::getInstance();

    // 通用创建资源行函数
    auto createRow = [&](const std::string& name, int y, int currentVal, 
                        Label** outLabel, const std::function<void(int)>& callback) {
        
        // 名字
        auto nameLabel = Label::createWithSystemFont(name, "Arial", 16);
        nameLabel->setAnchorPoint(Vec2(0, 0.5));
        nameLabel->setPosition(Vec2(30, y));
        _panel->addChild(nameLabel);

        // 数值
        *outLabel = Label::createWithSystemFont(std::to_string(currentVal), "Arial", 16);
        (*outLabel)->setPosition(Vec2(150, y));
        _panel->addChild(*outLabel);

        // 按钮组
        struct BtnConfig { const char* text; int val; };
        BtnConfig btns[] = { {"-1k", -1000}, {"+1k", 1000}, {"+10k", 10000} };

        int x = 220;
        for (auto& cfg : btns) {
            auto btn = Button::create();
            btn->setTitleText(cfg.text);
            btn->setTitleFontSize(14);
            btn->setPosition(Vec2(x, y));
            // 简单的按钮背景以便看清
            // 实际项目中可能使用图片，这里仅用文字
            btn->addClickEventListener([=](Ref*) {
                callback(cfg.val); 
            });
            _panel->addChild(btn);
            x += 60;
        }
    };

    // 金币
    createRow("金币:", 410, dataManager->getGold(), &_goldValueLabel, 
        [this](int d){ this->onGoldChanged(d); });

    // 圣水
    createRow("圣水:", 370, dataManager->getElixir(), &_elixirValueLabel, 
        [this](int d){ this->onElixirChanged(d); });
    
    // 宝石 (稍微特殊，数值小一点)
    auto gemLabel = Label::createWithSystemFont("宝石:", "Arial", 16);
    gemLabel->setAnchorPoint(Vec2(0, 0.5));
    gemLabel->setPosition(Vec2(30, 330));
    _panel->addChild(gemLabel);

    _gemValueLabel = Label::createWithSystemFont(std::to_string(dataManager->getGem()), "Arial", 16);
    _gemValueLabel->setPosition(Vec2(150, 330));
    _panel->addChild(_gemValueLabel);

    // 宝石按钮
    int x = 220;
    int gemVals[] = {-100, 100, 1000};
    const char* gemTxt[] = {"-100", "+100", "+1k"};
    for(int i=0; i<3; ++i) {
        auto btn = Button::create();
        btn->setTitleText(gemTxt[i]);
        btn->setTitleFontSize(14);
        btn->setPosition(Vec2(x, 330));
        btn->addClickEventListener([=](Ref*){ this->onGemChanged(gemVals[i]); });
        _panel->addChild(btn);
        x += 60;
    }
}

void DebugLayer::initBuildingSection() {
    auto label = Label::createWithSystemFont("建筑管理", "Arial", 18);
    label->setAnchorPoint(Vec2(0, 0.5));
    label->setPosition(Vec2(20, 290));
    label->setColor(Color3B::YELLOW);
    _panel->addChild(label);

    // 选中的建筑信息
    _selectedBuildingLabel = Label::createWithSystemFont("当前未选中建筑 (请在游戏内点击)", "Arial", 14);
    _selectedBuildingLabel->setPosition(Vec2(300, 260));
    _selectedBuildingLabel->setColor(Color3B::GRAY);
    _panel->addChild(_selectedBuildingLabel);

    // 瞬间完成建造按钮
    auto completeBtn = Button::create();
    completeBtn->setTitleText("[ ⚡ 瞬间完成所有建造 ⚡ ]");
    completeBtn->setTitleFontSize(16);
    completeBtn->setTitleColor(Color3B::GREEN);
    completeBtn->setPosition(Vec2(300, 230));
    completeBtn->addClickEventListener([this](Ref*) { this->onCompleteAllConstructions(); });
    _panel->addChild(completeBtn);

    // 删除按钮
    auto deleteBtn = Button::create();
    deleteBtn->setTitleText("[ 🗑️ 删除当前选中建筑 ]");
    deleteBtn->setTitleFontSize(16);
    deleteBtn->setTitleColor(Color3B::RED);
    deleteBtn->setPosition(Vec2(300, 200));
    deleteBtn->addClickEventListener([this](Ref*) { this->onDeleteBuilding(); });
    _panel->addChild(deleteBtn);
}

void DebugLayer::initSaveSection() {
    auto label = Label::createWithSystemFont("存档操作", "Arial", 18);
    label->setAnchorPoint(Vec2(0, 0.5));
    label->setPosition(Vec2(20, 150));
    label->setColor(Color3B::YELLOW);
    _panel->addChild(label);

    // 强制保存
    auto saveBtn = Button::create();
    saveBtn->setTitleText("[ 💾 强制保存 ]");
    saveBtn->setPosition(Vec2(150, 110));
    saveBtn->setTitleFontSize(16);
    saveBtn->addClickEventListener([this](Ref*) { this->onForceSave(); });
    _panel->addChild(saveBtn);

    // 重置存档
    auto resetBtn = Button::create();
    resetBtn->setTitleText("[ ⚠️ 清空存档重置 ]");
    resetBtn->setPosition(Vec2(450, 110));
    resetBtn->setTitleFontSize(16);
    resetBtn->setTitleColor(Color3B::ORANGE);
    resetBtn->addClickEventListener([this](Ref*) { this->onResetSave(); });
    _panel->addChild(resetBtn);
}

void DebugLayer::close() {
    this->removeFromParent();
}

void DebugLayer::refreshBuildingList() {
    // 刷新选中状态文本
    // 实际项目中可能需要从 VillageLayer 获取当前选中的 ID
    // 这里简化处理，假设用户通过点击建筑后来到这里
    // 或者我们可以在 onEnter 时获取
}

void DebugLayer::onGoldChanged(int delta) {
    int current = VillageDataManager::getInstance()->getGold();
    DebugHelper::setGold(current + delta);
    _goldValueLabel->setString(std::to_string(VillageDataManager::getInstance()->getGold()));
}

void DebugLayer::onElixirChanged(int delta) {
    int current = VillageDataManager::getInstance()->getElixir();
    DebugHelper::setElixir(current + delta);
    _elixirValueLabel->setString(std::to_string(VillageDataManager::getInstance()->getElixir()));
}

void DebugLayer::onGemChanged(int delta) {
    int current = VillageDataManager::getInstance()->getGem();
    DebugHelper::setGem(current + delta);
    _gemValueLabel->setString(std::to_string(VillageDataManager::getInstance()->getGem()));
}

void DebugLayer::onCompleteAllConstructions() {
    DebugHelper::completeAllConstructions();
    // 简单的反馈动画
    auto s = Director::getInstance()->getWinSize();
    auto label = Label::createWithSystemFont("所有建造已完成!", "Arial", 24);
    label->setPosition(s/2);
    label->setColor(Color3B::GREEN);
    this->addChild(label, 100);
    label->runAction(Sequence::create(MoveBy::create(1.0f, Vec2(0, 50)), FadeOut::create(0.5f), RemoveSelf::create(), nullptr));
}

void DebugLayer::onDeleteBuilding() {
    auto scene = Director::getInstance()->getRunningScene();
    auto villageLayer = dynamic_cast<VillageLayer*>(scene->getChildByTag(1));
    
    if (!villageLayer) {
        _selectedBuildingLabel->setString("错误: 未找到VillageLayer");
        return;
    }

    int selectedId = villageLayer->getSelectedBuildingId();
    if (selectedId == -1) {
        _selectedBuildingLabel->setString("请先在游戏中选中一个建筑!");
        _selectedBuildingLabel->setColor(Color3B::RED);
        return;
    }

    DebugHelper::deleteBuilding(selectedId);
    _selectedBuildingLabel->setString("建筑已删除 ID: " + std::to_string(selectedId));
    _selectedBuildingLabel->setColor(Color3B::GREEN);
}

void DebugLayer::onForceSave() {
    DebugHelper::forceSave();
    _selectedBuildingLabel->setString("已保存!");
}

void DebugLayer::onResetSave() {
    DebugHelper::resetSaveData();
    _selectedBuildingLabel->setString("存档已清除，请重启游戏");
}
