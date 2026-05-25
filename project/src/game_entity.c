#include "game.h"

bool collision(uint8_t *board, t_tuple pos) {

    if (pos.x < 0 || pos.y < 0 || pos.x >= BOARD_COLS || pos.y >= BOARD_ROWS) return true;

    return board[BOARD_IDX(pos.x, pos.y)] != 0;
}
