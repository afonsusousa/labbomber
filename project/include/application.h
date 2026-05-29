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

typedef enum {
    APP_STATE_MENU_MAIN = 0,
    APP_STATE_MENU_NAME,
    APP_STATE_MENU_SCOREBOARD,
    APP_STATE_MENU_PAUSE,
    APP_STATE_GAME
} app_state_t;

typedef struct s_ctx {
    t_gui gui;
    t_game_state game;
    t_time real_time;
    app_state_t  state; 
} t_ctx;

#include "macros.h"

int app_update_real_time(t_ctx *ctx);
void app_tick_real_time(t_ctx *ctx);

void app_set_state(t_ctx *ctx, app_state_t new_state);
void game_set_phase(t_game_state *game, game_phase_t new_phase);

#endif /* LCOM_PROJECT_APPLICATION_H */
