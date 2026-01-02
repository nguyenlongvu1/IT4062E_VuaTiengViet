#pragma once
#include <string>
#include <SDL2/SDL_mixer.h>

class AudioManager {
public:
    static AudioManager& instance();

    // Music
    void playBackgroundMusic(const std::string& path);
    void stopMusic();

    // Click sound
    void loadClickSound(const std::string& path);
    void playClickSound();

    // Volume
    void setVolume(int v);
    int getVolume() const;

     void setSFXVolume(int v);
    int getSFXVolume() const;

private:
    AudioManager();
    ~AudioManager();

    Mix_Music* bgMusic = nullptr;
    Mix_Chunk* clickChunk = nullptr;

    int volume = 100;
    int sfxVolume = 100;
};
