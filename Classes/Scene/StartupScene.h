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
    // --- 阶段一：Supercell Splash 元素 ---
    cocos2d::LayerColor* _splashLayer; // 黑色背景层
    cocos2d::Sprite* _logo;            // Supercell Logo
    cocos2d::Label* _legalTextLabel;   // 动态法律文本

    // --- 阶段二：Loading 元素 ---
    cocos2d::Sprite* _loadingBg;       // 游戏加载图
    cocos2d::ui::LoadingBar* _progressBar; // 进度条
    
    // ========== 音频 ID ==========
    int _supercellJingleID;   // Supercell 音效 ID
    int _startupJingleID;     // 启动场景音效 ID

    // --- 逻辑函数 ---
    void showSplashPhase();       // 显示 Logo 和文字
    void showLoadingPhase(float dt); // 切换到加载条阶段
    void updateLoadingBar(float dt); // 跑进度条逻辑
    void goToVillageScene(float dt); // 跳转场景
};

#endif // __STARTUP_SCENE_H__
