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
uint8_t bomb_explosion_frame(const bomb_t *bomb);

#endif /* LCOM_PROJECT_BOMB_CONTROLLER_H */
