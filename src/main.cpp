#include <SDL.h>
#include <cstdio>
#include <cstring>
#include "game.h"
#include "drawing.h"

int main(int argc, char* argv[]) {
    bool smokeTest = argc == 2 && std::strcmp(argv[1], "--smoke-test") == 0;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }
    Uint32 flags = smokeTest ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN;
    SDL_Window* window = SDL_CreateWindow("Little Platformer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 960, 640, flags);
    if (!window) {
        std::fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        std::fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_RenderSetLogicalSize(renderer, 960, 640);
    Game game = {};
    game.screen = MENU;
    bool running = true;
    bool jumpRequested = false;
    bool focused = true;
    Uint64 previousTime = SDL_GetPerformanceCounter();
    double frequency = double(SDL_GetPerformanceFrequency());
    double accumulator = 0;
    int frames = 0;

    while (running) {
        Uint64 now = SDL_GetPerformanceCounter();
        double elapsed = double(now - previousTime) / frequency;
        previousTime = now;
        // После перетаскивания окна не пытаемся догнать несколько секунд игры.
        if (elapsed > 0.1) elapsed = 0.1;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                    focused = false;
                    jumpRequested = false;
                    if (game.screen == PLAYING) game.screen = PAUSED;
                }
                if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) focused = true;
            }
            if (event.type != SDL_KEYDOWN || event.key.repeat || !focused) continue;
            SDL_Keycode key = event.key.keysym.sym;
            if (key == SDLK_ESCAPE) running = false;
            if (key == SDLK_RETURN && (game.screen == MENU ||
                game.screen == GAME_OVER || game.screen == VICTORY)) {
                startGame(game);
                accumulator = 0;
                jumpRequested = false;
            }
            if (key == SDLK_p) {
                if (game.screen == PLAYING) game.screen = PAUSED;
                else if (game.screen == PAUSED) game.screen = PLAYING;
                jumpRequested = false;
            }
            if (key == SDLK_r && (game.screen == PLAYING || game.screen == PAUSED)) {
                startGame(game);
                jumpRequested = false;
                accumulator = 0;
            }
            if ((key == SDLK_SPACE || key == SDLK_w || key == SDLK_UP) &&
                game.screen == PLAYING) jumpRequested = true;
        }
        const Uint8* keys = SDL_GetKeyboardState(NULL);
        int direction = 0;
        if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]) --direction;
        if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) ++direction;
        if (game.screen == PLAYING) {
            accumulator += elapsed;
            while (accumulator >= STEP) {
                updateGame(game, direction, jumpRequested);
                jumpRequested = false;
                accumulator -= STEP;
            }
        } else {
            accumulator = 0;
            jumpRequested = false;
        }
        drawGame(renderer, game);
        SDL_Delay(1);
        if (smokeTest) {
            ++frames;
            // Проверяем создание окна и отрисовку каждого экрана.
            if (frames == 1) startGame(game);
            if (frames == 2) game.screen = PAUSED;
            if (frames == 3) game.screen = GAME_OVER;
            if (frames == 4) game.screen = VICTORY;
            if (frames == 5) running = false;
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
