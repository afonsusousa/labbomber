#include "draw.h"
#include "vbe.h"
#include "assets_cache.h"
#include "game.h"
#include <stdint.h>
#include <lcom/xpm.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

t_tuple entering_cell(const t_game_state *game, direction_t dir, int32_t px, int32_t py, uint32_t w, uint32_t h) {
    if (game == NULL) return (t_tuple){0,0};

    int dx = (dir == DIR_RIGHT) - (dir == DIR_LEFT);
    int dy = (dir == DIR_DOWN) - (dir == DIR_UP);

    /* Compute effective board cell directly (avoid calling aux functions) */
    int tile = game->tile_size;
    int half = tile / 2;
    t_tuple result = (t_tuple){ (px - half) / tile, (py - half) / tile };
    int half_w = (int)w / 2;
    int half_h = (int)h / 2;

    if (dx > 0)
        result.x = (px + half_w) / tile;
    else if (dx < 0)
        result.x = (px - half_w - 1) / tile;

    if (dy > 0)
        result.y = (py + half_h) / tile;
    else if (dy < 0)
        result.y = (py - half_h - 1) / tile;

    return result;
}

t_tuple continuous_board_pos(const t_game_state *game, direction_t dir, int32_t px, int32_t py) {
    if (game == NULL) return (t_tuple){0,0};

    int dx = (dir == DIR_RIGHT) - (dir == DIR_LEFT);
    int dy = (dir == DIR_DOWN) - (dir == DIR_UP);

    int tile = game->tile_size;
    int offset = tile / 10;
    t_tuple out;

    if (dx > 0) out.x = (px - offset) / tile;
    else if (dx < 0) out.x = (px + offset) / tile;
    else out.x = px / tile;

    if (dy > 0) out.y = (py - offset) / tile;
    else if (dy < 0) out.y = (py + offset) / tile;
    else out.y = py / tile;

    return out;
}

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
    if (player == NULL || !sprites_initialized) return 1;

    int current_phase = player->animation_phase % 4; 
    int current_direction = player->sprite_dir % 4; 

    int sprite_index = SPRITE_PLAYER_1_STANDING + (current_phase * 4) + current_direction;

    if (sprite_index >= SPRITE_CACHE_SIZE || scaled_sprite_cache[sprite_index].bytes == NULL) {
        return 1;
    }

    xpm_image_t img = scaled_sprite_cache[sprite_index];

    int32_t draw_y = game->start_y + player->pos.y;

    // --- THE SNEAK OFFSET ---
    if (current_phase == 1) {
        uint32_t sneak_amount = img.height / 20;
        if (sneak_amount == 0) sneak_amount = 1;
        draw_y += sneak_amount;
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

    t_tuple new_pos = player->pos;
    int tile = game->tile_size, half = tile / 2, speed = 4; 

    int dx = (player->dir == DIR_RIGHT) - (player->dir == DIR_LEFT);
    int dy = (player->dir == DIR_DOWN) - (player->dir == DIR_UP);

    new_pos.x += dx * speed;
    new_pos.y += dy * speed;

    t_tuple new_board_pos = player->board_pos;
    int t = 0;

    if (dx > 0) {
        new_board_pos.x = (player->pos.x - half) / tile + 1;
        t = new_board_pos.x * tile + half;
        if (new_pos.x > t) new_pos.x = t;
    } else if (dx < 0) {
        new_board_pos.x = (player->pos.x - half - 1) / tile;
        t = new_board_pos.x * tile + half;
        if (new_pos.x < t) new_pos.x = t;
    } else if (dy > 0) {
        new_board_pos.y = (player->pos.y - half) / tile + 1;
        t = new_board_pos.y * tile + half;
        if (new_pos.y > t) new_pos.y = t;
    } else if (dy < 0) {
        new_board_pos.y = (player->pos.y - half - 1) / tile;
        t = new_board_pos.y * tile + half;
        if (new_pos.y < t) new_pos.y = t;
    }

    if (collision(game->board, new_board_pos)){
        player->is_moving = false;
        return;
    }

    int snap_x = ((new_pos.x - half) % tile) == 0;
    int snap_y = ((new_pos.y - half) % tile) == 0;

    if ((dx != 0 && snap_y) || (dy != 0 && snap_x)) {
        player->pos = new_pos;
        if (snap_x && snap_y) {
            
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
    }
    player->board_pos = continuous_board_pos(game, player->dir, player->pos.x, player->pos.y);
}

void update_player_animation(player_t *player, uint32_t logical_ticks) {
    if (player == NULL) return;

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
