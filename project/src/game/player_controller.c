#include "game/player_controller.h"
#include "game/entity_controller.h"
#include "core/macros.h"
#include "view/assets_cache.h"
#include "i8042.h"
#include <string.h>

t_tuple spawnpoint_generator(uint8_t *board, uint32_t click_count) {
    const int inner_width = BOARD_COLS - 2;
    const int inner_height = BOARD_ROWS - 2;
    const int inner_size = inner_width * inner_height;

    click_count %= (inner_size * 2);

    int middle = inner_size / 2;

    int direction;
    int offset;

    if (click_count == 0) {
        direction = 1;
        offset = 0;
    } else if (click_count % 2 == 0) {
        direction = 1;
        offset = click_count / 2;
    } else {
        direction = -1;
        offset = (click_count + 1) / 2;
    }

    for (int i = 0; i < inner_size; i++) {
        int inner_index = middle + direction * (offset + i);

        while (inner_index < 0) {
            inner_index += inner_size;
        }

        inner_index %= inner_size;

        int x = (inner_index % inner_width) + 1;
        int y = (inner_index / inner_width) + 1;
        int index = y * BOARD_COLS + x;

        if (board[index] == 0) {
            t_tuple result;
            result.x = x;
            result.y = y;
            return result;
        }
    }

    t_tuple fallback;
    fallback.x = 1;
    fallback.y = 1;
    return fallback;
}


void update_player_death_animation(t_game_state *game, player_t *player) {
    if (player == NULL || !player->active) return;

    if (player->pos.y == 672) {
        player->active = false;
        if (!game->players[PLAYER_1].active && !game->players[PLAYER_2].active) game->animation_timer = 0;
        else game->animation_timer--;
        return;
    }

    if (game->animation_timer > GAME_TICKS_PER_SECOND * 4.5) {
        game->animation_timer--;
        return;
    }

    if (game->animation_timer > GAME_TICKS_PER_SECOND * 4.2) {
        player->pos.y -= 4;
    }

    else if (game->animation_timer > 0) {
        player->pos.y += 4;
    }

    game->animation_timer--;
}

void update_player_win_animation(t_game_state *game, player_t *player) {
    if (player == NULL || !player->active) return;

    if (game->animation_timer == GAME_TICKS_PER_SECOND * 3) {
        player->pos.x = player->board_pos.x * game->tile_size + game->tile_size / 2;
        player->pos.y = player->board_pos.y * game->tile_size + game->tile_size / 2;

        player->sprite_dir = DIR_DOWN;
        player->animation_phase = 0;

        game->animation_timer--;
        return;
    }

    if (game->animation_timer < GAME_TICKS_PER_SECOND * 2.2 && game->animation_timer > GAME_TICKS_PER_SECOND * 2) {
        player->pos.y -= 1;
        game->animation_timer--;
        return;
    }

    if (game->animation_timer < GAME_TICKS_PER_SECOND * 2 && game->animation_timer > GAME_TICKS_PER_SECOND * 1.8) {
        player->pos.y += 1;
        game->animation_timer--;
        return;
    }

    if (game->animation_timer < GAME_TICKS_PER_SECOND * 1 && game->animation_timer > GAME_TICKS_PER_SECOND * 0.8) {
        player->pos.y -= 1;
        game->animation_timer--;
        return;
    }

    if (game->animation_timer < GAME_TICKS_PER_SECOND * 0.8 && game->animation_timer > GAME_TICKS_PER_SECOND * 0.6) {
        player->pos.y += 1;
        game->animation_timer--;
        return;
    }

    if (game->animation_timer == 1) {
        player->active = false;
    }
    game->animation_timer--;
}

static void remove_movement_key(player_t *player, uint8_t key_index) {
    for (int i = 0; i < player->stack_count; i++) {
        if (player->movement_stack[i] == key_index) {
            size_t items_to_move = player->stack_count - 1 - i;

            if (items_to_move > 0) {
                memmove(&player->movement_stack[i],
                        &player->movement_stack[i + 1],
                        items_to_move * sizeof(uint8_t));
            }
            player->stack_count--;
            return;
        }
    }
}

static void push_movement_key(player_t *player, uint8_t key_index) {
    remove_movement_key(player, key_index);

    if (player->stack_count < 4) {
        player->movement_stack[player->stack_count] = key_index;
        player->stack_count++;
    }
}

void update_player_direction(player_t *player, uint8_t key, bool is_make) {
    if (key != KEY_W && key != KEY_A && key != KEY_D && key != KEY_S) return;

    is_make ? push_movement_key(player, key) : remove_movement_key(player, key);

    if (player->stack_count > 0 && !player->is_moving) {
        uint8_t top_key = player->movement_stack[player->stack_count - 1];

        player->dir = (top_key == KEY_W) ? DIR_UP :
                  (top_key == KEY_A) ? DIR_LEFT :
                  (top_key == KEY_D) ? DIR_RIGHT : DIR_DOWN;

        player->sprite_dir = player->dir; // Visual update since we start moving
        player->is_moving = true;
    }
}

