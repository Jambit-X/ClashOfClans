#pragma once

#include "cocos2d.h"

USING_NS_CC;

class DefenseBuildingAnimation : public Node {
public:
    static DefenseBuildingAnimation* create(Node* parentNode, int buildingType);

    virtual bool init(Node* parentNode, int buildingType);

    // ✅ 新增：播放完整攻击动画
    void playAttackAnimation(const Vec2& targetWorldPos, const std::function<void()>& callback = nullptr);
    void playAttackAnimation();  // 无参数版本

    // 现有函数
    void aimAt(const Vec2& targetWorldPos);
    void playFireAnimation(const std::function<void()>& callback = nullptr);
    void playMuzzleFlash();
    void resetBarrel();
    void setVisible(bool visible);

    void setAnimationOffset(const Vec2& offset);
    void setBarrelOffset(const Vec2& offset);

    // 测试函数 - 按顺序显示所有炮管帧
    void testAllBarrelFrames();
private:
    DefenseBuildingAnimation();
    virtual ~DefenseBuildingAnimation();

    void initCannonSprites(Node* parentNode);
    void setBarrelFrame(float angleDegrees);

    int _buildingType;
    Node* _parentNode;

    Sprite* _baseSprite;
    Sprite* _barrelSprite;
    Sprite* _muzzleFlashSprite;

    Vec2 _animationOffset;
    Vec2 _barrelOffset;
    Vec2 _barrelInitialPos;
};
