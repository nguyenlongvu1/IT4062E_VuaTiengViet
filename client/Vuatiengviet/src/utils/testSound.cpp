// test_sound.cpp
#include <SDL2/SDL.h>
#include <stdio.h>

int main() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf("FAIL: %s\n", SDL_GetError());
    } else {
        printf("SUCCESS: Audio initialized!\n");
        printf("Driver: %s\n", SDL_GetCurrentAudioDriver());
        SDL_Quit();
    }
    return 0;
}