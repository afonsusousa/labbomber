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

void update_player_movement(player_t *player, int32_t start_x, int32_t start_y) {
    if (player == NULL) return;
    
    const int MOVE_SPEED = 2;
    const int MAX_WIDTH = 1024;   // Board width constraint
    const int MAX_HEIGHT = 768;   // Board height constraint
    
    if (!player->is_moving) {
        return;
    }
    
    switch (player->direction) {
        case PLAYER_BACK:
            // Move up
            if (player->y - MOVE_SPEED >= start_y) {
                player->y -= MOVE_SPEED;
            }
            break;
        case PLAYER_LEFT:
            // Move left
            if (player->x - MOVE_SPEED >= start_x) {
                player->x -= MOVE_SPEED;
            }
            break;
        case PLAYER_RIGHT:
            // Move right
            if (player->x + MOVE_SPEED <= start_x + MAX_WIDTH) {
                player->x += MOVE_SPEED;
            }
            break;
        case PLAYER_STANDING:
            // Move down
            if (player->y + MOVE_SPEED <= start_y + MAX_HEIGHT) {
                player->y += MOVE_SPEED;
            }
            break;
        default:
            break;
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
