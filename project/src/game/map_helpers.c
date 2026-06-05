#include "game/map_helpers.h"
#include "models/board.h"
#include "view/assets_cache.h"
#include <stdint.h>

static int is_solid(const uint8_t *board, int rows, int cols, int x, int y) {
    if (x < 0 || x >= cols || y < 0 || y >= rows) return 1;
    uint8_t val = board[y * cols + x];
    return (val != TILE_TYPE_GRASS && val != TILE_TYPE_DOOR && val != TILE_TYPE_POWERUP_REACH && val != TILE_TYPE_POWERUP_COUNT);
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
    if (cell_type == TILE_TYPE_WALL) {
        return SPRITE_WALL1;
    }

    int seed = (x * 31) + (y * 7);
    switch (seed % 3) {
        case 0:  return SPRITE_WALL1;
        case 1:  return SPRITE_WALL2;
        default: return SPRITE_WALL3;
    }
}
