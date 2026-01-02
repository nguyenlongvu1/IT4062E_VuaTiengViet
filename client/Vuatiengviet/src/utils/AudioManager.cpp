#include "AudioManager.h"
#include <QDebug>
#include <SDL2/SDL.h>

// ===== Singleton =====
AudioManager& AudioManager::instance() {
    static AudioManager instance;
    return instance;
}

// ===== INIT =====
AudioManager::AudioManager() {
    SDL_setenv("SDL_AUDIODRIVER", "alsa", 1);

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        qDebug() << "SDL_Init failed:" << SDL_GetError();
        return;
    }

    int flags = MIX_INIT_OGG | MIX_INIT_MP3;
    if ((Mix_Init(flags) & flags) != flags) {
        qDebug() << "Mix_Init failed:" << Mix_GetError();
        return;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        qDebug() << "Mix_OpenAudio failed:" << Mix_GetError();
        return;
    }

    Mix_AllocateChannels(16);
}

// ===== DESTROY =====
AudioManager::~AudioManager() {
    stopMusic();

    if (bgMusic) {
        Mix_FreeMusic(bgMusic);
        bgMusic = nullptr;
    }

    if (clickChunk) {
        Mix_FreeChunk(clickChunk);
        clickChunk = nullptr;
    }

    Mix_CloseAudio();
    Mix_Quit();
    SDL_Quit();
}

// ===== MUSIC =====
void AudioManager::playBackgroundMusic(const std::string& path) {
    stopMusic();

    if (bgMusic) {
        Mix_FreeMusic(bgMusic);
        bgMusic = nullptr;
    }

    bgMusic = Mix_LoadMUS(path.c_str());
    if (!bgMusic) {
        qDebug() << "Mix_LoadMUS failed:" << Mix_GetError();
        return;
    }

    Mix_VolumeMusic((volume * MIX_MAX_VOLUME) / 100);
    Mix_PlayMusic(bgMusic, -1);
}

void AudioManager::stopMusic() {
    Mix_HaltMusic();
}

// ===== CLICK SOUND =====
void AudioManager::loadClickSound(const std::string& path)
{
    if (clickChunk) {
        Mix_FreeChunk(clickChunk);
        clickChunk = nullptr;
    }

    clickChunk = Mix_LoadWAV(path.c_str());
    if (!clickChunk) {
        qDebug() << "Load click failed:" << Mix_GetError();
        return;
    }

    Mix_VolumeChunk(clickChunk, (sfxVolume * MIX_MAX_VOLUME) / 100);
}

void AudioManager::playClickSound()
{
    if (!clickChunk) return;
    Mix_PlayChannel(-1, clickChunk, 0);
}
void AudioManager::setVolume(int v) {
    volume = std::clamp(v, 0, 100);
    Mix_VolumeMusic((volume * MIX_MAX_VOLUME) / 100);
}

int AudioManager::getVolume() const {
    return volume;
}

void AudioManager::setSFXVolume(int v) {
    sfxVolume = std::clamp(v, 0, 100);
    if (clickChunk) {
        Mix_VolumeChunk(clickChunk, (sfxVolume * MIX_MAX_VOLUME) / 100);
    }
}

int AudioManager::getSFXVolume() const {
    return sfxVolume;
}
