#ifndef LCOM_PROJECT_BOMB_CONTROLLER_H
#define LCOM_PROJECT_BOMB_CONTROLLER_H

#include "models/game_state.h"
#include <stdint.h>

void bomb_init(bomb_t *bomb);
void bomb_reset(bomb_t *bomb);
void bomb_clear_explosion(bomb_t *bomb);
void bomb_update(t_game_state *game, bomb_t *bomb);
void bomb_begin_explosion(t_game_state *game, bomb_t *bomb);
void bomb_update_explosion(t_game_state *game, bomb_t *bomb);
void place_player_bomb(t_game_state *game, player_t *player);
void bomb_drag_move(t_game_state *game, int32_t px, int32_t py);
void bomb_drag_end(t_game_state *game);
uint8_t bomb_explosion_frame(const bomb_t *bomb);
int8_t bomb_drag_start(t_game_state *game, int32_t px, int32_t py);

#endif /* LCOM_PROJECT_BOMB_CONTROLLER_H */
