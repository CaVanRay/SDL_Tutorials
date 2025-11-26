
#include <SDL3/SDL.h>

int main() {

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed!");
        return -1;
    }

    SDL_Window* window;
    window = SDL_CreateWindow("Cavan - SDL3", 320, 240, SDL_WINDOW_RESIZABLE);

    SDL_Delay(5000);

    SDL_Quit();

    return 0;
}