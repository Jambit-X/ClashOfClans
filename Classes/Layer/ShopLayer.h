// ShopLayer.h
// 商店层声明，处理建筑和道具的购买功能

#ifndef __SHOP_LAYER_H__
#define __SHOP_LAYER_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <vector>
#include <string>

// 商店商品数据结构
struct ShopItemData {
    int id;
    std::string name;
    std::string imagePath;
    int cost;
    std::string costType;  // "金币", "圣水", "宝石"
    std::string time;
};

class ShopLayer : public cocos2d::Layer {
public:
    static cocos2d::Scene* createScene();
    CREATE_FUNC(ShopLayer);
    virtual bool init() override;

private:
    // UI初始化
    void initTabs();
    void initScrollView();
    void initBottomBar();

    // 逻辑控制
    void switchTab(int categoryIndex);  // 切换标签: 0=军队, 1=资源, 2=防御, 3=陷阱
    void addShopItem(const ShopItemData& data, int index);

    // 数据源
    std::vector<ShopItemData> getDummyData(int categoryIndex);

    // 回调函数
    void onCloseClicked(cocos2d::Ref* sender);
    void onTabClicked(cocos2d::Ref* sender, int index);
    void onPurchaseBuilding(const ShopItemData& data);

    cocos2d::ui::ScrollView* _scrollView;
    std::vector<cocos2d::ui::Button*> _tabButtons;

    // 布局常量配置
    const float TAB_WIDTH = 180.0f;
    const float TAB_HEIGHT = 50.0f;
    const float TAB_SPACING = 10.0f;
    const float TAB_TOP_OFFSET = 130.0f;

    // 颜色配置
    const cocos2d::Color4B COLOR_TAB_NORMAL = cocos2d::Color4B(50, 50, 50, 255);
    const cocos2d::Color4B COLOR_TAB_SELECT = cocos2d::Color4B(100, 200, 50, 255);

    void showErrorDialog(const std::string& message);
    void showTips(const std::string& message, const cocos2d::Color3B& color);
};

#endif // __SHOP_LAYER_H__
