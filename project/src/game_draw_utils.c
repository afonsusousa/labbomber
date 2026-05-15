#include "vbe.h"
#include "assets_cache.h"
#include <stdint.h>
#include <lcom/xpm.h>

static void draw_xpm_scaled(hw_video_t *video, xpm_image_t img, int32_t x, int32_t y, uint32_t size) {
    if (img.bytes == NULL || size == 0) return;

    if (size == img.width && size == img.height) {
        hw_vbe_draw_xpm(video, img.bytes, img, x, y);
        return;
    }

    uint32_t scale = size / img.width;
    if (scale == 0) scale = 1;

    hw_vbe_draw_scaled_xpm(video, img.bytes, img, x, y, scale);
}

void draw_grass(hw_video_t *video, int32_t x, int32_t y, int type, uint32_t size) {
    if (!sprites_initialized) return;

    int sprite_index;
    switch (type) {
        case 0: sprite_index = SPRITE_GRASS; break;
        case 1: sprite_index = SPRITE_GRASS_TOP_LEFT; break;
        case 2: sprite_index = SPRITE_GRASS_TOP; break;
        case 3: sprite_index = SPRITE_GRASS_LEFT; break;
        case 4: sprite_index = SPRITE_GRASS_LEFT_BORDER; break;
        case 5: sprite_index = SPRITE_GRASS_TOP_BORDER; break;
        case 6: sprite_index = SPRITE_GRASS_TOP_LEFT_BORDER; break;
        default: sprite_index = SPRITE_GRASS; break;
    }

    if (sprite_cache[sprite_index].bytes != NULL) {
        draw_xpm_scaled(video, sprite_cache[sprite_index], x, y, size);
    }
}

static int is_solid(const uint8_t *board, int rows, int cols, int x, int y) {
    if (x < 0 || x >= cols || y < 0 || y >= rows) return 1;
    uint8_t val = board[y * cols + x];
    return (val != 0); 
}

//needs cleaning up
int decide_grass_sprite(const uint8_t *board, int rows, int cols, int x, int y) {
    int U  = is_solid(board, rows, cols, x, y - 1);     // Wall directly above
    int L  = is_solid(board, rows, cols, x - 1, y);     // Wall directly left
    int UL = is_solid(board, rows, cols, x - 1, y - 1); // Wall top-left
    int UR = is_solid(board, rows, cols, x + 1, y - 1); // Wall top-right 
    int DL = is_solid(board, rows, cols, x - 1, y + 1); // Wall bottom-left 

    // 1. Inner Corner (Walls above and left meet)
    if (U && L) {
        return 1; // SPRITE_GRASS_TOP_LEFT 
    } 
    
    // 2. Top Edge Shadow
    if (U) {
        if (!UR) {
            return 5; // SPRITE_GRASS_TOP_BORDER (Terminating shadow)
        }
        return 2; // SPRITE_GRASS_TOP (Continuous shadow)
    } 
    
    // 3. Left Edge Shadow
    if (L) {
        if (!DL) {
            return 4; // SPRITE_GRASS_LEFT_BORDER (Terminating shadow)
        }
        return 3; // SPRITE_GRASS_LEFT (Continuous shadow)
    }
    
    // 4. Outer Corner Shadow
    if (UL)
        return 6; // SPRITE_GRASS_TOP_LEFT_BORDER
    
    // 5. Plain Center Grass
    return 0; // SPRITE_GRASS
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

// Draw wall sprite
void draw_wall(hw_video_t *video, int32_t x, int32_t y, int sprite_index, uint32_t size) {
    if (!sprites_initialized || sprite_cache[sprite_index].bytes == NULL) {
        hw_vbe_draw_rect(video, x, y, size, size, 0x666666);
        return;
    }
    
    xpm_image_t img = sprite_cache[sprite_index];
    draw_xpm_scaled(video, img, x, y, size);
}

// Draw brick sprite
void draw_brick(hw_video_t *video, int32_t x, int32_t y, uint32_t size) {
    if (!sprites_initialized || sprite_cache[SPRITE_BRICK].bytes == NULL) {
        hw_vbe_draw_rect(video, x, y, size, size, 0x884400);
        return;
    }
    
    xpm_image_t img = sprite_cache[SPRITE_BRICK];
    draw_xpm_scaled(video, img, x, y, size);
}
