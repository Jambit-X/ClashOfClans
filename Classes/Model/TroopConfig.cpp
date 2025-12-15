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

// Classes/Model/TroopConfig.cpp

void TroopConfig::initConfig() {
    _troops.clear();
    std::string pathPrefix = "UI/training-camp/troop-cards/";

    // 1. 野蛮人
    _troops.push_back({
        1001, "野蛮人", pathPrefix + "Barbarian.png", 1,
        1, 16, 8, 45,
        "单体伤害", "地面目标", "无", 1,
        "依靠结实的肌肉在敌人的村庄肆虐。让他们冲锋陷阵吧！"
        });

    // 2. 弓箭手
    _troops.push_back({
        1002, "弓箭手", pathPrefix + "Archer.png", 1,
        1, 24, 7, 20,
        "单体伤害", "地面/空中", "无", 1,
        "这些百步穿杨的神射手在战场上总是以此为荣。她们虽然血量不高，但射程优势巨大。"
        });

    // 3. 哥布林
    _troops.push_back({
        1003, "哥布林", pathPrefix + "Goblin.png", 2,
        1, 32, 11, 25,
        "单体伤害", "地面目标", "资源建筑", 1,
        "这些烦人的小生物眼里只有资源。它们移动速度极快，对金币和圣水有着无穷的渴望。"
        });

    // 4. 巨人
    _troops.push_back({
        1004, "巨人", pathPrefix + "Giant.png", 2,
        5, 12, 11, 300,
        "单体伤害", "地面目标", "防御建筑", 1,
        "这些大家伙虽然看起来笨重，但却能承受惊人的伤害。它们专注于摧毁防御建筑。"
        });

    // 5. 炸弹人
    _troops.push_back({
        1005, "炸弹人", pathPrefix + "Wall_Breaker.png", 3,
        2, 24, 12, 20,
        "区域溅射", "地面目标", "城墙", 1,
        "除了炸毁城墙，没有什么能让这些亡灵更开心的了。为你的地面部队开路！"
        });

    // 6. 气球兵
    _troops.push_back({
        1006, "气球兵", pathPrefix + "Balloon.png", 3,
        5, 10, 25, 150,
        "区域溅射", "地面目标", "防御建筑", 1,
        "这些高级的气球兵投掷炸弹造成巨大的溅射伤害。但在防空火箭面前它们很脆弱。"
        });

    // 建立索引
    for (size_t i = 0; i < _troops.size(); ++i) {
        _idToIndex[_troops[i].id] = i;
    }
}

TroopInfo TroopConfig::getTroopById(int id) {
    if (_idToIndex.find(id) != _idToIndex.end()) {
        return _troops[_idToIndex[id]];
    }
    return {}; // 返回空
}

const std::vector<TroopInfo>& TroopConfig::getAllTroops() const {
    return _troops;
}