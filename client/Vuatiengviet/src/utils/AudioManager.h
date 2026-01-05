#pragma once
#include <string>
#include <SDL2/SDL_mixer.h>
#include <QString>      // <--- Thay đổi: Dùng QString
#include <QByteArray>

class AudioManager {
public:
    static AudioManager& instance();

    // Music
   void playBackgroundMusic(const QString& resourcePath); 
    void loadClickSound(const QString& resourcePath);
    void stopMusic();

    // Click sound
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

    QByteArray m_musicData;
};
