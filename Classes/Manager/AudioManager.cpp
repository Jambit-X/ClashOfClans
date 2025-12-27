// AudioManager.cpp
// 音频管理器实现，处理所有音频播放和控制

#include "AudioManager.h"

USING_NS_CC;

AudioManager* AudioManager::_instance = nullptr;

AudioManager* AudioManager::getInstance() {
    if (!_instance) {
        _instance = new AudioManager();
    }
    return _instance;
}

void AudioManager::destroyInstance() {
    if (_instance) {
        delete _instance;
        _instance = nullptr;
    }
}

AudioManager::AudioManager() {
    CCLOG("AudioManager: Initialized");
}

AudioManager::~AudioManager() {
    stopAllAudio();
    CCLOG("AudioManager: Destroyed");
}

int AudioManager::playEffect(const std::string& filename, float volume) {
    CCLOG("AudioManager::playEffect - %s (volume: %.2f)", filename.c_str(), volume);
    
    // 检查文件是否存在
    auto fileUtils = cocos2d::FileUtils::getInstance();
    if (!fileUtils->isFileExist(filename)) {
        CCLOG("AudioManager: ERROR - Audio file not found: %s", filename.c_str());
        return -1;
    }
    
    int audioID = cocos2d::experimental::AudioEngine::play2d(filename, false, volume);
    
    if (audioID != cocos2d::experimental::AudioEngine::INVALID_AUDIO_ID) {
        _playingAudios[audioID] = filename;
        CCLOG("AudioManager: Playing audio ID=%d, total active: %zu", audioID, _playingAudios.size());
    } else {
        CCLOG("AudioManager: ERROR - Failed to play audio");
    }
    
    return audioID;
}

int AudioManager::playBackgroundMusic(const std::string& filename, float volume, bool loop) {
    CCLOG("AudioManager::playBackgroundMusic - %s (volume: %.2f, loop: %s)", 
          filename.c_str(), volume, loop ? "YES" : "NO");
    
    // 检查文件是否存在
    auto fileUtils = cocos2d::FileUtils::getInstance();
    if (!fileUtils->isFileExist(filename)) {
        CCLOG("AudioManager: ERROR - Audio file not found: %s", filename.c_str());
        return -1;
    }
    
    int audioID = cocos2d::experimental::AudioEngine::play2d(filename, loop, volume);
    
    if (audioID != cocos2d::experimental::AudioEngine::INVALID_AUDIO_ID) {
        _playingAudios[audioID] = filename;
        CCLOG("AudioManager: Background music playing, ID=%d", audioID);
    } else {
        CCLOG("AudioManager: ERROR - Failed to play background music");
    }
    
    return audioID;
}

void AudioManager::stopAudio(int audioID) {
    if (audioID == cocos2d::experimental::AudioEngine::INVALID_AUDIO_ID) {
        return;
    }
    
    auto it = _playingAudios.find(audioID);
    if (it != _playingAudios.end()) {
        CCLOG("AudioManager: Stopping audio ID=%d (%s)", audioID, it->second.c_str());
        cocos2d::experimental::AudioEngine::stop(audioID);
        _playingAudios.erase(it);
    }
}

void AudioManager::stopAllAudio() {
    CCLOG("AudioManager: Stopping all audio (%zu active)", _playingAudios.size());
    cocos2d::experimental::AudioEngine::stopAll();
    _playingAudios.clear();
}

void AudioManager::pauseAudio(int audioID) {
    if (audioID == cocos2d::experimental::AudioEngine::INVALID_AUDIO_ID) return;
    
    auto it = _playingAudios.find(audioID);
    if (it != _playingAudios.end()) {
        CCLOG("AudioManager: Pausing audio ID=%d", audioID);
        cocos2d::experimental::AudioEngine::pause(audioID);
    }
}

void AudioManager::resumeAudio(int audioID) {
    if (audioID == cocos2d::experimental::AudioEngine::INVALID_AUDIO_ID) return;
    
    auto it = _playingAudios.find(audioID);
    if (it != _playingAudios.end()) {
        CCLOG("AudioManager: Resuming audio ID=%d", audioID);
        cocos2d::experimental::AudioEngine::resume(audioID);
    }
}

void AudioManager::setVolume(int audioID, float volume) {
    if (audioID == cocos2d::experimental::AudioEngine::INVALID_AUDIO_ID) return;
    
    cocos2d::experimental::AudioEngine::setVolume(audioID, volume);
    
    auto it = _playingAudios.find(audioID);
    if (it != _playingAudios.end()) {
        CCLOG("AudioManager: Set volume ID=%d to %.2f", audioID, volume);
    }
}

void AudioManager::preloadAudio(const std::string& filename) {
    CCLOG("AudioManager: Preloading audio %s", filename.c_str());
    
    cocos2d::experimental::AudioEngine::preload(filename, [filename](bool success) {
        if (success) {
            CCLOG("AudioManager: Successfully preloaded %s", filename.c_str());
        } else {
            CCLOG("AudioManager: ERROR - Failed to preload %s", filename.c_str());
        }
    });
}

void AudioManager::unloadAudio(const std::string& filename) {
    cocos2d::experimental::AudioEngine::uncache(filename);
    CCLOG("AudioManager: Unloaded audio %s", filename.c_str());
}
