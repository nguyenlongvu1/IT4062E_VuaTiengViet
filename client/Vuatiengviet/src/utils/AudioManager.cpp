#include "AudioManager.h"
#include <QDebug>
#include <QFile>
#include <SDL2/SDL.h> 
#include <SDL2/SDL_mixer.h>

// ===== Singleton =====
AudioManager& AudioManager::instance() {
    static AudioManager instance;
    return instance;
}

// ===== INIT =====
AudioManager::AudioManager() {
    // SDL_setenv("SDL_AUDIODRIVER", "alsa", 1);
    // SDL_setenv("SDL_AUDIODRIVER", "pulseaudio", 1);

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
void AudioManager::playBackgroundMusic(const QString& resourcePath) {
    stopMusic(); // Dừng nhạc cũ

    // Dọn dẹp nhạc cũ
    if (bgMusic) {
        Mix_FreeMusic(bgMusic);
        bgMusic = nullptr;
    }
    
    // [QUAN TRỌNG] Xóa dữ liệu đệm cũ
    m_musicData.clear(); 

    // 1. Đọc file từ Resource Qt vào biến thành viên m_musicData
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open music resource:" << resourcePath;
        return;
    }
    m_musicData = file.readAll(); // Lưu vào RAM
    file.close();

    // 2. Tạo RWops từ dữ liệu trong RAM
    SDL_RWops* rw = SDL_RWFromConstMem(m_musicData.constData(), m_musicData.size());
    if (!rw) {
        qDebug() << "SDL_RWFromConstMem failed:" << SDL_GetError();
        return;
    }

    // 3. Load Music từ RWops (freesrc=1 để tự giải phóng rw)
    bgMusic = Mix_LoadMUS_RW(rw, 1);
    if (!bgMusic) {
        qDebug() << "Mix_LoadMUS_RW failed:" << Mix_GetError();
        return;
    }

    Mix_PlayMusic(bgMusic, -1);
    qDebug() << "Playing music from RAM:" << resourcePath;
}

void AudioManager::stopMusic() {
    Mix_HaltMusic();
}

// ===== CLICK SOUND =====
void AudioManager::loadClickSound(const QString& resourcePath)
{
    if (clickChunk) {
        Mix_FreeChunk(clickChunk);
        clickChunk = nullptr;
    }

    // 1. Đọc file từ Resource
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open click resource:" << resourcePath;
        return;
    }
    QByteArray data = file.readAll();
    file.close();

    // 2. Tạo RWops
    SDL_RWops* rw = SDL_RWFromConstMem(data.constData(), data.size());
    
    // 3. Load WAV
    // Lưu ý: Với Chunk (WAV), SDL sẽ copy dữ liệu vào bộ nhớ riêng của nó,
    // nên ta không cần giữ biến 'data' hay 'rw' sau khi hàm này kết thúc.
    clickChunk = Mix_LoadWAV_RW(rw, 1);

    if (!clickChunk) {
        qDebug() << "Load click failed:" << Mix_GetError();
        return;
    }

    // Set volume mặc định
    // (Giả sử bạn có biến sfxVolume, nếu không có thì bỏ dòng này)
    // Mix_VolumeChunk(clickChunk, (sfxVolume * MIX_MAX_VOLUME) / 100); 
    
    qDebug() << "Loaded click sound from RAM:" << resourcePath;
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
