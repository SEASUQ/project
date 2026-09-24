#ifndef GAME_H
#define GAME_H

const int TILE = 40;
const int MAP_WIDTH = 64;
const int MAP_HEIGHT = 16;
const int LEVEL_COUNT = 3;
const int MAX_ENEMIES = 6;
const float STEP = 1.0f / 120.0f;

struct Body {
    float x, y, width, height;
};

struct Player {
    Body body;
    float speedY;
    bool onGround;
};

struct Enemy {
    Body body;
    float left, right, speed;
    bool alive;
};

enum Screen { MENU, PLAYING, PAUSED, GAME_OVER, VICTORY };

struct Game {
    char map[MAP_HEIGHT][MAP_WIDTH];
    Player player;
    Enemy enemies[MAX_ENEMIES];
    int enemyCount;
    int level;
    int lives;
    int coins;
    int levelStartCoins;
    float camera;
    Screen screen;
};

bool overlaps(Body a, Body b);
bool solidAt(const Game& game, int column, int row);
void loadLevel(Game& game);
void startGame(Game& game);
void updateGame(Game& game, int direction, bool jump);

#endif
