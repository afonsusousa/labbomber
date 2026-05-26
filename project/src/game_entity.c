#include "game.h"

bool collision(uint8_t *board, t_tuple pos) {

    if (pos.x < 0 || pos.y < 0 || pos.x >= BOARD_COLS || pos.y >= BOARD_ROWS) return true;

    return board[BOARD_IDX(pos.x, pos.y)] != TILE_TYPE_GRASS;
}

direction_t opposite_dir(direction_t dir) {

    if (dir == DIR_LEFT) return DIR_RIGHT;

    if (dir == DIR_RIGHT) return DIR_LEFT;

    if (dir == DIR_UP) return DIR_DOWN;

    return DIR_UP;
}

int get_valid_directions(uint8_t *board, t_tuple pos, direction_t out[4]) {

    if (board == NULL || out == NULL) return 0;

    int count = 0;

    t_tuple next;

    // LEFT
    next = pos;
    next.x--;

    if (!collision(board, next)) {
        out[count] = DIR_LEFT;
        count++;
    }

    // RIGHT
    next = pos;
    next.x++;

    if (!collision(board, next)) {
        out[count] = DIR_RIGHT;
        count++;
    }

    // UP
    next = pos;
    next.y--;

    if (!collision(board, next)) {
        out[count] = DIR_UP;
        count++;
    }

    // DOWN
    next = pos;
    next.y++;

    if (!collision(board, next)) {
        out[count] = DIR_DOWN;
        count++;
    }

    return count;
}

t_tuple entering_cell(const t_game_state *game, direction_t dir, int32_t px, int32_t py, uint32_t w, uint32_t h) {
    if (game == NULL) return (t_tuple){0,0};

    int dx = (dir == DIR_RIGHT) - (dir == DIR_LEFT);
    int dy = (dir == DIR_DOWN) - (dir == DIR_UP);

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
