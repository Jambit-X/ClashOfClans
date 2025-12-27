// BattleProgressUI.h
// 战斗进度UI类，显示战斗星数和摧毁率进度

#pragma once

#include "cocos2d.h"

class BattleProgressUI : public cocos2d::Node {
public:
    static BattleProgressUI* create();

    virtual bool init() override;

    // 更新星星显示(0-3星)
    void updateStars(int starCount);

    // 更新摧毁率显示(0.0-100.0)
    void updateProgress(float progress);

    // 显示UI并播放淡入动画
    void show();

    // 播放结算动画，移动到屏幕中央并放大
    void playResultAnimation(const std::function<void()>& onComplete = nullptr);

    // 重置到初始状态
    void reset();

private:
    cocos2d::Node* _container;
    cocos2d::Sprite* _starSprites[3];
    cocos2d::Label* _progressLabel;
    cocos2d::LayerColor* _background;

    int _currentStars;
    float _currentProgress;
    bool _isShown;

    void initBackground();
    void initStars();
    void initProgressLabel();
    void playStarAnimation(int starIndex);

    BattleProgressUI* _battleProgressUI;
    bool _hasDeployedFirstTroop;
};
