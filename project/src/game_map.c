#include "draw.h"
#include "assets_cache.h"
#include <stdint.h>
#include <lcom/xpm.h>

static int is_solid(const uint8_t *board, int rows, int cols, int x, int y) {
    if (x < 0 || x >= cols || y < 0 || y >= rows) return 1;
    uint8_t val = board[y * cols + x];
    return (val != 0);
}

void draw_grass(hw_video_t *video, int32_t x, int32_t y, int sprite_index, uint32_t size) {
    if (!sprites_initialized || sprite_cache[sprite_index].bytes == NULL) return;

    xpm_image_t img = sprite_cache[sprite_index];
    uint32_t scale = size / img.width;
    if (scale == 0) scale = 1;
    hw_vbe_draw_scaled_xpm(video, img.bytes, img, x, y, scale);
}

int decide_grass_sprite(const uint8_t *board, int rows, int cols, int x, int y) {
    int U  = is_solid(board, rows, cols, x, y - 1);     // UP
    int L  = is_solid(board, rows, cols, x - 1, y);     // LEFT
    int UL = is_solid(board, rows, cols, x - 1, y - 1); // UPLEFT

    if (U && L)
        return SPRITE_GRASS_TOP_LEFT;

    if (U) {
        if (!UL)
            return SPRITE_GRASS_TOP_BORDER;
        return SPRITE_GRASS_TOP;
    }
    
    if (L) {
        if (!UL)
            return SPRITE_GRASS_LEFT_BORDER;
        return SPRITE_GRASS_LEFT;
    }
    
    if (UL)
        return SPRITE_GRASS_TOP_LEFT_BORDER;
    
    return SPRITE_GRASS;
}

int decide_wall_sprite(const uint8_t *board, int rows, int cols, int x, int y) {
    uint8_t cell_type = board[y * cols + x];

    // If it's a solid block (type 1)
    if (cell_type == 1) {
        return SPRITE_WALL1;
    }

    int seed = (x * 31) + (y * 7); 
    switch (seed % 3) {
        case 0:  return SPRITE_WALL1;
        case 1:  return SPRITE_WALL2;
        default: return SPRITE_WALL3;
    }
}

void draw_wall(hw_video_t *video, int32_t x, int32_t y, int sprite_index, uint32_t size) {
    if (!sprites_initialized || sprite_cache[sprite_index].bytes == NULL) {
        hw_vbe_draw_rect(video, x, y, size, size, 0x666666);
        return;
    }
    
    xpm_image_t img = sprite_cache[sprite_index];
    uint32_t scale = size / img.width;
    if (scale == 0) scale = 1;
    hw_vbe_draw_scaled_xpm(video, img.bytes, img, x, y, scale);
}

void draw_brick(hw_video_t *video, int32_t x, int32_t y, uint32_t size) {
    if (!sprites_initialized || sprite_cache[SPRITE_BRICK].bytes == NULL) {
        hw_vbe_draw_rect(video, x, y, size, size, 0x884400);
        return;
    }
    
    xpm_image_t img = sprite_cache[SPRITE_BRICK];
    uint32_t scale = size / img.width;
    if (scale == 0) scale = 1;
    hw_vbe_draw_scaled_xpm(video, img.bytes, img, x, y, scale);
}
