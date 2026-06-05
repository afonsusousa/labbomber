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

// Entity helpers
bool        collision(struct s_game_state *game, const struct s_entity *entity, t_tuple pos);
direction_t opposite_dir(direction_t dir);
int         get_valid_directions(struct s_game_state *game, t_tuple pos, direction_t out[4]);
bool        entity_overlaps(t_tuple pos_a, t_tuple size_a, t_tuple pos_b, t_tuple size_b);
bool        player_collides_with_enemy(const t_game_state *game, const player_t *player);

// Entity movement
t_tuple get_board_pos(const t_game_state *game, const entity_t *entity);
void    update_entity_movement(t_game_state *game, entity_t *entity);

// Player helpers
t_tuple spawnpoint_generator(uint8_t *board, uint32_t click_count);
void    player_init(t_game_state *game, player_t *player, t_tuple spawnpoint);
void    update_player_movement(t_game_state *game, player_t *player);
void    update_player_animation(player_t *player, uint32_t logical_ticks);
void    update_player_direction(player_t *player, uint8_t scancode, bool is_make);
void    player_bomb_count(t_game_state *game);
void    update_player_death_animation(t_game_state *game, player_t *player);
void    update_player_win_animation(t_game_state *game, player_t *player);
void    update_player_lives(player_t *player, int change);

// Enemy helpers
void    enemy_init(t_game_state *game, enemy_t *enemy, t_tuple spawnpoint);
int     spawn_enemies_singleplayer(uint8_t *board, t_tuple player, int n, t_tuple out[MAX_ENEMIES]);
int     spawn_enemies_multiplayer(uint8_t *board, int n, t_tuple out[MAX_ENEMIES]);
int     spawnpoint_new_enemy_multiplayer(t_game_state *game, t_tuple *out);
bool    enemy_can_move(t_game_state *game, enemy_t *enemy, direction_t dir);
void    choose_enemy_direction(t_game_state *game, enemy_t *enemy);
void    update_enemy_movement(t_game_state *game, enemy_t *enemy);
void    update_enemy_animation(t_game_state *game, enemy_t *enemy, uint32_t logical_ticks);
void    update_enemy_lives(t_game_state *game, enemy_t *enemy, int change);

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
