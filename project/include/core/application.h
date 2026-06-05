#ifndef LCOM_PROJECT_APPLICATION_H
#define LCOM_PROJECT_APPLICATION_H

#include "game/game.h"
#include "gui/gui.h"

#define SCOREBOARD_PATH "/home/lcom/labs/project/scoreboard.dat"

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
    bool multiplayer_local_start_ready;
    bool multiplayer_remote_start_ready;
    bool multiplayer_start_game_sent;
    bool multiplayer_start_game_pending;
    bool multiplayer_game_started;
    uint8_t multiplayer_local_player;
    uint8_t multiplayer_remote_player;
    uint16_t multiplayer_local_nonce;
    uint16_t multiplayer_remote_nonce;
    uint8_t multiplayer_local_tiebreaker;
    uint8_t multiplayer_remote_tiebreaker;
    uint32_t multiplayer_match_seed;
    uint8_t multiplayer_last_player_lives[MAX_PLAYERS];
    bool multiplayer_last_player_active[MAX_PLAYERS];
    uint8_t multiplayer_last_player_powerups[MAX_PLAYERS];
    bool multiplayer_name_sent;
    bool multiplayer_name_received;
    char multiplayer_local_name[32];
    char multiplayer_remote_name[32];

    uint8_t multiplayer_rx_state;
    uint8_t multiplayer_rx_type;
    uint8_t multiplayer_rx_data[3];
    uint8_t multiplayer_rx_pos;
} t_ctx;

#endif /* LCOM_PROJECT_APPLICATION_H */
