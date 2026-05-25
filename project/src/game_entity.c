#include "game.h"

bool collision(uint8_t *board, t_tuple pos) {

    if (pos.x < 0 || pos.y < 0 || pos.x >= BOARD_COLS || pos.y >= BOARD_ROWS) return true;

    return board[BOARD_IDX(pos.x, pos.y)] != 0;
}

direction_t opposite_dir(direction_t dir) {

    if (dir == DIR_LEFT) return DIR_RIGHT;

    if (dir == DIR_RIGHT) return DIR_LEFT;

    if (dir == DIR_BACK) return DIR_STANDING;

    return DIR_BACK;
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

    // BACK
    next = pos;
    next.y--;

    if (!collision(board, next)) {
        out[count] = DIR_BACK;
        count++;
    }

    // STANDING
    next = pos;
    next.y++;

    if (!collision(board, next)) {
        out[count] = DIR_STANDING;
        count++;
    }

    return count;
}
