#pragma once

#include "cocos2d.h"

USING_NS_CC;

/**
 * @brief 通用血条组件 - 可用于建筑、兵种等任何需要显示生命值的对象
 *
 * 功能：
 * 1. 自动根据血量百分比改变颜色（绿→黄→红）
 * 2. 满血时自动隐藏，受伤后显示
 * 3. 支持自定义宽度、高度、位置偏移
 * 4. 自动作为父节点的子节点，跟随父节点移动
 */
class HealthBarComponent : public Node {
public:
    /**
     * @brief 创建血条组件
     * @param config 血条配置
     * @return 血条组件实例
     */
    struct Config {
        float width;           // 血条宽度（像素）
        float height;          // 血条高度（像素）
        Vec2 offset;           // 相对于父节点的偏移（X, Y）
        float highThreshold;   // 高血量阈值（%），超过此值显示绿色
        float mediumThreshold; // 中血量阈值（%），介于此值和高阈值之间显示黄色
        bool showWhenFull;     // 满血时是否显示
        float fadeInDuration;  // 淡入动画时长（秒）
        
        // 构造函数提供默认值（兼容 C++11/Android NDK）
        Config() : width(40.0f), height(6.0f), offset(Vec2(0, 10)),
                   highThreshold(60.0f), mediumThreshold(30.0f),
                   showWhenFull(false), fadeInDuration(0.2f) {}
    };

    static HealthBarComponent* create();  // 无参版本
    static HealthBarComponent* create(const Config& config);

    virtual bool init(const Config& config);

    /**
     * @brief 更新血条显示
     * @param currentHP 当前生命值
     * @param maxHP 最大生命值
     */
    void updateHealth(int currentHP, int maxHP);

    /**
     * @brief 显示血条（带淡入动画）
     */
    void show();

    /**
     * @brief 隐藏血条
     */
    void hide();

    /**
     * @brief 更新血条位置（相对于父节点）
     * @param parentSize 父节点的内容大小
     */
    void updatePosition(const Size& parentSize);

private:
    // 配置参数
    Config _config;

    // UI组件
    Sprite* _background = nullptr;      // 背景（深灰色）
    ProgressTimer* _progressBar = nullptr;  // 进度条（绿/黄/红）

    // 状态
    bool _isShowing = false;

    // 创建UI元素
    void createBarComponents();

    // 根据百分比获取颜色
    Color3B getColorForPercent(float percent) const;
};
