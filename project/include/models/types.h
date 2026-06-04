#ifndef MODELS_TYPES_H
#define MODELS_TYPES_H

#include <stdint.h>
#include <stdbool.h>

typedef struct s_tuple {
    int32_t x;
    int32_t y;
} t_tuple;

typedef enum {
    DIR_DOWN  = 0,
    DIR_LEFT  = 1,
    DIR_RIGHT = 2,
    DIR_UP    = 3
} direction_t;

typedef enum {
    MATCH_RUNNING,
    MATCH_PAUSED,
    MATCH_WON,
    MATCH_LOST,
    MATCH_EXITING
} match_state_t;

#endif /* MODELS_TYPES_H */
