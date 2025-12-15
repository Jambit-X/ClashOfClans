#/****************************************************************************
# Copyright (c) 2013-2014 cocos2d-x.org
# Copyright (c) 2015-2017 Chukong Technologies Inc.
#
# http://www.cocos2d-x.org
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# ****************************************************************************/

cmake_minimum_required(VERSION 3.6)

set(APP_NAME ClashOfClans)

project(${APP_NAME})

set(COCOS2DX_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cocos2d)
set(CMAKE_MODULE_PATH ${COCOS2DX_ROOT_PATH}/cmake/Modules/)

include(CocosBuildSet)
add_subdirectory(${COCOS2DX_ROOT_PATH}/cocos ${ENGINE_BINARY_PATH}/cocos/core)

# record sources, headers, resources...
set(GAME_SOURCE)
set(GAME_HEADER)

set(GAME_RES_FOLDER
    "${CMAKE_CURRENT_SOURCE_DIR}/Resources"
    )
if(APPLE OR WINDOWS)
    cocos_mark_multi_resources(common_res_files RES_TO "Resources" FOLDERS ${GAME_RES_FOLDER})
endif()

# add cross-platforms source files and header files 
list(APPEND GAME_SOURCE
     Classes/AppDelegate/AppDelegate.cpp
     Classes/Scene/StartupScene.cpp
     Classes/Scene/VillageScene.cpp
     Classes/Scene/BattleScene.cpp
     Classes/Layer/BattleTroopLayer.cpp
     Classes/Layer/VillageLayer.cpp
     Classes/Layer/ShopLayer.cpp
     Classes/Layer/HUDLayer.cpp
     Classes/Layer/BattleMapLayer.cpp
     Classes/Layer/BattleHUDLayer.cpp
     Classes/Layer/BattleResultLayer.cpp
     Classes/Layer/DebugLayer.cpp
     Classes/Component/ConstructionAnimation.cpp
     Classes/Layer/TrainingLayer.cpp
     Classes/Controller/MoveMapController.cpp
     Classes/Controller/MoveBuildingController.cpp
     Classes/Controller/BuildingPlacementController.cpp
     Classes/Controller/BattleProcessController.cpp
     Classes/Manager/Resource/ResourceProductionSystem.cpp
     Classes/Manager/AnimationManager.cpp
     Classes/Manager/BuildingManager.cpp
     Classes/Manager/BuildingSpeedupManager.cpp
     Classes/Manager/BuildingUpgradeManager.cpp
     Classes/Manager/VillageDataManager.cpp
     Classes/Model/BuildingRequirements.cpp
     Classes/Model/BuildingConfig.cpp
     Classes/Model/TroopConfig.cpp
     Classes/Util/GridMapUtils.cpp
     Classes/Sprite/BuildingSprite.cpp
     Classes/Sprite/BattleUnitSprite.cpp
     Classes/UI/PlacementConfirmUI.cpp
     Classes/UI/ResourceCollectionUI.cpp
     Classes/Util/GridMapUtils.cpp
     Classes/Util/FindPathUtil.cpp
     Classes/Util/FindPathUtil.cpp
     Classes/Util/DebugHelper.cpp
     Classes/Layer/DebugLayer.cpp
     )
list(APPEND GAME_HEADER
     ../proj.win32/Constants.h
     Classes/AppDelegate/AppDelegate.h
     Classes/Scene/StartupScene.h
     Classes/Scene/VillageScene.h
     Classes/Scene/BattleScene.h
     Classes/Layer/BattleTroopLayer.h
     Classes/Layer/ShopLayer.h
     Classes/Layer/VillageLayer.h
     Classes/Layer/HUDLayer.h
     Classes/Layer/BattleMapLayer.h
     Classes/Layer/BattleHUDLayer.h
     Classes/Layer/BattleResultLayer.h
     Classes/Layer/DebugLayer.h
     Classes/Component/ConstructionAnimation.h
     Classes/Layer/TrainingLayer.h
     Classes/Controller/MoveMapController.h
     Classes/Controller/MoveBuildingController.h
     Classes/Controller/BuildingPlacementController.h
     Classes/Controller/BattleProcessController.h
     Classes/Manager/Resource/ResourceProductionSystem.h
     Classes/Manager/AnimationManager.h
     Classes/Manager/BuildingSpeedupManager.h
     Classes/Manager/BuildingUpgradeManager.h
     Classes/Manager/BuildingManager.h
     Classes/Manager/VillageDataManager.h
     Classes/Model/VillageData.h
     Classes/Model/BuildingRequirements.h
     Classes/Model/BuildingConfig.h
     Classes/Model/TroopConfig.h
     Classes/Util/GridMapUtils.h
     Classes/Sprite/BuildingSprite.h
     Classes/Sprite/BattleUnitSprite.h
     Classes/UI/PlacementConfirmUI.h
     Classes/UI/ResourceCollectionUI.h
     Classes/Util/GridMapUtils.h
     Classes/Util/FindPathUtil.h
     Classes/Util/FindPathUtil.h
     Classes/Util/DebugHelper.h
     Classes/Layer/DebugLayer.h
     )

