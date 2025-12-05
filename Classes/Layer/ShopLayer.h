#ifndef __SHOP_LAYER_H__
#define __SHOP_LAYER_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <vector>
#include <string>

// 商店商品数据结构
struct ShopItemData {
    int id;                 // 唯一ID
    std::string name;       // 显示名称
    std::string imagePath;  // 图片路径
    int cost;               // 价格数值
    std::string costType;   // 货币单位: "金币", "圣水", "宝石"
    std::string time;       // 建造时间描述
};

class ShopLayer : public cocos2d::Layer {
public:
    static cocos2d::Scene* createScene(); // 方便单独调试
    CREATE_FUNC(ShopLayer);
    virtual bool init() override;

private:
    // --- UI 初始化 ---
    void initTabs();         // 初始化顶部标签
    void initScrollView();   // 初始化中间滚动列表
    void initBottomBar();    // 初始化底部资源条

    // --- 逻辑控制 ---
    // 切换标签: 0=军队, 1=资源, 2=防御, 3=陷阱
    void switchTab(int categoryIndex);

    // 向列表添加一个商品卡片
    void addShopItem(const ShopItemData& data, int index);

    // --- 数据源 ---
    std::vector<ShopItemData> getDummyData(int categoryIndex);

    // --- 回调函数 ---
    void onCloseClicked(cocos2d::Ref* sender);
    void onTabClicked(cocos2d::Ref* sender, int index);

private:
    cocos2d::ui::ScrollView* _scrollView;
    std::vector<cocos2d::ui::Button*> _tabButtons; // 保存标签按钮引用，用于变色

    // --- 布局常量配置 (在此处调整大小) ---
    const float TAB_WIDTH = 180.0f;   // 标签宽度
    const float TAB_HEIGHT = 50.0f;   // 标签高度 (长条形)
    const float TAB_SPACING = 10.0f;  // 标签之间的间隙

    // 标签栏底部距离屏幕顶部的偏移量 (越大越靠下)
    // 之前可能只有 80~100，现在设大一点，让它整体下移
    const float TAB_TOP_OFFSET = 130.0f;

    // --- 颜色配置 ---
    const cocos2d::Color4B COLOR_TAB_NORMAL = cocos2d::Color4B(50, 50, 50, 255);   // 未选中(深灰)
    const cocos2d::Color4B COLOR_TAB_SELECT = cocos2d::Color4B(100, 200, 50, 255); // 选中(绿)
};

#endif // __SHOP_LAYER_H__