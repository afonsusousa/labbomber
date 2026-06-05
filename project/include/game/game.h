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
void    game_state_handle_click(t_game_state *game, int32_t x, int32_t y);
void    game_state_handle_key_press(t_game_state *game, uint8_t scancode);
void    game_state_handle_player_key(t_game_state *game, uint8_t player_id, uint8_t scancode);

void    gui_show_game_view(struct s_ctx *ctx);
void    gui_reset_game_view(struct s_ctx *ctx);

// Map helpers
int     decide_grass_sprite(const uint8_t *board, int rows, int cols, int x, int y);
int     decide_wall_sprite(const uint8_t *board, int rows, int cols, int x, int y);

// Bomb helpers
void    bomb_init(bomb_t *bomb);
void    bomb_reset(bomb_t *bomb);
void    bomb_clear_explosion(bomb_t *bomb);
void    bomb_update(t_game_state *game, bomb_t *bomb);
void    bomb_begin_explosion(t_game_state *game, bomb_t *bomb);
void    bomb_update_explosion(t_game_state *game, bomb_t *bomb);
void    place_player_bomb(t_game_state *game, player_t *player);
uint8_t bomb_explosion_frame(const bomb_t *bomb);

#endif /* LCOM_PROJECT_GAME_H */
