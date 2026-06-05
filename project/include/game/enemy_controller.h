#ifndef LCOM_PROJECT_ENEMY_CONTROLLER_H
#define LCOM_PROJECT_ENEMY_CONTROLLER_H

#include "models/game_state.h"
#include <stdbool.h>
#include <stdint.h>

void enemy_init(t_game_state *game, enemy_t *enemy, t_tuple spawnpoint);
bool enemy_can_move(t_game_state *game, enemy_t *enemy, direction_t dir);
void choose_enemy_direction(t_game_state *game, enemy_t *enemy);
void update_enemy_movement(t_game_state *game, enemy_t *enemy);
void update_enemy_animation(t_game_state *game, enemy_t *enemy, uint32_t logical_ticks);
void update_enemy_lives(t_game_state *game, enemy_t *enemy, int change);

#endif /* LCOM_PROJECT_ENEMY_CONTROLLER_H */
