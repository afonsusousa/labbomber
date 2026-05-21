#ifndef LCOM_PROJECT_APPLICATION_H
#define LCOM_PROJECT_APPLICATION_H

#include "game.h"
#include "gui.h"

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
    t_time real_time;
} t_ctx;

#include "macros.h"

int app_update_real_time(t_ctx *ctx);
void app_tick_real_time(t_ctx *ctx);

#endif /* LCOM_PROJECT_APPLICATION_H */
