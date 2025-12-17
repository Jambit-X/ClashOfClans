#pragma once
#ifndef __BATTLE_RESULT_LAYER_H__
#define __BATTLE_RESULT_LAYER_H__

#include "cocos2d.h"
#include <map>

class BattleResultLayer : public cocos2d::LayerColor {
public:
    // 带消耗数据和掠夺资源的创建方法
    static BattleResultLayer* createWithData(
        const std::map<int, int>& usedTroops,
        const std::map<int, int>& troopLevels,
        int lootedGold = 0,
        int lootedElixir = 0);
    
    virtual bool init() override;
    bool initWithData(const std::map<int, int>& usedTroops,
                      const std::map<int, int>& troopLevels,
                      int lootedGold,
                      int lootedElixir);
    
    CREATE_FUNC(BattleResultLayer);

private:
    std::map<int, int> _usedTroops;
    std::map<int, int> _troopLevels;
    int _lootedGold = 0;
    int _lootedElixir = 0;
    
    void createTroopCards();
};

#endif // __BATTLE_RESULT_LAYER_H__