#ifndef MODELS_SCORE_H
#define MODELS_SCORE_H

#include <stdint.h>

typedef struct {
    char     player_name[32];
    uint32_t score;
    uint32_t duration_ticks;
    uint8_t  day;
    uint8_t  month;
    uint8_t  year;
} score_entry_t;

#endif /* MODELS_SCORE_H */
