#include "game/game.h"
#include "game/player_controller.h"
#include "game/entity_controller.h"
#include "vbe.h"
#include "core/macros.h"
#include "view/assets_cache.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define IN_BOUNDS(x, y) ((x) >= 0 && (x) < BOARD_COLS && (y) >= 0 && (y) < BOARD_ROWS)
#define BOMB_REACH(bomb, dir) (bomb->radius <  bomb->reach[dir]) ? bomb->radius :  bomb->reach[dir]

static const int32_t DIR_X[EXPLOSION_DIR_COUNT] = { 1, -1, 0, 0 };
static const int32_t DIR_Y[EXPLOSION_DIR_COUNT] = { 0, 0, 1, -1 };

void bomb_clear_explosion(bomb_t *bomb) {
    if (!bomb) return;
    bomb->explosion_timer = 0;
    bomb->radius = 0;
    memset(bomb->reach, 0, sizeof(bomb->reach));
}

uint8_t bomb_explosion_frame(const bomb_t *bomb) {
    if (!bomb || bomb->explosion_timer >= BOMB_EXPLOSION_DURATION_TICKS) return 0;

    const uint32_t max_frame = 5;
    const uint32_t total_steps = (2 * max_frame) + 1; // 11
    uint32_t age = BOMB_EXPLOSION_DURATION_TICKS - bomb->explosion_timer;

    uint32_t step = (age * total_steps) / BOMB_EXPLOSION_DURATION_TICKS;
    if (step >= total_steps) step = total_steps - 1;

    return (uint8_t)(step <= max_frame ? step : (2 * max_frame) - step);
}

static void bomb_compute_reach(t_game_state *game, bomb_t *bomb) {
    memset(bomb->reach, 0, sizeof(bomb->reach));

    uint8_t bonus_reach = 0;
    //powerup
    if (bomb->player_id < MAX_PLAYERS) {
        bonus_reach = GET_POWERUP_REACH(game->players[bomb->player_id].powerups);
    }

    for (uint8_t dir = 0; dir < EXPLOSION_DIR_COUNT; dir++) {
        for (uint8_t dist = 1; dist <= BOMB_EXPLOSION_RANGE + bonus_reach; dist++) {
            int32_t x = bomb->board_pos.x + (DIR_X[dir] * dist);
            int32_t y = bomb->board_pos.y + (DIR_Y[dir] * dist);

            if (!IN_BOUNDS(x, y) || game->board[y * BOARD_COLS + x] == TILE_TYPE_WALL) {
                bomb->reach[dir] = dist - 1;
                break;
            }

            bomb->reach[dir] = dist;
            if (game->board[y * BOARD_COLS + x] == TILE_TYPE_BRICK) break; // Stop at destructible block
        }
    }
}

static void damage_entities_at(t_game_state *game, int32_t cx, int32_t cy) {
    int32_t tile = (int32_t)game->tile_size;
    t_tuple exp_pos = { cx * tile + tile / 2, cy * tile + tile / 2 };
    t_tuple exp_size = { tile, tile };

    for (int i = 0; i < MAX_PLAYERS; i++) {
        player_t *p = &game->players[i];
        if (p->active && entity_overlaps(p->pos, p->size, exp_pos, exp_size))
            update_player_lives(p, -1);
    }
    for (int i = 0; i < game->enemy_count; i++) {
        enemy_t *e = &game->enemies[i];
        if (e->active && entity_overlaps(e->pos, e->size, exp_pos, exp_size))
            update_enemy_lives(game, e, -1);
    }
}

static void bomb_apply_explosion_contact(t_game_state *game, bomb_t *bomb, uint8_t radius) {

    // Damage at center
    damage_entities_at(game, bomb->board_pos.x, bomb->board_pos.y);

    for (uint8_t dir = 0; dir < EXPLOSION_DIR_COUNT; dir++) {
        uint8_t reach = BOMB_REACH(bomb, dir);
        for (uint8_t dist = 1; dist <= reach; dist++) {
            int32_t x = bomb->board_pos.x + (DIR_X[dir] * dist);
            int32_t y = bomb->board_pos.y + (DIR_Y[dir] * dist);

            damage_entities_at(game, x, y);

            if (dist == reach && radius >= reach && reach > 0) {
                if (IN_BOUNDS(x, y) && game->board[y * BOARD_COLS + x] == TILE_TYPE_BRICK) {
                    if (game->door_pos.x == x && game->door_pos.y == y) {
                        game->board[y * BOARD_COLS + x] = TILE_TYPE_DOOR;
                    } else {
                        game->board[y * BOARD_COLS + x] = TILE_TYPE_GRASS;
                    }
                }
            }
        }
    }
}

void bomb_begin_explosion(t_game_state *game, bomb_t *bomb) {
    if (!game || !bomb) return;
    bomb_clear_explosion(bomb);

    bomb->active = true;
    bomb->state = BOMB_FIRE;
    bomb->bomb_timer = 0;
    bomb->explosion_timer = BOMB_EXPLOSION_DURATION_TICKS;
    bomb_compute_reach(game, bomb);
}

void bomb_update_explosion(t_game_state *game, bomb_t *bomb) {
    if (!game || !bomb || !bomb->active || bomb->state != BOMB_FIRE) return;

    if (bomb->explosion_timer == 0) {
        bomb_reset(bomb);
        return;
    }

    bomb->explosion_timer--;
    bomb->radius = (uint8_t)(bomb_explosion_frame(bomb) / 2);
    bomb_apply_explosion_contact(game, bomb, bomb->radius);

    if (bomb->explosion_timer == 0) bomb_reset(bomb);
}

