#ifndef LCOM_PROJECT_GAME_H
#define LCOM_PROJECT_GAME_H

#include "models/types.h"
#include "models/board.h"
#include "models/entity.h"
#include "models/bomb.h"
#include "models/game_state.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

struct s_ctx;
struct s_time;

// Game state
int     game_state_init(t_game_state *game, uint32_t width, uint32_t height, struct s_time time, bool is_multiplayer);
void    game_state_reset(t_game_state *game, struct s_time time, bool is_multiplayer);
void    game_state_destroy(t_game_state *game);
void    game_state_update(struct s_ctx *ctx);
void    game_state_handle_click(t_game_state *game, int32_t x, int32_t y, bool place_bomb);
void    game_state_handle_key_press(t_game_state *game, uint8_t scancode);
void    game_state_handle_player_key(t_game_state *game, uint8_t player_id, uint8_t scancode);

#endif /* LCOM_PROJECT_GAME_H */
