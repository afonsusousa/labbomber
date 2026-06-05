#ifndef MODELS_GAME_STATE_H
#define MODELS_GAME_STATE_H

#include "models/types.h"
#include "models/entity.h"
#include "models/bomb.h"
#include "models/board.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct s_game_state {
    uint32_t width;
    uint32_t height;
    uint32_t tile_size;
    int32_t  start_x;
    int32_t  start_y;

    uint8_t board[BOARD_ROWS * BOARD_COLS];

    t_tuple door_pos;
    bool    door_open;

    player_t players[MAX_PLAYERS];
    uint8_t  current_player;

    enemy_t enemies[MAX_ENEMIES];
    uint8_t enemy_count;

    bomb_t bomb[MAX_BOMBS];

    uint32_t      score;
    uint32_t      enemies_to_kill;
    uint32_t      logical_ticks;
    uint32_t      last_enemy_spawn_ticks; //solving bug of spawning multiples enemies on the same tick
    uint32_t      time_limit;
    uint32_t      click_count;
    bool          is_frozen;
    uint32_t      animation_timer;
    uint32_t      enemy_seed;
    uint32_t      multiplayer_last_contact_ticks;
    bool          is_multiplayer;
    match_state_t match_state;
} t_game_state;

#endif /* MODELS_GAME_STATE_H */
