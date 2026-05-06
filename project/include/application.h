#ifndef LCOM_PROJECT_APPLICATION_H
#define LCOM_PROJECT_APPLICATION_H

#include "game.h"
#include "gui.h"

// Hardware-agnostic time abstraction
typedef struct s_time {
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} t_time;

typedef struct s_ctx {
    t_gui gui;
    t_game_state game;
    t_time real_time;       // Decoupled from hardware
} t_ctx;

#endif /* LCOM_PROJECT_APPLICATION_H */
