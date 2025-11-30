/*******************
 * StartupScene: 启动等待动画
 *******************/

#pragma once
#ifndef _STARTUP_SCENE_H_
#define _STARTUP_SCENE_H_

#include "cocos2d.h"

/*
 * Class Name : StartupScene
 * Class Function : 启动等待动画
*/ 

class StartupScene : public cocos2d::Scene {
public:
    // 创建
    static cocos2d::Scene* createScene();

    // 初始化
    virtual bool init();

    // 
    CREATE_FUNC(StartupScene);
};

#endif // !_STARTUP_SCENE_H_