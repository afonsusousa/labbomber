#include "draw.h"
#include "vbe.h"
#include "assets_cache.h"
#include "draw.h"
#include <stdint.h>
#include <lcom/xpm.h>
#include <stdlib.h>

static int seed = 0;

void set_date_seed(int day, int month, int year) {
    seed = year * 10000 + month * 100 + day;
}

int draw_player(player_t *player, hw_video_t *video, uint32_t size) {
    if (player == NULL || !sprites_initialized) return 1;
    if (player->player_id < 1 || player->player_id > 4) return 1;

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

void get_player_start_position(int player_id, uint32_t width, uint32_t height, int32_t *out_x, int32_t *out_y) {
    if (out_x == NULL || out_y == NULL) return;
    if (player_id == 1) {
        *out_x = 50;
        *out_y = 50;
    } else {
        *out_x = width - 100;
        *out_y = height - 100;
    }
}

void update_player_movement(player_t *player, int32_t start_x, int32_t start_y) {
    if (player == NULL) return;
    
    const int SQUARE_SIZE = 300;
    const int MOVE_SPEED = 2;
    const int PAUSE_FRAMES = 120;
    
    int32_t rel_x = player->x - start_x;
    int32_t rel_y = player->y - start_y;
    
    //THIS IS JUST A TEST RN
    if (rel_x < SQUARE_SIZE && rel_y == 0) {
        // Moving right
        player->x += MOVE_SPEED;
        player->direction = PLAYER_RIGHT;
        player->is_moving = true;
        if (player->x - start_x >= SQUARE_SIZE) {
            player->x = start_x + SQUARE_SIZE;
            player->pause_counter = PAUSE_FRAMES;
        }
    } else if (rel_x == SQUARE_SIZE && rel_y < SQUARE_SIZE) {
        // Moving down
        player->y += MOVE_SPEED;
        player->direction = PLAYER_STANDING;
        player->is_moving = true;
        if (player->y - start_y >= SQUARE_SIZE) {
            player->y = start_y + SQUARE_SIZE;
            player->pause_counter = PAUSE_FRAMES;
        }
    } else if (rel_x > 0 && rel_y == SQUARE_SIZE) {
        // Moving left
        player->x -= MOVE_SPEED;
        player->direction = PLAYER_LEFT;
        player->is_moving = true;
        if (player->x - start_x <= 0) {
            player->x = start_x;
            player->pause_counter = PAUSE_FRAMES;
        }
    } else if (rel_x == 0 && rel_y > 0) {
        // Moving up
        player->y -= MOVE_SPEED;
        player->direction = PLAYER_BACK;
        player->is_moving = true;
        if (player->y - start_y <= 0) {
            player->y = start_y;
            player->pause_counter = PAUSE_FRAMES;
        }
    } else {
        // At starting position not moving
        player->is_moving = false;
    }
}

void update_player_animation(player_t *player, uint32_t logical_ticks) {
    if (player == NULL) return;

    if (!player->is_moving) {
        if (player->animation_phase > 1) {
            player->animation_phase = 0;
        }

        if (logical_ticks % 30 == 0) {
            if (player->animation_phase == 0) {
                player->animation_phase = 1;
            } else {
                player->animation_phase = 0;
            }
        }
    } else {
        if (player->animation_phase < 2) {
            player->animation_phase = 2;
        }

        if (logical_ticks % 15 == 0) {
            if (player->animation_phase == 2) {
                player->animation_phase = 3;
            } else {
                player->animation_phase = 2;
            }
        }
    }
}
