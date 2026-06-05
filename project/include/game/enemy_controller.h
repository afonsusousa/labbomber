#ifndef LCOM_PROJECT_ENEMY_CONTROLLER_H
#define LCOM_PROJECT_ENEMY_CONTROLLER_H

#include "models/game_state.h"
#include <stdbool.h>
#include <stdint.h>

void enemy_init(t_game_state *game, enemy_t *enemy, t_tuple spawnpoint);
bool enemy_can_move(t_game_state *game, enemy_t *enemy, direction_t dir);
void choose_enemy_direction(t_game_state *game, enemy_t *enemy);
bool enemy_try_resume_movement(t_game_state *game, enemy_t *enemy);
void update_enemy_movement(t_game_state *game, enemy_t *enemy);
void update_enemy_animation(t_game_state *game, enemy_t *enemy, uint32_t logical_ticks);
void update_enemy_lives(t_game_state *game, enemy_t *enemy, int change);

int spawn_enemies_singleplayer(uint8_t *board, t_tuple player, int n, t_tuple out[MAX_ENEMIES]);
int spawn_enemies_multiplayer(uint8_t *board, int n, t_tuple out[MAX_ENEMIES]);
int spawn_new_enemy(t_game_state *game, t_tuple *out);

#endif /* LCOM_PROJECT_ENEMY_CONTROLLER_H */
