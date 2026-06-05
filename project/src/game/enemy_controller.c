#include "game/enemy_controller.h"
#include "game/entity_controller.h"
#include "view/assets_cache.h"
#include "models/board.h"
#include <stdlib.h>

bool enemy_can_move(t_game_state *game, enemy_t *enemy, direction_t dir) {
    if (game == NULL || enemy == NULL) return false;

    t_tuple next = enemy->board_pos;

    if (dir == DIR_LEFT) next.x--;
    else if (dir == DIR_RIGHT) next.x++;
    else if (dir == DIR_UP) next.y--;
    else next.y++;

    return !collision(game, enemy, next);
}

// enemis always try to move forward or turn at corners, but have a 5% chance of inverting direction (to avoid getting stuck in small loops)
void choose_enemy_direction(t_game_state *game, enemy_t *enemy) {

    if (game == NULL || enemy == NULL) return;

    direction_t opposite = opposite_dir(enemy->dir);

    // 5% de chance de inverter direção
    if ((rand() % 20) == 1) {
        enemy->dir = opposite;
        return;
    }

    direction_t valid_dirs[4];

    int count = 0;

    direction_t dirs[4] = {
        DIR_LEFT,
        DIR_RIGHT,
        DIR_UP,
        DIR_DOWN
    };

    for (int i = 0; i < 4; i++) {
        direction_t dir = dirs[i];

        if (dir == opposite) continue;

        if (enemy_can_move(game, enemy, dir)) {
            valid_dirs[count] = dir;
            count++;
        }
    }

    // no option but to go back
    if (count == 0) {
        if (enemy_can_move(game, enemy, opposite)) enemy->dir = opposite;
        return;
    }

    // choose random valid direction
    enemy->dir = valid_dirs[rand() % count];
}

static void enemy_on_snap(t_game_state *game, entity_t *enemy) {
    choose_enemy_direction(game, enemy);
    enemy->sprite_dir = enemy->dir;
}

void enemy_init(t_game_state *game, enemy_t *enemy, t_tuple spawnpoint) {
    if (!game || !enemy) return;

    enemy->pos.x = (spawnpoint.x * game->tile_size) + (game->tile_size / 2);
    enemy->pos.y = (spawnpoint.y * game->tile_size) + (game->tile_size / 2);

    uint32_t ew = (game->tile_size * 8) / 12;
    uint32_t eh = ew;

    if (sprites_initialized && sprite_cache[SPRITE_ENEMY_1_STANDING].bytes != NULL) {
        uint32_t img_w = sprite_cache[SPRITE_ENEMY_1_STANDING].width;
        uint32_t img_h = sprite_cache[SPRITE_ENEMY_1_STANDING].height;
        eh = (img_h * ew) / img_w;
    }

    enemy->size.x = ew;
    enemy->size.y = eh;

    enemy->board_pos = spawnpoint;
    enemy->active = true;
    enemy->is_moving = true;
    enemy->bomb_max = 0;
    enemy->bomb_available = 0;

    direction_t valid_dirs[4];
    int count = get_valid_directions(game, enemy->board_pos, valid_dirs);

    if (count > 0) {
        direction_t dir = valid_dirs[rand() % count];
        enemy->dir = dir;
        enemy->sprite_dir = dir;
    } else {
        enemy->dir = DIR_UP;
        enemy->sprite_dir = DIR_UP;
        enemy->is_moving = false;
    }

    enemy->animation_phase = 0;
    enemy->speed = ENEMY_SPEED;
    enemy->on_snap = enemy_on_snap;
    enemy->invincibility_timer = 0;
    enemy->lives = 1;
}

void update_enemy_lives(t_game_state *game, enemy_t *enemy, int change) {
    if (enemy == NULL || !enemy->active || enemy->lives == 0) return;

    if (change < 0) {
        if (enemy->invincibility_timer > 0) return;
        enemy->invincibility_timer = GAME_TICKS_PER_SECOND / 2;
    }

    int new_lives = (int)enemy->lives + change;
    if (new_lives < 0) new_lives = 0;
    enemy->lives = (uint8_t)new_lives;

    if (enemy->lives == 0) {
        enemy->invincibility_timer = GAME_TICKS_PER_SECOND; // 1 second blink before death
        game->score += 100;
    }
}

void update_enemy_movement(t_game_state *game, enemy_t *enemy) {
    if (!enemy || !enemy->active || !enemy->is_moving || enemy->lives == 0) return;
    update_entity_movement(game, enemy);
}

void update_enemy_animation(t_game_state *game, enemy_t *enemy, uint32_t logical_ticks) {
    if (enemy == NULL || !enemy->active) return;

    if (enemy->invincibility_timer > 0) {
        enemy->invincibility_timer--;
        
        if (enemy->lives == 0 && enemy->invincibility_timer == 0) {
            enemy->active = false;

            // power-up drop
            if (game != NULL) {
                int active_enemies = 0;
                for (int i = 0; i < MAX_ENEMIES; i++) {
                    if (game->enemies[i].active) active_enemies++;
                }

                // 20% + 5% per enemy, cap at 50%
                int drop_chance = 20 + (active_enemies * 5);
                if (drop_chance > 50) drop_chance = 50;
                drop_chance = 100;

                int r = rand() % 100;
                if (r < drop_chance / 2) {
                    game->board[enemy->board_pos.y * BOARD_COLS + enemy->board_pos.x] = TILE_TYPE_POWERUP_REACH;
                } else if (r < drop_chance) {
                    game->board[enemy->board_pos.y * BOARD_COLS + enemy->board_pos.x] = TILE_TYPE_POWERUP_COUNT;
                }
            }
        }
    }

    if (enemy->lives == 0) return; // Stop animation if dying

    if (logical_ticks % 30 == 0) {
        enemy->animation_phase = (enemy->animation_phase + 1) % 4;
    }

    if (!enemy->is_moving) return;
}
