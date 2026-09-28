#include "game.h"

bool overlaps(Body a, Body b) {
    return a.x < b.x + b.width && a.x + a.width > b.x &&
           a.y < b.y + b.height && a.y + a.height > b.y;
}

bool solidAt(const Game& game, int column, int row) {
    if (column < 0 || column >= MAP_WIDTH) return true;
    if (row < 0 || row >= MAP_HEIGHT) return false;
    return game.map[row][column] == '#';
}

static void platform(Game& game, int first, int last, int row) {
    for (int x = first; x <= last; ++x) game.map[row][x] = '#';
}

static void pit(Game& game, int first, int last) {
    for (int x = first; x <= last; ++x) {
        game.map[14][x] = '.';
        game.map[15][x] = '.';
    }
}

static void coinLine(Game& game, int first, int last, int row) {
    for (int x = first; x <= last; x += 2) game.map[row][x] = 'o';
}

static void addEnemy(Game& game, int first, int last) {
    if (game.enemyCount >= MAX_ENEMIES) return;
    Enemy& enemy = game.enemies[game.enemyCount++];
    enemy.body = {first * float(TILE), 14 * float(TILE) - 28, 32, 28};
    enemy.left = first * float(TILE);
    enemy.right = (last + 1) * float(TILE) - enemy.body.width;
    enemy.speed = 70 + game.level * 15.0f;
    enemy.alive = true;
}

void loadLevel(Game& game) {
    for (int y = 0; y < MAP_HEIGHT; ++y)
        for (int x = 0; x < MAP_WIDTH; ++x)
            game.map[y][x] = y >= 14 ? '#' : '.';
    game.enemyCount = 0;
    game.player.body = {60, 14 * float(TILE) - 34, 28, 34};
    game.player.speedY = 0;
    game.player.onGround = true;
    game.camera = 0;
    game.levelStartCoins = game.coins;

    // Номера клеток задают простой уровень без загрузки внешних файлов.
    platform(game, 5, 8, 12);
    platform(game, 10, 13, 10);
    platform(game, 20, 23, 12);
    platform(game, 26, 29, 10);
    platform(game, 40, 43, 12);
    platform(game, 46, 49, 10);
    coinLine(game, 5, 8, 11);
    coinLine(game, 10, 13, 9);
    coinLine(game, 20, 23, 11);
    coinLine(game, 26, 29, 9);
    coinLine(game, 40, 43, 11);
    coinLine(game, 46, 49, 9);
    coinLine(game, 3, 61, 13);
    pit(game, 16, 18);
    pit(game, 34, 36);
    addEnemy(game, 9, 14);
    addEnemy(game, 24, 31);
    addEnemy(game, 44, 51);

    if (game.level >= 1) {
        pit(game, 54, 56);
        platform(game, 57, 59, 12);
        coinLine(game, 57, 59, 11);
        addEnemy(game, 37, 39);
    }
    if (game.level == 2) {
        pit(game, 30, 32);
        game.map[10][28] = '.';
        game.map[10][29] = '.';
        game.map[9][28] = '.';
        // Убираем врага, чей маршрут пересекает новую яму.
        game.enemies[1].right = 30 * float(TILE) - 32;
        addEnemy(game, 58, 61);
    }
    // Монеты внутри ям не нужны: собрать их без падения нельзя.
    for (int x = 0; x < MAP_WIDTH; ++x)
        if (game.map[14][x] != '#') game.map[13][x] = '.';
    game.map[13][62] = 'F';
}

void startGame(Game& game) {
    game.level = 0;
    game.lives = 3;
    game.coins = 0;
    game.screen = PLAYING;
    loadLevel(game);
}

static void loseLife(Game& game) {
    --game.lives;
    if (game.lives <= 0) {
        game.screen = GAME_OVER;
    } else {
        // При повторе уровня монеты тоже возвращаются к исходному состоянию.
        game.coins = game.levelStartCoins;
        loadLevel(game);
    }
}

static void moveHorizontal(Game& game, float distance) {
    Body& body = game.player.body;
    body.x += distance;
    int left = int(body.x / TILE);
    int right = int((body.x + body.width - 0.01f) / TILE);
    int top = int(body.y / TILE);
    int bottom = int((body.y + body.height - 0.01f) / TILE);
    for (int y = top; y <= bottom; ++y) {
        for (int x = left; x <= right; ++x) {
            if (!solidAt(game, x, y)) continue;
            if (distance > 0) body.x = x * float(TILE) - body.width;
            if (distance < 0) body.x = (x + 1) * float(TILE);
        }
    }
    if (body.x < 0) body.x = 0;
    if (body.x > MAP_WIDTH * TILE - body.width)
        body.x = MAP_WIDTH * TILE - body.width;
}

static void moveVertical(Game& game) {
    Player& player = game.player;
    Body& body = player.body;
    body.y += player.speedY * STEP;
    player.onGround = false;
    int left = int(body.x / TILE);
    int right = int((body.x + body.width - 0.01f) / TILE);
    int top = int(body.y / TILE);
    int bottom = int((body.y + body.height - 0.01f) / TILE);
    for (int y = top; y <= bottom; ++y) {
        for (int x = left; x <= right; ++x) {
            if (!solidAt(game, x, y)) continue;
            if (player.speedY > 0) {
                body.y = y * float(TILE) - body.height;
                player.onGround = true;
            } else if (player.speedY < 0) {
                body.y = (y + 1) * float(TILE);
            }
            player.speedY = 0;
        }
    }
}

void updateGame(Game& game, int direction, bool jump) {
    if (game.screen != PLAYING) return;
    if (direction < -1) direction = -1;
    if (direction > 1) direction = 1;
    Player& player = game.player;
    float previousBottom = player.body.y + player.body.height;
    if (jump && player.onGround) {
        player.speedY = -620;
        player.onGround = false;
    }
    moveHorizontal(game, direction * 280.0f * STEP);
    player.speedY += 1600.0f * STEP;
    moveVertical(game);
    if (player.body.y > MAP_HEIGHT * TILE + 80) {
        loseLife(game);
        return;
    }

    for (int i = 0; i < game.enemyCount; ++i) {
        Enemy& enemy = game.enemies[i];
        if (!enemy.alive) continue;
        enemy.body.x += enemy.speed * STEP;
        if (enemy.body.x < enemy.left) {
            enemy.body.x = enemy.left;
            enemy.speed = -enemy.speed;
        }
        if (enemy.body.x > enemy.right) {
            enemy.body.x = enemy.right;
            enemy.speed = -enemy.speed;
        }
        if (!overlaps(player.body, enemy.body)) continue;
        if (player.speedY > 0 && previousBottom <= enemy.body.y + 4) {
            enemy.alive = false;
            player.body.y = enemy.body.y - player.body.height;
            player.speedY = -380;
            player.onGround = false;
        } else {
            loseLife(game);
            return;
        }
    }

    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            char cell = game.map[y][x];
            if (cell != 'o' && cell != 'F') continue;
            Body item = {x * float(TILE) + 8, y * float(TILE) + 8, 24, 32};
            if (!overlaps(player.body, item)) continue;
            if (cell == 'o') {
                game.map[y][x] = '.';
                ++game.coins;
            } else {
                ++game.level;
                if (game.level >= LEVEL_COUNT) game.screen = VICTORY;
                else loadLevel(game);
                return;
            }
        }
    }
    game.camera = player.body.x - 960 / 3.0f;
    if (game.camera < 0) game.camera = 0;
    if (game.camera > MAP_WIDTH * TILE - 960)
        game.camera = MAP_WIDTH * TILE - 960;
}
