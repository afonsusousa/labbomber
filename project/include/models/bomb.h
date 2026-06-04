#ifndef MODELS_BOMB_H
#define MODELS_BOMB_H

#include "models/types.h"
#include <stdint.h>
#include <stdbool.h>

#define MAX_BOMBS 4

#define BOMB_EXPLOSION_RANGE 1

#define BOMB_DURATION_TICKS           ((5 * 60) / 2)
#define BOMB_EXPLOSION_DURATION_TICKS ((2 * 60) / 5)
#define BOMB_FUSE_PHASES              8

#define BOMB_INACTIVE 0
#define BOMB_PLACED   1
#define BOMB_BLINK    2
#define BOMB_EXPLODE  3
#define BOMB_FIRE     4

enum {
    EXPLOSION_DIR_RIGHT = 0,
    EXPLOSION_DIR_LEFT  = 1,
    EXPLOSION_DIR_DOWN  = 2,
    EXPLOSION_DIR_UP    = 3,
    EXPLOSION_DIR_COUNT = 4,
};

typedef struct {
    bool     active;
    uint8_t  state;
    uint8_t  player_id;
    t_tuple  board_pos;
    uint32_t bomb_timer;
    uint32_t explosion_timer;
    uint8_t  radius;
    uint8_t  reach[EXPLOSION_DIR_COUNT];
} bomb_t;

#endif /* MODELS_BOMB_H */
