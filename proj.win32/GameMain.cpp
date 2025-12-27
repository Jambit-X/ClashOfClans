// GameMain.cpp
// Windows平台程序入口点，初始化应用程序并启动游戏主循环

#define WIN32_LEAN_AND_MEAN // 排除Windows头文件中不常用的部分，加快编译速度

#include <Windows.h>
#include <tchar.h>
#include "platform/CCStdC.h"
#include "AppDelegate/AppDelegate.h"
#include "cocos2d.h"

USING_NS_CC;

/**
 * @brief Windows程序入口点
 * @param hInstance 当前应用程序实例句柄
 * @param hPrevInstance 前一个实例句柄（Win32中始终为NULL，保留用于16位Windows兼容）
 * @param lpCmdLine 命令行参数字符串
 * @param nCmdShow 窗口显示方式（如SW_SHOW, SW_HIDE等）
 * @return 应用程序退出代码
 * 
 * 说明：
 * - _tWinMain是Unicode和ANSI兼容的WinMain版本
 * - 创建AppDelegate实例并运行Cocos2d-x应用程序主循环
 */
int WINAPI _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    // 标记未使用的参数，避免编译器警告
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 创建应用程序委托实例（负责初始化游戏场景和逻辑）
    AppDelegate app;
    
    // 启动Cocos2d-x应用程序主循环
    return Application::getInstance()->run();
}
