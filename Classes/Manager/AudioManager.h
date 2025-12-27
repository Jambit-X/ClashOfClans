// AudioManager.h
// 音频管理器头文件，管理游戏中的所有音频播放

#pragma once

#include "cocos2d.h"
#include "audio/include/AudioEngine.h"
#include <string>
#include <map>

USING_NS_CC;

class AudioManager {
public:
    static AudioManager* getInstance();
    static void destroyInstance();
    
    // 播放音效（一次性）
    int playEffect(const std::string& filename, float volume = 1.0f);
    
    // 播放背景音乐（循环）
    int playBackgroundMusic(const std::string& filename, float volume = 1.0f, bool loop = true);
    
    // 停止指定音频
    void stopAudio(int audioID);
    
    // 停止所有音频
    void stopAllAudio();
    
    // 暂停指定音频
    void pauseAudio(int audioID);
    
    // 恢复指定音频
    void resumeAudio(int audioID);
    
    // 设置音频音量
    void setVolume(int audioID, float volume);
    
    // 预加载音频文件
    void preloadAudio(const std::string& filename);
    
    // 卸载音频文件
    void unloadAudio(const std::string& filename);
    
private:
    AudioManager();
    ~AudioManager();
    
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
    
    static AudioManager* _instance;
    
    // 记录正在播放的音频
    std::map<int, std::string> _playingAudios;
};
