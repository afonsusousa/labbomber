#include "draw.h"
#include "vbe.h"
#include "assets_cache.h"
#include "draw.h"
#include <stdint.h>
#include <lcom/xpm.h>
#include <stdlib.h>
#include <string.h>

static int seed = 0;

void set_date_seed(int day, int month, int year) {
    seed = year * 10000 + month * 100 + day;
}

int draw_player(player_t *player, hw_video_t *video, uint32_t size) {
    if (player == NULL || !sprites_initialized) return 1;

    int current_phase = player->animation_phase % 4; 
    int current_direction = player->direction % 4; 

    int sprite_index = SPRITE_PLAYER_1_STANDING + (current_phase * 4) + current_direction;

    if (sprite_index >= SPRITE_CACHE_SIZE || sprite_cache[sprite_index].bytes == NULL) {
        return 1;
    }

    xpm_image_t img = sprite_cache[sprite_index];
    uint32_t scale = size / img.width;
    if (scale == 0) scale = 1;
    
    int32_t draw_x = player->x + 100;
    int32_t draw_y = player->y + 100;
    
    // --- THE SNEAK OFFSET ---
    if (current_phase == 1) {
        draw_y += (1 * scale); 
    }
    
    hw_vbe_draw_scaled_xpm(video, img.bytes, img, draw_x, draw_y, scale);
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

void update_player_direction(player_t *player, uint8_t key_index, bool is_make) {
    if (key_index != KEY_W && key_index != KEY_A && 
        key_index != KEY_D && key_index != KEY_S) {
        return;
    }

    if (is_make) {
        push_movement_key(player, key_index);
    } else {
        remove_movement_key(player, key_index);
    }

    if (player->stack_count > 0) {
        player->is_moving = true;
        uint8_t active_key = player->movement_stack[player->stack_count - 1];
        
        if (active_key == KEY_W) player->direction = PLAYER_BACK;
        else if (active_key == KEY_A) player->direction = PLAYER_LEFT;
        else if (active_key == KEY_D) player->direction = PLAYER_RIGHT;
        else if (active_key == KEY_S) player->direction = PLAYER_STANDING;
    } else {
        player->is_moving = false;
    }
}

// collisions will go here
void update_player_movement(player_t *player, int32_t start_x, int32_t start_y) {
    if (player == NULL) return;
    
    const int MOVE_SPEED = 2;
    const int MAX_WIDTH = 1024;
    const int MAX_HEIGHT = 768;
    
    if (!player->is_moving) {
        return;
    }
    
    if (player->direction == PLAYER_BACK) {
        if (player->y - MOVE_SPEED >= start_y)
            player->y -= MOVE_SPEED;
    } 
    else if (player->direction == PLAYER_LEFT) {
        if (player->x - MOVE_SPEED >= start_x)
            player->x -= MOVE_SPEED;
    } 
    else if (player->direction == PLAYER_RIGHT) {
        if (player->x + MOVE_SPEED <= start_x + MAX_WIDTH)
            player->x += MOVE_SPEED;
    } 
    else if (player->direction == PLAYER_STANDING) {
        if (player->y + MOVE_SPEED <= start_y + MAX_HEIGHT)
            player->y += MOVE_SPEED;
    }
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
