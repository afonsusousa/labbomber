#ifndef LCOM_PROJECT_PLAYER_CONTROLLER_H
#define LCOM_PROJECT_PLAYER_CONTROLLER_H

#include "models/game_state.h"
#include <stdint.h>
#include <stdbool.h>

void    player_init(t_game_state *game, player_t *player, t_tuple spawnpoint);
void    update_player_movement(t_game_state *game, player_t *player);
void    update_player_animation(player_t *player, uint32_t logical_ticks);
void    update_player_direction(player_t *player, uint8_t key, bool is_make);
void    update_player_death_animation(t_game_state *game, player_t *player);
void    update_player_win_animation(t_game_state *game, player_t *player);
void    update_player_lives(player_t *player, int change);
void    player_bomb_count(t_game_state *game);
t_tuple spawnpoint_generator(uint8_t *board, uint32_t click_count);

#endif /* LCOM_PROJECT_PLAYER_CONTROLLER_H */
