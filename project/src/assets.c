#include "assets_cache.h"
#include "assets.h"
#include "glyphs.h"
#include "assets_player.h"
#include "assets_enemy.h"
#include <lcom/xpm.h>
#include <stdlib.h>
#include <stdbool.h>

/* Sprite cache storage */
xpm_image_t sprite_cache[SPRITE_CACHE_SIZE];
xpm_image_t scaled_sprite_cache[SPRITE_CACHE_SIZE];
bool sprites_initialized = false;

void init_sprite_cache() {
    for (int i = 32; i < 127; i++) {
        const xpm_row_t* xpm = get_letter_xpm(i);
        if (xpm != NULL) {
            xpm_load((xpm_map_t)xpm, XPM_5_6_5, &sprite_cache[i]);
        } else {
            sprite_cache[i].bytes = NULL;
        }
    }

    xpm_load((xpm_map_t)my_mouse_xpm, XPM_5_6_5, &sprite_cache[SPRITE_MOUSE_POINTER]);
    xpm_load((xpm_map_t)text_cursor_xpm, XPM_5_6_5, &sprite_cache[SPRITE_MOUSE_TEXT]);

    /* Load all game assets */
    xpm_load((xpm_map_t)bomb1, XPM_5_6_5, &sprite_cache[SPRITE_BOMB1]);
    xpm_load((xpm_map_t)bomb2, XPM_5_6_5, &sprite_cache[SPRITE_BOMB2]);
    xpm_load((xpm_map_t)bomb3, XPM_5_6_5, &sprite_cache[SPRITE_BOMB3]);
    xpm_load((xpm_map_t)brick, XPM_5_6_5, &sprite_cache[SPRITE_BRICK]);
    xpm_load((xpm_map_t)explosion_1_center, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_1_CENTER]);
    xpm_load((xpm_map_t)explosion_2_center, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_2_CENTER]);
    xpm_load((xpm_map_t)explosion_3_arm, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_3_ARM]);
    xpm_load((xpm_map_t)explosion_3_center, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_3_CENTER]);
    xpm_load((xpm_map_t)explosion_3_hand, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_3_HAND]);
    xpm_load((xpm_map_t)explosion_4_arm, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_4_ARM]);
    xpm_load((xpm_map_t)explosion_4_center, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_4_CENTER]);
    xpm_load((xpm_map_t)explosion_4_hand, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_4_HAND]);
    xpm_load((xpm_map_t)explosion_5_arm, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_5_ARM]);
    xpm_load((xpm_map_t)explosion_5_center, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_5_CENTER]);
    xpm_load((xpm_map_t)explosion_5_hand, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_5_HAND]);
    
    /* Load player assets */
    xpm_load((xpm_map_t)player_1_standing_xpm, XPM_5_6_5, &sprite_cache[SPRITE_PLAYER_1_STANDING]);
    xpm_load((xpm_map_t)player_1_left_xpm, XPM_5_6_5, &sprite_cache[SPRITE_PLAYER_1_LEFT]);
    xpm_load((xpm_map_t)player_1_right_xpm, XPM_5_6_5, &sprite_cache[SPRITE_PLAYER_1_RIGHT]);
    xpm_load((xpm_map_t)player_1_back_xpm, XPM_5_6_5, &sprite_cache[SPRITE_PLAYER_1_BACK]);
    xpm_load((xpm_map_t)player_2_standing_xpm, XPM_5_6_5, &sprite_cache[SPRITE_PLAYER_2_STANDING]);
    xpm_load((xpm_map_t)player_2_left_xpm, XPM_5_6_5, &sprite_cache[SPRITE_PLAYER_2_LEFT]);
    xpm_load((xpm_map_t)player_2_right_xpm, XPM_5_6_5, &sprite_cache[SPRITE_PLAYER_2_RIGHT]);
    xpm_load((xpm_map_t)player_2_back_xpm, XPM_5_6_5, &sprite_cache[SPRITE_PLAYER_2_BACK]);
    xpm_load((xpm_map_t)player_3_standing_xpm, XPM_5_6_5, &sprite_cache[SPRITE_PLAYER_3_STANDING]);
    xpm_load((xpm_map_t)player_3_left_xpm, XPM_5_6_5, &sprite_cache[SPRITE_PLAYER_3_LEFT]);
    xpm_load((xpm_map_t)player_3_right_xpm, XPM_5_6_5, &sprite_cache[SPRITE_PLAYER_3_RIGHT]);
    xpm_load((xpm_map_t)player_3_back_xpm, XPM_5_6_5, &sprite_cache[SPRITE_PLAYER_3_BACK]);
    xpm_load((xpm_map_t)player_4_standing_xpm, XPM_5_6_5, &sprite_cache[SPRITE_PLAYER_4_STANDING]);
    xpm_load((xpm_map_t)player_4_left_xpm, XPM_5_6_5, &sprite_cache[SPRITE_PLAYER_4_LEFT]);
    xpm_load((xpm_map_t)player_4_right_xpm, XPM_5_6_5, &sprite_cache[SPRITE_PLAYER_4_RIGHT]);
    xpm_load((xpm_map_t)player_4_back_xpm, XPM_5_6_5, &sprite_cache[SPRITE_PLAYER_4_BACK]);

    xpm_load((xpm_map_t)enemy_1_standing_xpm, XPM_5_6_5, &sprite_cache[SPRITE_ENEMY_1_STANDING]);
    xpm_load((xpm_map_t)enemy_2_standing_xpm, XPM_5_6_5, &sprite_cache[SPRITE_ENEMY_2_STANDING]);
    xpm_load((xpm_map_t)enemy_3_standing_xpm, XPM_5_6_5, &sprite_cache[SPRITE_ENEMY_3_STANDING]);
    xpm_load((xpm_map_t)enemy_4_standing_xpm, XPM_5_6_5, &sprite_cache[SPRITE_ENEMY_4_STANDING]);

    xpm_load((xpm_map_t)explosion_6_arm, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_6_ARM]);
    xpm_load((xpm_map_t)explosion_6_center, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_6_CENTER]);
    xpm_load((xpm_map_t)explosion_6_hand, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_6_HAND]);
    xpm_load((xpm_map_t)flower1, XPM_5_6_5, &sprite_cache[SPRITE_FLOWER1]);
    xpm_load((xpm_map_t)flower2, XPM_5_6_5, &sprite_cache[SPRITE_FLOWER2]);
    xpm_load((xpm_map_t)flower3, XPM_5_6_5, &sprite_cache[SPRITE_FLOWER3]);
    xpm_load((xpm_map_t)grass_left_broder, XPM_5_6_5, &sprite_cache[SPRITE_GRASS_LEFT_BORDER]);
    xpm_load((xpm_map_t)grass_left, XPM_5_6_5, &sprite_cache[SPRITE_GRASS_LEFT]);
    xpm_load((xpm_map_t)grass_top_border, XPM_5_6_5, &sprite_cache[SPRITE_GRASS_TOP_BORDER]);
    xpm_load((xpm_map_t)grass_top_left_border, XPM_5_6_5, &sprite_cache[SPRITE_GRASS_TOP_LEFT_BORDER]);
    xpm_load((xpm_map_t)grass_top_left, XPM_5_6_5, &sprite_cache[SPRITE_GRASS_TOP_LEFT]);
    xpm_load((xpm_map_t)grass_top, XPM_5_6_5, &sprite_cache[SPRITE_GRASS_TOP]);
    xpm_load((xpm_map_t)grass, XPM_5_6_5, &sprite_cache[SPRITE_GRASS]);
    xpm_load((xpm_map_t)wall1, XPM_5_6_5, &sprite_cache[SPRITE_WALL1]);
    xpm_load((xpm_map_t)wall2, XPM_5_6_5, &sprite_cache[SPRITE_WALL2]);
    xpm_load((xpm_map_t)wall3, XPM_5_6_5, &sprite_cache[SPRITE_WALL3]);

    sprites_initialized = true;
}

