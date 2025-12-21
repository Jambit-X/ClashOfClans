#pragma once

#include "cocos2d.h"

/**
 * @brief 战斗进度UI - 显示星星和摧毁率
 *
 * 功能：
 * 1. 右下角显示当前星数和摧毁进度
 * 2. 放下第一个兵时淡入显示
 * 3. 结算时移动到屏幕中央并放大
 */
class BattleProgressUI : public cocos2d::Node {
public:
    static BattleProgressUI* create();

    virtual bool init() override;

    /**
     * @brief 更新星星显示
     * @param starCount 当前星数 (0-3)
     */
    void updateStars(int starCount);

    /**
     * @brief 更新摧毁率显示
     * @param progress 摧毁率 (0.0-100.0)
     */
    void updateProgress(float progress);

    /**
     * @brief 显示UI（淡入动画）
     */
    void show();

    /**
     * @brief 播放结算动画（移动到屏幕中央并放大）
     * @param onComplete 动画完成回调
     */
    void playResultAnimation(const std::function<void()>& onComplete = nullptr);

    /**
     * @brief 重置到初始状态（用于重新开始战斗）
     */
    void reset();

private:
    // UI容器
    cocos2d::Node* _container;

    // 星星精灵（3个）
    cocos2d::Sprite* _starSprites[3];

    // 摧毁率文字
    cocos2d::Label* _progressLabel;

    // 背景
    cocos2d::LayerColor* _background;

    // 当前状态
    int _currentStars;
    float _currentProgress;
    bool _isShown;

    // 初始化子元素
    void initBackground();
    void initStars();
    void initProgressLabel();

    // 星星动画
    void playStarAnimation(int starIndex);
    // ✅ 新增：战斗进度UI
    BattleProgressUI* _battleProgressUI;

    // ✅ 新增：追踪是否已放下第一个兵
    bool _hasDeployedFirstTroop;
};
