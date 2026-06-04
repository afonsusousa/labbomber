#ifndef MODELS_BOARD_H
#define MODELS_BOARD_H

#ifndef BOARD_ROWS
#define BOARD_ROWS 11
#endif

#ifndef BOARD_COLS
#define BOARD_COLS 17
#endif

#define TOTAL_CELLS     (BOARD_ROWS * BOARD_COLS)
#define BOARD_IDX(x, y) ((y) * BOARD_COLS + (x))

#define TILE_TYPE_GRASS         0
#define TILE_TYPE_WALL          1
#define TILE_TYPE_BRICK         2
#define TILE_TYPE_DOOR          3
#define TILE_TYPE_POWERUP_REACH 4
#define TILE_TYPE_POWERUP_COUNT 5

#define GET_X(game, value) ((game)->start_x + (value) * (game)->tile_size + (game)->tile_size / 2)
#define GET_Y(game, value) ((game)->start_y + (value) * (game)->tile_size + (game)->tile_size / 2)

#endif /* MODELS_BOARD_H */
