#pragma once
#ifndef __BATTLE_RESULT_LAYER_H__
#define __BATTLE_RESULT_LAYER_H__

#include "cocos2d.h"

class BattleResultLayer : public cocos2d::LayerColor {
public:
    virtual bool init() override;
    CREATE_FUNC(BattleResultLayer);
};

#endif // __BATTLE_RESULT_LAYER_H__