if(ANDROID)
    # change APP_NAME to the share library name for Android, it's value depend on AndroidManifest.xml
    set(APP_NAME MyGame)
    list(APPEND GAME_SOURCE
         proj.android/app/jni/hellocpp/main.cpp
         )
elseif(LINUX)
    list(APPEND GAME_SOURCE
         proj.linux/main.cpp
         )
elseif(WINDOWS)
    list(APPEND GAME_HEADER
         proj.win32/resource.h
         proj.win32/Constants.h
         )
    list(APPEND GAME_SOURCE
         proj.win32/GameMain.cpp
         proj.win32/game.rc
         ${common_res_files}
         )
elseif(APPLE)
    if(IOS)
        list(APPEND GAME_HEADER
             proj.ios_mac/ios/AppController.h
             proj.ios_mac/ios/RootViewController.h
             )
        set(APP_UI_RES
            proj.ios_mac/ios/LaunchScreen.storyboard
            proj.ios_mac/ios/LaunchScreenBackground.png
            proj.ios_mac/ios/Images.xcassets
            )
        list(APPEND GAME_SOURCE
             proj.ios_mac/ios/main.m
             proj.ios_mac/ios/AppController.mm
             proj.ios_mac/ios/RootViewController.mm
             proj.ios_mac/ios/Prefix.pch
             ${APP_UI_RES}
             )
    elseif(MACOSX)
        set(APP_UI_RES
            proj.ios_mac/mac/Icon.icns
            proj.ios_mac/mac/Info.plist
            )
        list(APPEND GAME_SOURCE
             proj.ios_mac/mac/main.cpp
             proj.ios_mac/mac/Prefix.pch
             ${APP_UI_RES}
             )
    endif()
    list(APPEND GAME_SOURCE ${common_res_files})
endif()

# mark app complie info and libs info
set(all_code_files
    ${GAME_HEADER}
    ${GAME_SOURCE}
    )
if(NOT ANDROID)
    add_executable(${APP_NAME} ${all_code_files})
else()
    add_library(${APP_NAME} SHARED ${all_code_files})
    add_subdirectory(${COCOS2DX_ROOT_PATH}/cocos/platform/android ${ENGINE_BINARY_PATH}/cocos/platform)
    target_link_libraries(${APP_NAME} -Wl,--whole-archive cpp_android_spec -Wl,--no-whole-archive)
endif()

target_link_libraries(${APP_NAME} cocos2d)
target_include_directories(${APP_NAME}
        PRIVATE Classes
        PRIVATE ${COCOS2DX_ROOT_PATH}/cocos/audio/include/
)

# mark app resources
setup_cocos_app_config(${APP_NAME})
if(APPLE)
    set_target_properties(${APP_NAME} PROPERTIES RESOURCE "${APP_UI_RES}")
    if(MACOSX)
        set_target_properties(${APP_NAME} PROPERTIES
                              MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/proj.ios_mac/mac/Info.plist"
                              )
    elseif(IOS)
        cocos_pak_xcode(${APP_NAME} INFO_PLIST "iOSBundleInfo.plist.in")
        set_xcode_property(${APP_NAME} ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon")
        set_xcode_property(${APP_NAME} DEVELOPMENT_TEAM "")
        set_xcode_property(${APP_NAME} CODE_SIGN_IDENTITY "iPhone Developer")
    endif()
elseif(WINDOWS)
    cocos_copy_target_dll(${APP_NAME})
endif()

if(LINUX OR WINDOWS)
    set(APP_RES_DIR "$<TARGET_FILE_DIR:${APP_NAME}>/Resources")
    cocos_copy_target_res(${APP_NAME} COPY_TO ${APP_RES_DIR} FOLDERS ${GAME_RES_FOLDER})
endif()
