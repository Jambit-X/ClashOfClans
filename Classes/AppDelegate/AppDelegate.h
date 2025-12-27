// AppDelegate.h
// 应用程序委托类声明，定义应用生命周期回调接口

#ifndef  _APP_DELEGATE_H_
#define  _APP_DELEGATE_H_

#include "cocos2d.h"

// 应用程序委托类
// 通过私有继承隐藏部分Director接口
class  AppDelegate : private cocos2d::Application
{
public:
    AppDelegate();
    virtual ~AppDelegate();

    // 初始化OpenGL上下文属性
    virtual void initGLContextAttrs();

    // 应用启动完成时调用，初始化Director和Scene
    // 返回true表示初始化成功，应用继续运行
    // 返回false表示初始化失败，应用终止
    virtual bool applicationDidFinishLaunching();

    // 应用进入后台时调用
    virtual void applicationDidEnterBackground();

    // 应用返回前台时调用
    virtual void applicationWillEnterForeground();
};

#endif // _APP_DELEGATE_H_

