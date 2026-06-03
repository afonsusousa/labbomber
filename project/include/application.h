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
    bool is_multiplayer;
    bool multiplayer_partner_ready;
    bool multiplayer_signal_sent;
    bool multiplayer_role_assigned;
    uint8_t multiplayer_local_player;
    uint8_t multiplayer_remote_player;
    uint16_t multiplayer_local_nonce;
    uint16_t multiplayer_remote_nonce;
    uint8_t multiplayer_rx_state;
    uint8_t multiplayer_rx_type;
    uint8_t multiplayer_rx_data[2];
    uint8_t multiplayer_rx_pos;
} t_ctx;

#include "macros.h"

int app_update_real_time(t_ctx *ctx);
void app_tick_real_time(t_ctx *ctx);

#endif /* LCOM_PROJECT_APPLICATION_H */
