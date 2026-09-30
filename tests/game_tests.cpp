#include "game.h"
#include <cstdio>

static int failures = 0;

static void check(bool condition, const char* message) {
    if (!condition) {
        std::fprintf(stderr, "FAILED: %s\n", message);
        ++failures;
    }
}

static void tick(Game& game, int count, int direction = 0) {
    for (int i = 0; i < count; ++i) updateGame(game, direction, false);
}

static void disableEnemies(Game& game) {
    for (int i = 0; i < game.enemyCount; ++i) game.enemies[i].alive = false;
}

int main() {
    Game game = {};
    startGame(game);
    check(game.lives == 3 && game.level == 0 && game.screen == PLAYING,
          "new game starts with three lives");
    tick(game, 120);
    check(game.player.onGround && game.player.body.y == 526,
          "player rests on floor");
    updateGame(game, 0, true);
    check(game.player.speedY < 0 && !game.player.onGround, "jump leaves floor");
    float speed = game.player.speedY;
    updateGame(game, 0, true);
    check(game.player.speedY > speed, "airborne player cannot jump again");
    tick(game, 120);
    check(game.player.onGround && game.player.body.y == 526, "jump lands on floor");

    game.screen = PAUSED;
    float oldX = game.player.body.x;
    tick(game, 100, 1);
    check(game.player.body.x == oldX, "paused game does not update");
    game.screen = PLAYING;
    tick(game, 120, -1);
    check(game.player.body.x == 0, "player cannot leave left boundary");

    startGame(game);
    game.player.body = {170, 486, 28, 34};
    game.player.onGround = false;
    tick(game, 20, 1);
    check(game.player.body.x <= 172, "platform side blocks horizontal motion");

    startGame(game);
    game.player.body = {220, 440, 28, 34};
    game.player.speedY = 100;
    game.player.onGround = false;
    tick(game, 30);
    check(game.player.onGround && game.player.body.y == 446,
          "falling player lands on raised platform");

    startGame(game);
    game.player.body = {220, 526, 28, 34};
    updateGame(game, 0, true);
    tick(game, 12);
    check(game.player.body.y >= 520 && game.player.speedY >= 0,
          "platform underside stops upward movement");

    startGame(game);
    game.player.body.x = 125;
    tick(game, 1);
    check(game.coins == 1 && game.map[13][3] == '.', "coin collected once");
    tick(game, 10);
    check(game.coins == 1, "collected coin does not repeat");
    game.player.body.y = 750;
    tick(game, 1);
    check(game.lives == 2 && game.coins == 0 && game.map[13][3] == 'o',
          "fall costs a life and resets current level coins");
    game.lives = 1;
    game.player.body.y = 750;
    tick(game, 1);
    check(game.screen == GAME_OVER && game.lives == 0, "last life ends game");

    startGame(game);
    game.player.body.x = game.enemies[0].body.x;
    tick(game, 1);
    check(game.lives == 2, "side collision with enemy costs a life");
    startGame(game);
    game.player.body.x = game.enemies[0].body.x;
    game.player.body.y = game.enemies[0].body.y - game.player.body.height - 1;
    game.player.speedY = 200;
    game.player.onGround = false;
    tick(game, 1);
    check(!game.enemies[0].alive && game.player.speedY < 0 && game.lives == 3,
          "landing on enemy defeats it and bounces player");

    startGame(game);
    for (int level = 0; level < LEVEL_COUNT; ++level) {
        game.player.body.x = 62 * TILE;
        tick(game, 1);
        check(game.level == level + 1, "flag advances level");
    }
    check(game.screen == VICTORY, "last flag finishes game");

    // Проходим все ямы реальными шагами физики. Врагов проверяем отдельно выше.
    for (int level = 0; level < LEVEL_COUNT; ++level) {
        startGame(game);
        game.level = level;
        loadLevel(game);
        disableEnemies(game);
        for (int frame = 0; frame < 2400 && game.level == level; ++frame) {
            bool jump = false;
            int column = int((game.player.body.x + game.player.body.width) / TILE);
            if (game.player.onGround) {
                for (int x = column; x <= column + 2 && x < MAP_WIDTH; ++x) {
                    float distance = x * float(TILE) -
                                     (game.player.body.x + game.player.body.width);
                    if (!solidAt(game, x, 14) && distance <= 60) jump = true;
                }
            }
            updateGame(game, 1, jump);
            if (game.lives < 3) break;
        }
        check(game.lives == 3 && game.level == level + 1,
              "all level pits are crossable with normal jump");
    }

    if (failures == 0) std::puts("All game checks passed.");
    return failures == 0 ? 0 : 1;
}