static void player_on_snap(t_game_state *game, entity_t *player) {
    (void)game; // Unused
    if (player->stack_count == 0) {
        player->is_moving = false;
    } else {
        uint8_t top_key = player->movement_stack[player->stack_count - 1];

        player->dir = (top_key == KEY_W) ? DIR_UP :
                      (top_key == KEY_A) ? DIR_LEFT :
                      (top_key == KEY_D) ? DIR_RIGHT : DIR_DOWN;

        player->sprite_dir = player->dir;
    }
}

void player_init(t_game_state *game, player_t *player, t_tuple spawnpoint) {
    if (game == NULL || player == NULL) return;

    player->pos = (t_tuple) {
        (spawnpoint.x * game->tile_size) + (game->tile_size / 2),
        (spawnpoint.y * game->tile_size) + (game->tile_size / 2)
    };

    uint32_t pw = (game->tile_size * 8) / 12;
    uint32_t ph = pw;

    if (sprites_initialized && sprite_cache[SPRITE_PLAYER_1_STANDING].bytes != NULL) {
        uint32_t img_w = sprite_cache[SPRITE_PLAYER_1_STANDING].width;
        uint32_t img_h = sprite_cache[SPRITE_PLAYER_1_STANDING].height;
        ph = (img_h * pw) / img_w;
    }

    player->size.x = pw;
    player->size.y = ph;

    player->board_pos = spawnpoint;
    player->sprite_dir = DIR_DOWN;
    player->dir = DIR_DOWN;
    player->animation_phase = 0;
    player->is_moving = false;
    player->stack_count = 0;
    player->speed = 4;
    player->bomb_max = 1;
    player->bomb_available = 1;
    player->on_snap = player_on_snap;
    player->invincibility_timer = 0;
    player->powerups = 0;
    player->active = true;
}

void update_player_movement(t_game_state *game, player_t *player) {
    if (!player || !player->is_moving) return;

    //direction cancelling
    if (player->stack_count > 0) {
        uint8_t top_key = player->movement_stack[player->stack_count - 1];
        int next_dir = (top_key == KEY_W) ? DIR_UP :
                       (top_key == KEY_A) ? DIR_LEFT :
                       (top_key == KEY_D) ? DIR_RIGHT : DIR_DOWN;

        if ((player->dir == DIR_LEFT && next_dir == DIR_RIGHT) ||
            (player->dir == DIR_RIGHT && next_dir == DIR_LEFT) ||
            (player->dir == DIR_DOWN && next_dir == DIR_UP) ||
            (player->dir == DIR_UP && next_dir == DIR_DOWN)) {

            player->dir = next_dir;
            player->sprite_dir = next_dir;
        }
    }

    update_entity_movement(game, player);
}

void update_player_animation(player_t *player, uint32_t logical_ticks) {
    if (player == NULL || !player->active) return;

    if (player->invincibility_timer > 0) {
        player->invincibility_timer--;
    }

    if (!player->is_moving) {
        if (player->animation_phase > 1)
            player->animation_phase = 0;

        if (logical_ticks % 30 == 0) {
            if (player->animation_phase == 0)
                player->animation_phase = 1;
            else
                player->animation_phase = 0;
        }
    } else {
        if (player->animation_phase < 2)
            player->animation_phase = 2;

        if (logical_ticks % 15 == 0) {
            if (player->animation_phase == 2)
                player->animation_phase = 3;
            else
                player->animation_phase = 2;
        }
    }
}

void player_bomb_count(t_game_state *game) {
    if (game == NULL) return;

    uint8_t active_counts[MAX_PLAYERS] = {0};
    for (int i = 0; i < MAX_BOMBS; i++) {
        const bomb_t *bomb = &game->bomb[i];
        if (!bomb->active) continue;
        if (bomb->player_id < MAX_PLAYERS) {
            active_counts[bomb->player_id]++;
        }
    }

    for (int i = 0; i < MAX_PLAYERS; i++) {
        player_t *player = &game->players[i];
        if (!player->active) {
            player->bomb_available = 0;
            continue;
        }

        uint8_t bonus_bombs = GET_POWERUP_COUNT(player->powerups);
        uint8_t effective_max = player->bomb_max + bonus_bombs;

        player->bomb_available = (effective_max > active_counts[i])
            ? (effective_max - active_counts[i])
            : 0;
    }
}

void update_player_lives(player_t *player, int change) {
    if (player == NULL || !player->active) return;

    if (change < 0) {
        if (player->invincibility_timer > 0) return;
        player->invincibility_timer = GAME_TICKS_PER_SECOND * 2; // 2 seconds of invincibility
    }

    int new_lives = (int)player->lives + change;
    if (new_lives < 0) new_lives = 0;
    player->lives = (uint8_t)new_lives;

    if (player->lives == 0) {

    }
}