static uint32_t get_pixel_color(uint8_t *ptr, uint8_t bpp) {
    if (bpp == 2) return *(uint16_t *)ptr;
    if (bpp == 4) return *(uint32_t *)ptr;
    uint32_t color = 0;
    for (int i = 0; i < bpp; i++) color |= (ptr[i] << (i * 8));
    return color;
}

static void set_pixel_color(uint8_t *ptr, uint8_t bpp, uint32_t color) {
    if (bpp == 2) *(uint16_t *)ptr = (uint16_t)color;
    else if (bpp == 4) *(uint32_t *)ptr = color;
    else {
        for (int i = 0; i < bpp; i++) ptr[i] = (color >> (i * 8)) & 0xFF;
    }
}

void scale_cached_sprite(int index, uint32_t target_w, uint32_t target_h, uint8_t bpp) {
    if (index < 0 || index >= SPRITE_CACHE_SIZE || sprite_cache[index].bytes == NULL) return;
    
    // Free previously scaled image if present
    if (scaled_sprite_cache[index].bytes != NULL && scaled_sprite_cache[index].bytes != sprite_cache[index].bytes) {
        free(scaled_sprite_cache[index].bytes);
    }
    
    // If no scaling needed
    if (target_w == sprite_cache[index].width && target_h == sprite_cache[index].height) {
        scaled_sprite_cache[index] = sprite_cache[index]; // Note: bytes pointer points to original, shouldn't be freed
        return;
    }

    xpm_image_t orig = sprite_cache[index];
    xpm_image_t scaled = orig;
    scaled.width = target_w;
    scaled.height = target_h;
    
    scaled.bytes = malloc(target_w * target_h * bpp);
    if (!scaled.bytes) return;

    for (uint32_t y = 0; y < target_h; y++) {
        uint32_t src_y = (y * orig.height) / target_h;
        uint8_t *src_row = orig.bytes + (src_y * orig.width * bpp);
        uint8_t *dst_row = scaled.bytes + (y * target_w * bpp);

        for (uint32_t x = 0; x < target_w; x++) {
            uint32_t src_x = (x * orig.width) / target_w;
            uint32_t color = get_pixel_color(src_row + (src_x * bpp), bpp);
            set_pixel_color(dst_row + (x * bpp), bpp, color);
        }
    }
    
    scaled_sprite_cache[index] = scaled;
}

