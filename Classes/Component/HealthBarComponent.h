// HealthBarComponent.h
// 通用血条组件声明，用于显示游戏单位的生命值状态

#pragma once

#include "cocos2d.h"

USING_NS_CC;

// 血条组件类
// 功能：根据血量百分比自动变色、满血隐藏、支持自定义样式
class HealthBarComponent : public Node {
public:
    // 血条配置结构
    struct Config {
        float width;           // 血条宽度（像素）
        float height;          // 血条高度（像素）
        Vec2 offset;           // 相对父节点的偏移（X, Y）
        float highThreshold;   // 高血量阈值（%），超过此值显示绿色
        float mediumThreshold; // 中血量阈值（%），介于此值和高阈值之间显示黄色
        bool showWhenFull;     // 满血时是否显示
        float fadeInDuration;  // 淡入动画时长（秒）
        
        // 默认配置
        Config() : width(40.0f), height(6.0f), offset(Vec2(0, 10)),
                   highThreshold(60.0f), mediumThreshold(30.0f),
                   showWhenFull(false), fadeInDuration(0.2f) {}
    };

    static HealthBarComponent* create();
    static HealthBarComponent* create(const Config& config);

    virtual bool init(const Config& config);

    // 更新血条显示
    void updateHealth(int currentHP, int maxHP);

    // 显示血条（带淡入动画）
    void show();

    // 隐藏血条
    void hide();

    // 更新血条位置（相对于父节点）
    void updatePosition(const Size& parentSize);

private:
    Config _config;                     // 配置参数

    Sprite* _background = nullptr;      // 背景精灵
    ProgressTimer* _progressBar = nullptr;  // 进度条

    bool _isShowing = false;            // 当前是否显示

    // 创建血条UI元素
    void createBarComponents();

    // 根据血量百分比获取颜色
    Color3B getColorForPercent(float percent) const;
};
