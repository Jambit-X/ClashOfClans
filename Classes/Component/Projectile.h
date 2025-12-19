#pragma once
#include "cocos2d.h"

USING_NS_CC;

/**
 * @brief 子弹/投射物类
 * 职责：处理各种类型子弹的飞行动画
 */
class Projectile : public Sprite {
public:
    /**
     * @brief 子弹类型枚举
     */
    enum class Type {
        CANNON_BULLET,   // 加农炮炮弹(直线飞行)
        ARROW,           // 箭塔箭矢(直线飞行+旋转)
        MORTAR_BULLET    // 迫击炮炮弹(抛物线飞行)
    };

    /**
     * @brief 创建子弹
     * @param type 子弹类型
     * @return 子弹实例
     */
    static Projectile* create(Type type);

    /**
     * @brief 直线飞行到目标(加农炮炮弹/箭矢)
     * @param target 目标位置
     * @param duration 飞行时间(秒)
     * @param onHit 命中回调
     */
    void flyTo(const Vec2& target, float duration, const std::function<void()>& onHit = nullptr);

    /**
     * @brief 抛物线飞行到目标(迫击炮炮弹)
     * @param target 目标位置
     * @param duration 飞行时间(秒)
     * @param height 抛物线高度
     * @param onHit 命中回调
     */
    void arcTo(const Vec2& target, float duration, float height, const std::function<void()>& onHit = nullptr);

    /**
     * @brief 设置是否根据飞行方向旋转(箭矢需要)
     * @param enabled 是否启用
     */
    void setRotateWithDirection(bool enabled) { _rotateWithDirection = enabled; }

    /**
     * @brief 播放命中特效(可选)
     */
    void playHitEffect();

protected:
    Projectile();
    virtual ~Projectile();

    bool init(Type type);

private:
    /**
     * @brief 根据类型获取贴图路径
     */
    std::string getTexturePathForType(Type type) const;

    /**
     * @brief 计算飞行方向的旋转角度
     */
    void updateRotationForDirection(const Vec2& direction);

    Type _type;
    bool _rotateWithDirection;  // 是否根据方向旋转(箭矢用)
    std::function<void()> _onHitCallback;
};