void scale_all_game_sprites(uint32_t tile_size, uint32_t player_w, uint32_t player_h, uint8_t bpp) {
    if (!sprites_initialized) return;

    // Grass
    scale_cached_sprite(SPRITE_GRASS_LEFT_BORDER, tile_size, tile_size, bpp);
    scale_cached_sprite(SPRITE_GRASS_LEFT, tile_size, tile_size, bpp);
    scale_cached_sprite(SPRITE_GRASS_TOP_BORDER, tile_size, tile_size, bpp);
    scale_cached_sprite(SPRITE_GRASS_TOP_LEFT_BORDER, tile_size, tile_size, bpp);
    scale_cached_sprite(SPRITE_GRASS_TOP_LEFT, tile_size, tile_size, bpp);
    scale_cached_sprite(SPRITE_GRASS_TOP, tile_size, tile_size, bpp);
    scale_cached_sprite(SPRITE_GRASS, tile_size, tile_size, bpp);

    // Walls & Bricks
    scale_cached_sprite(SPRITE_BOMB1, tile_size, tile_size, bpp);
    scale_cached_sprite(SPRITE_BOMB2, tile_size, tile_size, bpp);
    scale_cached_sprite(SPRITE_BOMB3, tile_size, tile_size, bpp);
    scale_cached_sprite(SPRITE_BRICK, tile_size, tile_size, bpp);
    scale_cached_sprite(SPRITE_WALL1, tile_size, tile_size, bpp);
    scale_cached_sprite(SPRITE_WALL2, tile_size, tile_size, bpp);
    scale_cached_sprite(SPRITE_WALL3, tile_size, tile_size, bpp);

    // Players (looping all 16 states)
    for (int i = 0; i < 16; i++) {
        scale_cached_sprite(SPRITE_PLAYER_1_STANDING + i, player_w, player_h, bpp);
    }
    // Enemy
    for (int i = 0; i < 4; i++) {
        scale_cached_sprite(SPRITE_ENEMY_1_STANDING + i, player_w, player_h, bpp);
    }
}
