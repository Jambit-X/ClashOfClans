#define WIN32_LEAN_AND_MEAN // 排除 Windows 头文件中不常用的部分

#include <Windows.h>
#include <tchar.h>
#include "platform/CCStdC.h"
#include "AppDelegate/AppDelegate.h"
#include "cocos2d.h"

USING_NS_CC;

int WINAPI _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // create the application instance
    AppDelegate app;
    return Application::getInstance()->run();
}
