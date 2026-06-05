#ifndef LCOM_PROJECT_ENTITY_CONTROLLER_H
#define LCOM_PROJECT_ENTITY_CONTROLLER_H

#include "models/game_state.h"
#include <stdbool.h>

bool        collision(t_game_state *game, const entity_t *entity, t_tuple pos);
direction_t opposite_dir(direction_t dir);
int         get_valid_directions(t_game_state *game, t_tuple pos, direction_t out[4]);
bool        entity_overlaps(t_tuple pos_a, t_tuple size_a, t_tuple pos_b, t_tuple size_b);
bool        player_collides_with_enemy(const t_game_state *game, const player_t *player);
t_tuple     get_board_pos(const t_game_state *game, const entity_t *entity);
void        update_entity_movement(t_game_state *game, entity_t *entity);

#endif
