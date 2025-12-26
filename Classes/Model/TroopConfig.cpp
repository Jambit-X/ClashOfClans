#pragma execution_character_set("utf-8")
#include "TroopConfig.h"

TroopConfig* TroopConfig::_instance = nullptr;

TroopConfig* TroopConfig::getInstance() {
    if (!_instance) {
        _instance = new TroopConfig();
        _instance->initConfig();
    }
    return _instance;
}

void TroopConfig::initConfig() {
    _troops.clear();
    std::string pathPrefix = "UI/training-camp/troop-cards/";

    TroopInfo troop;
    
    // 1. 野蛮人 (近战，攻击距离 0.4 格)
    troop = TroopInfo();
    troop.id = 1001;
    troop.name = "野蛮人";
    troop.iconPath = pathPrefix + "Barbarian.png";
    troop.unlockBarracksLvl = 1;
    troop.housingSpace = 1;
    troop.moveSpeed = 16;
    troop.damagePerSecond = 8;
    troop.hitpoints = 45;
    troop.damageType = "单体伤害";
    troop.target = "地面目标";
    troop.favorite = "无";
    troop.level = 1;
    troop.description = "依靠结实的肌肉在敌人的村庄肆虐。让他们冲锋陷阵吧！";
    troop.splashRadius = 0.0f;
    _troops.push_back(troop);

    // 2. 弓箭手 (远程，攻击距离 3.5 格)
    troop = TroopInfo();
    troop.id = 1002;
    troop.name = "弓箭手";
    troop.iconPath = pathPrefix + "Archer.png";
    troop.unlockBarracksLvl = 1;
    troop.housingSpace = 1;
    troop.moveSpeed = 24;
    troop.damagePerSecond = 7;
    troop.hitpoints = 20;
    troop.damageType = "单体伤害";
    troop.target = "地面/空中";
    troop.favorite = "无";
    troop.level = 1;
    troop.description = "这些百步穿杨的神射手在战场上总是以此为荣。她们虽然血量不高，但射程优势巨大。";
    troop.splashRadius = 0.0f;
    _troops.push_back(troop);

    // 3. 哥布林 (近战，攻击距离 0.4 格)
    troop = TroopInfo();
    troop.id = 1003;
    troop.name = "哥布林";
    troop.iconPath = pathPrefix + "Goblin.png";
    troop.unlockBarracksLvl = 2;
    troop.housingSpace = 1;
    troop.moveSpeed = 32;
    troop.damagePerSecond = 11;
    troop.hitpoints = 25;
    troop.damageType = "单体伤害";
    troop.target = "地面目标";
    troop.favorite = "资源建筑";
    troop.level = 1;
    troop.description = "这些烦人的小生物眼里只有资源。它们移动速度极快，对金币和圣水有着无穷的渴望。";
    troop.splashRadius = 0.0f;
    _troops.push_back(troop);

    // 4. 巨人 (近战，攻击距离 1.0 格)
    troop = TroopInfo();
    troop.id = 1004;
    troop.name = "巨人";
    troop.iconPath = pathPrefix + "Giant.png";
    troop.unlockBarracksLvl = 2;
    troop.housingSpace = 5;
    troop.moveSpeed = 12;
    troop.damagePerSecond = 11;
    troop.hitpoints = 300;
    troop.damageType = "单体伤害";
    troop.target = "地面目标";
    troop.favorite = "防御建筑";
    troop.level = 1;
    troop.description = "这些大家伙虽然看起来笨重，但却能承受惊人的伤害。它们专注于摧毁防御建筑。";
    troop.splashRadius = 0.0f;
    _troops.push_back(troop);

    // 5. 炸弹人 (近战，攻击距离 0.5 格)
    troop = TroopInfo();
    troop.id = 1005;
    troop.name = "炸弹人";
    troop.iconPath = pathPrefix + "Wall_Breaker.png";
    troop.unlockBarracksLvl = 3;
    troop.housingSpace = 2;
    troop.moveSpeed = 24;
    troop.damagePerSecond = 12;
    troop.hitpoints = 20;
    troop.damageType = "区域溅射";
    troop.target = "地面目标";
    troop.favorite = "城墙";
    troop.level = 1;
    troop.description = "除了炸毁城墙，没有什么能让这些亡灵更开心的了。为你的地面部队开路！";
    troop.splashRadius = 2.5f;
    _troops.push_back(troop);

    // 6. 气球兵 (远程，攻击距离 0.5 格 - 需要贴近目标投弹)
    troop = TroopInfo();
    troop.id = 1006;
    troop.name = "气球兵";
    troop.iconPath = pathPrefix + "Balloon.png";
    troop.unlockBarracksLvl = 3;
    troop.housingSpace = 5;
    troop.moveSpeed = 10;
    troop.damagePerSecond = 25;
    troop.hitpoints = 150;
    troop.damageType = "区域溅射";
    troop.target = "地面目标";
    troop.favorite = "防御建筑";
    troop.level = 1;
    troop.description = "这些高级的气球兵投掷炸弹造成巨大的溅射伤害。但在防空火箭面前它们很脆弱。";
    troop.splashRadius = 1.5f;
    _troops.push_back(troop);

    // 建立索引
    for (size_t i = 0; i < _troops.size(); ++i) {
        _idToIndex[_troops[i].id] = static_cast<int>(i);
    }
}

TroopInfo TroopConfig::getTroopById(int id) {
    if (_idToIndex.find(id) != _idToIndex.end()) {
        return _troops[_idToIndex[id]];
    }
    return TroopInfo(); // 返回空
}

const std::vector<TroopInfo>& TroopConfig::getAllTroops() const {
    return _troops;
}
