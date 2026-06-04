#include "draw.h"
#include "vbe.h"
#include "assets_cache.h"
#include "game.h"
#include <stdint.h>
#include <lcom/xpm.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static int seed = 0;

void set_date_seed(int day, int month, int year) {
    seed = year * 10000 + month * 100 + day;
}

t_tuple spawnpoint_generator(uint8_t *board, uint32_t click_count) {
    const int inner_width = BOARD_COLS - 2;
    const int inner_height = BOARD_ROWS - 2;

    while (true) {
        click_count %= (inner_width * inner_height);
        int x = (click_count % inner_width) + 1;
        int y = (click_count / inner_width) + 1;
        int index = y * BOARD_COLS + x;
        if (board[index] == 0) {
            t_tuple result;
            result.x = x;
            result.y = y;
            return result;
        }
        click_count++;
    }
}

int draw_player(player_t *player, hw_video_t *video, t_game_state *game) {
    if (player == NULL || !sprites_initialized || !player->active) return 1;

    if (player->lives == 0) {
        xpm_image_t img = scaled_sprite_cache[SPRITE_PLAYER_DEATH];
        hw_vbe_draw_xpm(
            video,
            img.bytes,
            img,
            game->start_x + player->pos.x,
            game->start_y + player->pos.y
        );
        return 0;
    }

    if (game->match_state == MATCH_WON) {
        xpm_image_t img = scaled_sprite_cache[SPRITE_PLAYER_WIN];
        hw_vbe_draw_xpm(
            video,
            img.bytes,
            img,
            game->start_x + player->pos.x,
            game->start_y + player->pos.y
        );
        return 0;
    }

    // Blink if invincible
    if (player->invincibility_timer > 0 && (player->invincibility_timer / 5) % 2 == 0) {
        return 0;
    }

    int current_phase = player->animation_phase % 4;
    int current_direction = player->sprite_dir % 4;

    int sprite_index = SPRITE_PLAYER_1_STANDING + (current_phase * 4) + current_direction;

    if (sprite_index >= SPRITE_CACHE_SIZE || scaled_sprite_cache[sprite_index].bytes == NULL) {
        return 1;
    }

    xpm_image_t img = scaled_sprite_cache[sprite_index];

    int32_t draw_y = game->start_y + player->pos.y;
    
    // Sneak animation
    if (!player->is_moving && player->animation_phase == 1) {
        uint32_t sneak_amount = img.height / 20;
        draw_y += (int32_t)sneak_amount;
    }

    hw_vbe_draw_xpm(
        video,
        img.bytes,
        img,
        game->start_x + player->pos.x,
        draw_y
    );
    return 0;
}

void update_player_death_animation(t_game_state *game, player_t *player) {
    if (player == NULL || !player->active) return;

    if (player->pos.y == 672) {
        player->active = false;
        game->animation_timer = 0;
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

    if (game->animation_timer == 0) {
        player->active = false;
        return;
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
        player->bomb_available = (player->bomb_max > active_counts[i])
            ? (player->bomb_max - active_counts[i])
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
        player->final_pos = player->pos;
    }
}
