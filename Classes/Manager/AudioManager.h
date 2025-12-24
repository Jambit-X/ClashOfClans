#pragma once

#include "cocos2d.h"
#include "audio/include/AudioEngine.h"
#include <string>
#include <map>

USING_NS_CC;

/**
 * @brief 音频管理器 - 管理游戏中的所有音频播放
 */
class AudioManager {
public:
    static AudioManager* getInstance();
    static void destroyInstance();
    
    /**
     * @brief 播放音效（一次性）
     * @param filename 音频文件路径（相对于 Resources 目录）
     * @param volume 音量 (0.0 - 1.0)，默认 1.0
     * @return 音频 ID（可用于停止播放）
     */
    int playEffect(const std::string& filename, float volume = 1.0f);
    
    /**
     * @brief 播放背景音乐（循环）
     * @param filename 音频文件路径
     * @param volume 音量 (0.0 - 1.0)，默认 1.0
     * @param loop 是否循环，默认 true
     * @return 音频 ID
     */
    int playBackgroundMusic(const std::string& filename, float volume = 1.0f, bool loop = true);
    
    /**
     * @brief 停止指定音频
     * @param audioID 音频 ID
     */
    void stopAudio(int audioID);
    
    /**
     * @brief 停止所有音频
     */
    void stopAllAudio();
    
    /**
     * @brief 暂停指定音频
     * @param audioID 音频 ID
     */
    void pauseAudio(int audioID);
    
    /**
     * @brief 恢复指定音频
     * @param audioID 音频 ID
     */
    void resumeAudio(int audioID);
    
    /**
     * @brief 设置音频音量
     * @param audioID 音频 ID
     * @param volume 音量 (0.0 - 1.0)
     */
    void setVolume(int audioID, float volume);
    
    /**
     * @brief 预加载音频文件
     * @param filename 音频文件路径
     */
    void preloadAudio(const std::string& filename);
    
    /**
     * @brief 卸载音频文件
     * @param filename 音频文件路径
     */
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
