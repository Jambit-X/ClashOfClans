// StartupScene.h
// 启动场景，包含Supercell闪屏和加载进度

#ifndef __STARTUP_SCENE_H__
#define __STARTUP_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

class StartupScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    CREATE_FUNC(StartupScene);

private:
    // Supercell闪屏阶段元素
    cocos2d::LayerColor* _splashLayer; // 黑色背景层
    cocos2d::Sprite* _logo;            // Supercell Logo
    cocos2d::Label* _legalTextLabel;   // 法律文本

    // 加载阶段元素
    cocos2d::Sprite* _loadingBg;       // 加载背景图
    cocos2d::ui::LoadingBar* _progressBar; // 进度条
    
    // 音频ID
    int _supercellJingleID;   // Supercell音效ID
    int _startupJingleID;     // 加载音效ID

    // 阶段切换函数
    void showSplashPhase();       // 显示Supercell闪屏
    void showLoadingPhase(float dt); // 切换到加载阶段
    void updateLoadingBar(float dt); // 更新进度条
    void goToVillageScene(float dt); // 跳转到村庄场景
};

#endif // __STARTUP_SCENE_H__
