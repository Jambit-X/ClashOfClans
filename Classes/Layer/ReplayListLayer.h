// ReplayListLayer.h
// 回放列表层声明，显示所有战斗回放记录

#ifndef __REPLAY_LIST_LAYER_H__
#define __REPLAY_LIST_LAYER_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Model/ReplayData.h"
#include <vector>
#include <string>

class ReplayListLayer : public cocos2d::Layer {
public:
    CREATE_FUNC(ReplayListLayer);
    virtual bool init() override;

private:
    cocos2d::ui::ScrollView* _scrollView;
    cocos2d::Node* _contentNode;

    // UI创建方法
    void loadReplayList();
    void createReplayCard(const ReplayMetadata& replay, float yPosition);

    // 事件处理
    void onWatchClicked(int replayId);
    void onDeleteClicked(int replayId);
    void onCloseClicked();

    // 辅助方法
    std::string getTimeAgo(time_t timestamp);
    std::string getTroopIconPath(int troopId);

    // 卡片尺寸常量
    const float CARD_WIDTH = 1400.0f;
    const float CARD_HEIGHT = 140.0f;
    const float CARD_SPACING = 15.0f;
};

#endif // __REPLAY_LIST_LAYER_H__
