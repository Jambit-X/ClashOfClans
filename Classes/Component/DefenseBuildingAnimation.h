#pragma once
#include "cocos2d.h"

USING_NS_CC;

/**
 * 防御建筑动画控制器
 * 职责：只负责动画播放，不涉及 AI 逻辑
 * 使用者：防御建筑 AI 组件
 */
class DefenseBuildingAnimation : public cocos2d::Node {  // ✅ 继承 Ref
public:
    /**
     * 创建加农炮动画控制器
     * @param parentNode 建筑精灵节点
     * @param buildingType 建筑类型 (301=加农炮, 302=箭塔)
     * @return 动画控制器实例
     */
    static DefenseBuildingAnimation* create(Node* parentNode, int buildingType);

    // ========== 核心动画接口（给 AI 组件调用）==========

    /**
     * 旋转炮管朝向目标
     * @param targetWorldPos 目标的世界坐标
     */
    void aimAt(const Vec2& targetWorldPos);

    /**
     * 播放开火动画
     * @param callback 开火完成回调（可选）
     */
    void playFireAnimation(const std::function<void()>& callback = nullptr);

    /**
     * 重置炮管到默认角度（朝下）
     */
    void resetBarrel();

    /**
     * 隐藏/显示动画（建筑被摧毁时隐藏）
     */
    void setVisible(bool visible);

    // 设置动画整体偏移量（用于微调位置）
    void setAnimationOffset(const Vec2& offset);

    // 设置炮管相对偏移量
    void setBarrelOffset(const Vec2& offset);

protected:  // ✅ 改为 protected（Ref 子类的规范）
    DefenseBuildingAnimation();
    virtual ~DefenseBuildingAnimation();

    bool init(Node* parentNode, int buildingType);

private:
    // ========== 内部实现 ==========

    /**
     * 初始化加农炮结构（底座 + 炮管）
     */
    void initCannonSprites(Node* parentNode);

    /**
     * 根据角度设置炮管帧
     */
    void setBarrelFrame(float angleDegrees);

    /**
     * 播放炮口火焰特效
     */
    void playMuzzleFlash();

    // ========== 成员变量 ==========

    int _buildingType;              // 建筑类型
    Node* _parentNode;              // 父节点（建筑精灵）
    Sprite* _baseSprite;            // 底座精灵
    Sprite* _barrelSprite;          // 炮管精灵
    Sprite* _muzzleFlashSprite;     // 炮口火焰特效
    // 偏移量配置
    Vec2 _animationOffset;  // 整体偏移
    Vec2 _barrelOffset;     // 炮管额外偏移
};
