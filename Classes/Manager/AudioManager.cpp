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
    CCLOG("==============================================");
    CCLOG("AudioManager: Initialized");
    CCLOG("==============================================");
}

AudioManager::~AudioManager() {
    // 停止所有音频
    stopAllAudio();
    CCLOG("AudioManager: Destroyed");
}

int AudioManager::playEffect(const std::string& filename, float volume) {
    CCLOG("----------------------------------------------");
    CCLOG("AudioManager::playEffect - START");
    CCLOG("  Filename: %s", filename.c_str());
    CCLOG("  Volume: %.2f", volume);
    
    // 检查文件是否存在
    auto fileUtils = cocos2d::FileUtils::getInstance();
    std::string fullPath = fileUtils->fullPathForFilename(filename);
    bool fileExists = fileUtils->isFileExist(filename);
    
    CCLOG("  Full path: %s", fullPath.c_str());
    CCLOG("  File exists: %s", fileExists ? "YES" : "NO");
    
    if (!fileExists) {
        CCLOG("  ERROR: Audio file not found!");
        CCLOG("----------------------------------------------");
        return -1;
    }
    
    int audioID = cocos2d::experimental::AudioEngine::play2d(filename, false, volume);
    
    if (audioID != cocos2d::experimental::AudioEngine::INVALID_AUDIO_ID) {
        _playingAudios[audioID] = filename;
        CCLOG("  SUCCESS: Audio playing with ID = %d", audioID);
        CCLOG("  Total playing audios: %zu", _playingAudios.size());
    } else {
        CCLOG("  ERROR: Failed to play audio! (ID = -1)");
    }
    
    CCLOG("AudioManager::playEffect - END");
    CCLOG("----------------------------------------------");
    
    return audioID;
}

int AudioManager::playBackgroundMusic(const std::string& filename, float volume, bool loop) {
    CCLOG("==============================================");
    CCLOG("AudioManager::playBackgroundMusic - START");
    CCLOG("  Filename: %s", filename.c_str());
    CCLOG("  Volume: %.2f", volume);
    CCLOG("  Loop: %s", loop ? "YES" : "NO");
    
    // 检查文件是否存在
    auto fileUtils = cocos2d::FileUtils::getInstance();
    std::string fullPath = fileUtils->fullPathForFilename(filename);
    bool fileExists = fileUtils->isFileExist(filename);
    
    CCLOG("  Full path resolved: %s", fullPath.c_str());
    CCLOG("  File exists: %s", fileExists ? "YES" : "NO");
    
    if (!fileExists) {
        CCLOG("  ERROR: Audio file not found!");
        CCLOG("  Possible reasons:");
        CCLOG("    1. File not in Resources/Audios/ folder");
        CCLOG("    2. File name mismatch (check spelling/case)");
        CCLOG("    3. File not included in CMakeLists.txt");
        CCLOG("==============================================");
        return -1;
    }
    
    int audioID = cocos2d::experimental::AudioEngine::play2d(filename, loop, volume);
    
    if (audioID != cocos2d::experimental::AudioEngine::INVALID_AUDIO_ID) {
        _playingAudios[audioID] = filename;
        CCLOG("  SUCCESS: Background music playing");
        CCLOG("    Audio ID: %d", audioID);
        CCLOG("    Total active audios: %zu", _playingAudios.size());
        
        // 获取音频状态
        auto state = cocos2d::experimental::AudioEngine::getState(audioID);
        CCLOG("    Audio State: %d (0=Error, 1=Initializing, 2=Playing, 3=Paused)", (int)state);
    } else {
        CCLOG("  ERROR: Failed to play background music! (ID = -1)");
        CCLOG("  Possible reasons:");
        CCLOG("    1. Audio format not supported (try MP3/OGG/WAV)");
        CCLOG("    2. File corrupted");
        CCLOG("    3. Audio engine not initialized");
    }
    
    CCLOG("AudioManager::playBackgroundMusic - END");
    CCLOG("==============================================");
    
    return audioID;
}

void AudioManager::stopAudio(int audioID) {
    if (audioID == cocos2d::experimental::AudioEngine::INVALID_AUDIO_ID) {
        CCLOG("AudioManager::stopAudio - Invalid audio ID, ignoring");
        return;
    }
    
    auto it = _playingAudios.find(audioID);
    if (it != _playingAudios.end()) {
        CCLOG("----------------------------------------------");
        CCLOG("AudioManager::stopAudio");
        CCLOG("  Stopping: %s (ID: %d)", it->second.c_str(), audioID);
        
        cocos2d::experimental::AudioEngine::stop(audioID);
        _playingAudios.erase(it);
        
        CCLOG("  Remaining active audios: %zu", _playingAudios.size());
        CCLOG("----------------------------------------------");
    } else {
        CCLOG("AudioManager::stopAudio - Audio ID %d not found in playing list", audioID);
    }
}

void AudioManager::stopAllAudio() {
    CCLOG("==============================================");
    CCLOG("AudioManager::stopAllAudio");
    CCLOG("  Stopping %zu active audio(s)", _playingAudios.size());
    
    for (const auto& pair : _playingAudios) {
        CCLOG("    - Stopping: %s (ID: %d)", pair.second.c_str(), pair.first);
    }
    
    cocos2d::experimental::AudioEngine::stopAll();
    _playingAudios.clear();
    
    CCLOG("  All audio stopped");
    CCLOG("==============================================");
}

void AudioManager::pauseAudio(int audioID) {
    if (audioID == cocos2d::experimental::AudioEngine::INVALID_AUDIO_ID) return;
    
    auto it = _playingAudios.find(audioID);
    if (it != _playingAudios.end()) {
        CCLOG("AudioManager: Pausing audio '%s' (ID: %d)", it->second.c_str(), audioID);
        cocos2d::experimental::AudioEngine::pause(audioID);
    }
}

void AudioManager::resumeAudio(int audioID) {
    if (audioID == cocos2d::experimental::AudioEngine::INVALID_AUDIO_ID) return;
    
    auto it = _playingAudios.find(audioID);
    if (it != _playingAudios.end()) {
        CCLOG("AudioManager: Resuming audio '%s' (ID: %d)", it->second.c_str(), audioID);
        cocos2d::experimental::AudioEngine::resume(audioID);
    }
}

void AudioManager::setVolume(int audioID, float volume) {
    if (audioID == cocos2d::experimental::AudioEngine::INVALID_AUDIO_ID) return;
    
    cocos2d::experimental::AudioEngine::setVolume(audioID, volume);
    
    auto it = _playingAudios.find(audioID);
    if (it != _playingAudios.end()) {
        CCLOG("AudioManager: Set volume for '%s' (ID: %d) to %.2f", 
              it->second.c_str(), audioID, volume);
    }
}

void AudioManager::preloadAudio(const std::string& filename) {
    CCLOG("AudioManager: Preloading audio '%s'...", filename.c_str());
    
    cocos2d::experimental::AudioEngine::preload(filename, [filename](bool success) {
        if (success) {
            CCLOG("AudioManager: Successfully preloaded '%s'", filename.c_str());
        } else {
            CCLOG("AudioManager: ERROR - Failed to preload '%s'", filename.c_str());
        }
    });
}

void AudioManager::unloadAudio(const std::string& filename) {
    cocos2d::experimental::AudioEngine::uncache(filename);
    CCLOG("AudioManager: Unloaded audio '%s'", filename.c_str());
}
