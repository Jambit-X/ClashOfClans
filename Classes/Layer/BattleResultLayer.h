// BattleResultLayer.h
// 战斗结算层声明，显示战斗结果和资源掠夺信息

#pragma once
#ifndef __BATTLE_RESULT_LAYER_H__
#define __BATTLE_RESULT_LAYER_H__

#include "cocos2d.h"
#include <map>

class BattleResultLayer : public cocos2d::LayerColor {
public:
    // 创建结算层（带消耗数据和掠夺资源）
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
    
    // 退出时清理音乐
    virtual void onExit() override;
    
    CREATE_FUNC(BattleResultLayer);

private:
    std::map<int, int> _usedTroops;
    std::map<int, int> _troopLevels;
    int _lootedGold = 0;
    int _lootedElixir = 0;
    
    int _resultMusicID = -1;  // 音乐ID
    
    void createTroopCards();
    void playResultMusic();
};

#endif // __BATTLE_RESULT_LAYER_H__
