// DefenseBuildingAnimation.h
// 防御建筑动画组件声明，管理炮塔等防御建筑的攻击动画

#pragma once

#include "cocos2d.h"

USING_NS_CC;

// 防御建筑动画管理类
class DefenseBuildingAnimation : public Node {
public:
    static DefenseBuildingAnimation* create(Node* parentNode, int buildingType);

    virtual bool init(Node* parentNode, int buildingType);

    // 播放完整攻击动画（瞄准+开火）
    void playAttackAnimation(const Vec2& targetWorldPos, const std::function<void()>& callback = nullptr);
    void playAttackAnimation();

    // 瞄准目标位置
    void aimAt(const Vec2& targetWorldPos);
    
    // 播放开火动画
    void playFireAnimation(const std::function<void()>& callback = nullptr);
    
    // 播放炮口火焰特效
    void playMuzzleFlash();
    
    // 重置炮管朝向
    void resetBarrel();
    
    // 设置可见性
    void setVisible(bool visible);

    // 设置动画偏移
    void setAnimationOffset(const Vec2& offset);
    void setBarrelOffset(const Vec2& offset);

    // 测试函数：按顺序显示所有炮管帧
    void testAllBarrelFrames();
    
private:
    DefenseBuildingAnimation();
    virtual ~DefenseBuildingAnimation();

    // 初始化加农炮精灵
    void initCannonSprites(Node* parentNode);
    
    // 根据角度设置炮管帧
    void setBarrelFrame(float angleDegrees);

    int _buildingType;              // 建筑类型
    Node* _parentNode;              // 父节点

    Sprite* _baseSprite;            // 底座精灵
    Sprite* _barrelSprite;          // 炮管精灵
    Sprite* _muzzleFlashSprite;     // 炮口火焰精灵

    Vec2 _animationOffset;          // 动画整体偏移
    Vec2 _barrelOffset;             // 炮管偏移
    Vec2 _barrelInitialPos;         // 炮管初始位置
};
