#include "drawing.h"
#include <cstdio>

static void rectangle(SDL_Renderer* renderer, int x, int y, int w, int h,
                      int red, int green, int blue) {
    SDL_SetRenderDrawColor(renderer, Uint8(red), Uint8(green), Uint8(blue), 255);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(renderer, &rect);
}

// Шрифт 5x7: каждый элемент массива описывает одну строку буквы.
// Он позволяет рисовать надписи без файлов шрифтов и дополнительных библиотек.
static const unsigned char FONT[36][7] = {
    {14,17,17,31,17,17,17}, {30,17,17,30,17,17,30},
    {14,17,16,16,16,17,14}, {30,17,17,17,17,17,30},
    {31,16,16,30,16,16,31}, {31,16,16,30,16,16,16},
    {14,17,16,23,17,17,14}, {17,17,17,31,17,17,17},
    {14,4,4,4,4,4,14}, {7,2,2,2,18,18,12},
    {17,18,20,24,20,18,17}, {16,16,16,16,16,16,31},
    {17,27,21,21,17,17,17}, {17,25,21,19,17,17,17},
    {14,17,17,17,17,17,14}, {30,17,17,30,16,16,16},
    {14,17,17,17,21,18,13}, {30,17,17,30,20,18,17},
    {15,16,16,14,1,1,30}, {31,4,4,4,4,4,4},
    {17,17,17,17,17,17,14}, {17,17,17,17,17,10,4},
    {17,17,17,21,21,21,10}, {17,17,10,4,10,17,17},
    {17,17,10,4,4,4,4}, {31,1,2,4,8,16,31},
    {14,17,19,21,25,17,14}, {4,12,4,4,4,4,14},
    {14,17,1,2,4,8,31}, {30,1,1,14,1,1,30},
    {2,6,10,18,31,2,2}, {31,16,16,30,1,1,30},
    {14,16,16,30,17,17,14}, {31,1,2,4,8,8,8},
    {14,17,17,14,17,17,14}, {14,17,17,15,1,1,14}
};

static void text(SDL_Renderer* renderer, const char* message, int x, int y, int size) {
    for (int i = 0; message[i] != '\0'; ++i) {
        char letter = message[i];
        int index = -1;
        if (letter >= 'A' && letter <= 'Z') index = letter - 'A';
        if (letter >= '0' && letter <= '9') index = letter - '0' + 26;
        if (index >= 0) {
            for (int row = 0; row < 7; ++row)
                for (int column = 0; column < 5; ++column)
                    if (FONT[index][row] & (1 << (4 - column)))
                        rectangle(renderer, x + column * size, y + row * size,
                                  size, size, 240, 245, 255);
        }
        x += 6 * size;
    }
}

static void centeredText(SDL_Renderer* renderer, const char* message, int y, int size) {
    int length = 0;
    while (message[length] != '\0') ++length;
    text(renderer, message, (960 - length * 6 * size + size) / 2, y, size);
}

static void background(SDL_Renderer* renderer, float camera) {
    for (int y = 0; y < 640; y += 8)
        rectangle(renderer, 0, y, 960, 8, 25 + y / 40, 44 + y / 24, 78 + y / 18);
    rectangle(renderer, 790, 100, 56, 56, 255, 216, 130);
    for (int i = 0; i < 10; ++i) {
        int x = i * 230 - int(camera * 0.2f) % 230;
        rectangle(renderer, x, 395, 180, 245, 43, 75, 100);
        rectangle(renderer, x + 24, 365, 130, 40, 43, 75, 100);
        rectangle(renderer, x + 47, 337, 84, 30, 43, 75, 100);
    }
}

void drawGame(SDL_Renderer* renderer, const Game& game) {
    background(renderer, game.camera);
    if (game.screen == MENU) {
        centeredText(renderer, "LITTLE PLATFORMER", 130, 6);
        centeredText(renderer, "THREE LEVELS AND ONE ADVENTURE", 215, 3);
        centeredText(renderer, "ENTER TO START", 310, 4);
        centeredText(renderer, "A D OR ARROWS TO MOVE", 405, 3);
        centeredText(renderer, "SPACE OR W OR UP TO JUMP", 450, 3);
        centeredText(renderer, "P TO PAUSE   R TO RESTART   ESC TO QUIT", 525, 2);
        SDL_RenderPresent(renderer);
        return;
    }
    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            int screenX = x * TILE - int(game.camera);
            if (screenX < -TILE || screenX > 960) continue;
            int screenY = y * TILE;
            if (game.map[y][x] == '#') {
                rectangle(renderer, screenX, screenY, TILE, TILE, 111, 79, 65);
                rectangle(renderer, screenX + 2, screenY + 8, TILE - 4, TILE - 10, 139, 99, 76);
                if (y == 0 || game.map[y - 1][x] != '#') {
                    rectangle(renderer, screenX, screenY, TILE, 8, 106, 190, 113);
                    rectangle(renderer, screenX, screenY + 8, TILE, 3, 67, 133, 91);
                }
            } else if (game.map[y][x] == 'o') {
                rectangle(renderer, screenX + 13, screenY + 9, 14, 22, 255, 185, 55);
                rectangle(renderer, screenX + 9, screenY + 13, 22, 14, 255, 204, 75);
                rectangle(renderer, screenX + 14, screenY + 13, 4, 12, 255, 238, 165);
            } else if (game.map[y][x] == 'F') {
                rectangle(renderer, screenX + 10, screenY - 40, 4, 80, 235, 242, 248);
                rectangle(renderer, screenX + 14, screenY - 40, 26, 24, 111, 225, 164);
            }
        }
    }
    for (int i = 0; i < game.enemyCount; ++i) {
        const Enemy& enemy = game.enemies[i];
        if (!enemy.alive) continue;
        int x = int(enemy.body.x - game.camera);
        int y = int(enemy.body.y);
        rectangle(renderer, x, y + 5, 32, 23, 224, 98, 105);
        rectangle(renderer, x + 4, y, 24, 6, 239, 132, 125);
        rectangle(renderer, x + 6, y + 9, 5, 6, 38, 40, 60);
        rectangle(renderer, x + 21, y + 9, 5, 6, 38, 40, 60);
    }
    int px = int(game.player.body.x - game.camera);
    int py = int(game.player.body.y);
    rectangle(renderer, px, py + 10, 28, 24, 83, 181, 239);
    rectangle(renderer, px + 3, py, 22, 15, 255, 218, 172);
    rectangle(renderer, px + 3, py, 22, 5, 49, 72, 106);
    rectangle(renderer, px + 16, py + 7, 4, 4, 38, 40, 60);
    rectangle(renderer, px + 2, py + 29, 9, 5, 49, 72, 106);
    rectangle(renderer, px + 17, py + 29, 9, 5, 49, 72, 106);

    rectangle(renderer, 0, 0, 960, 56, 22, 34, 58);
    char hud[80];
    int shownLevel = game.level < LEVEL_COUNT ? game.level + 1 : LEVEL_COUNT;
    std::snprintf(hud, sizeof(hud), "LEVEL %d OF 3   LIVES %d   COINS %d",
                  shownLevel, game.lives, game.coins);
    text(renderer, hud, 24, 18, 3);
    text(renderer, "P PAUSE", 798, 22, 2);

    if (game.screen != PLAYING) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 12, 20, 35, 215);
        SDL_Rect panel = {70, 180, 820, 280};
        SDL_RenderFillRect(renderer, &panel);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        if (game.screen == PAUSED) {
            centeredText(renderer, "PAUSED", 235, 6);
            centeredText(renderer, "P TO CONTINUE", 335, 3);
        } else if (game.screen == GAME_OVER) {
            centeredText(renderer, "GAME OVER", 235, 6);
            centeredText(renderer, "ENTER TO TRY AGAIN", 335, 3);
        } else if (game.screen == VICTORY) {
            centeredText(renderer, "YOU WIN", 220, 6);
            std::snprintf(hud, sizeof(hud), "COINS COLLECTED %d", game.coins);
            centeredText(renderer, hud, 300, 3);
            centeredText(renderer, "ENTER TO PLAY AGAIN", 365, 3);
        }
    }
    SDL_RenderPresent(renderer);
}
