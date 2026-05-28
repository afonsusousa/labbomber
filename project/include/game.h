#ifndef LCOM_PROJECT_GAME_H
#define LCOM_PROJECT_GAME_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "../lib/vbe/vbe.h"

struct s_ctx;
struct s_time;
struct s_game_state;

typedef enum {
    DIR_DOWN = 0,
    DIR_LEFT = 1,
    DIR_RIGHT = 2,
    DIR_UP = 3
} direction_t;

#ifndef BOARD_ROWS
#define BOARD_ROWS 11
#endif

#ifndef BOARD_COLS
#define BOARD_COLS 17
#endif

#define TOTAL_CELLS     (BOARD_ROWS * BOARD_COLS)
#define BOARD_IDX(x, y) ((y) * BOARD_COLS + (x))

#define TILE_TYPE_GRASS 0
#define TILE_TYPE_WALL  1
#define TILE_TYPE_BRICK 2

#define GAME_TICKS_PER_SECOND 60

#define MAX_PLAYERS 2
#define PLAYER_1    0
#define PLAYER_2    1

#define MAX_ENEMIES          10
#define MIN_DIST_FROM_PLAYER  6
#define ENEMY_SPEED           2

#define BOMB_EXPLOSION_RANGE 2

#define BOMB_DURATION_TICKS           ((7 * GAME_TICKS_PER_SECOND) / 2)
#define BOMB_EXPLOSION_DURATION_TICKS (GAME_TICKS_PER_SECOND / 2)
#define BOMB_FUSE_PHASES              8

#define BOMB_INACTIVE 0
#define BOMB_PLACED   1
#define BOMB_BLINK    2
#define BOMB_EXPLODE  3
#define BOMB_FIRE     4

#define GET_X(game, value) ((game)->start_x + (value) * (game)->tile_size + (game)->tile_size / 2)
#define GET_Y(game, value) ((game)->start_y + (value) * (game)->tile_size + (game)->tile_size / 2)

typedef struct s_tuple {
    int32_t x;
    int32_t y;
} t_tuple;

typedef struct s_entity {
    t_tuple     pos;
    t_tuple     board_pos;
    direction_t dir;
    direction_t sprite_dir;
    uint8_t     animation_phase;
    bool        is_moving;
    bool        active;
    uint8_t     movement_stack[4];
    uint8_t     stack_count;
    uint8_t     speed;
    uint32_t    w;
    uint32_t    h;
    uint8_t     lives;
    void (*on_snap)(struct s_game_state *game, struct s_entity *entity);
} entity_t;

typedef entity_t player_t;
typedef entity_t enemy_t;

enum {
    EXPLOSION_DIR_RIGHT = 0,
    EXPLOSION_DIR_LEFT  = 1,
    EXPLOSION_DIR_DOWN  = 2,
    EXPLOSION_DIR_UP    = 3,
    EXPLOSION_DIR_COUNT = 4,
};

typedef struct {
    bool active;
    uint8_t state;
    t_tuple board_pos;
    uint32_t bomb_timer;
    uint32_t explosion_timer;
    uint8_t radius;
    uint8_t reach[EXPLOSION_DIR_COUNT];
} bomb_t;

typedef struct s_game_state {
    uint32_t width;
    uint32_t height;
    uint32_t tile_size;
    int32_t start_x;
    int32_t start_y;

    uint8_t board[BOARD_ROWS * BOARD_COLS];

    player_t players[MAX_PLAYERS];

    enemy_t enemies[MAX_ENEMIES];
    uint8_t enemy_count;

    bomb_t bomb;

    uint32_t logical_ticks;
    uint32_t click_count;
    bool debug_mode;
    bool is_paused;
} t_game_state;

int     game_state_init(t_game_state *game, uint32_t width, uint32_t height, struct s_time time);
void    game_state_reset(t_game_state *game, struct s_time time);
void    game_state_destroy(t_game_state *game);
void    game_state_update(struct s_ctx *ctx);
void    game_state_handle_click(t_game_state *game, int32_t x, int32_t y);
void    game_state_handle_key_press(t_game_state *game, uint8_t scancode);

void    gui_show_game_view(struct s_ctx *ctx);
void    gui_reset_game_view(struct s_ctx *ctx);

// Entity helpers
bool    collision(uint8_t *board, t_tuple pos);
direction_t opposite_dir(direction_t dir);
int     get_valid_directions(uint8_t *board, t_tuple pos, direction_t out[4]);
bool    entity_overlaps(const entity_t *a, const entity_t *b);
bool    player_collides_with_enemy(const t_game_state *game, const player_t *player);


// Player movement helpers
t_tuple spawnpoint_generator(uint8_t *board, uint32_t click_count);
void    get_player_start_position(int player_id, uint32_t width, uint32_t height, int32_t *out_x, int32_t *out_y);
void    player_init(t_game_state *game, player_t *player, t_tuple spawnpoint);
void    update_player_movement(t_game_state *game, player_t *player);
void    update_player_animation(player_t *player, uint32_t logical_ticks);
void    update_player_direction(player_t *player, uint8_t scancode, bool is_make);

/**
 * get_entering_cell - return the adjacent board cell an entity may overlap
 * when moving.
 */
t_tuple get_entering_cell(const t_game_state *game, const entity_t *entity);

/**
 * get_board_pos - compute a continuous/smoothed board cell for an entity.
 *
 * Uses a small hardcoded threshold (offset = tile / 10) to bias rounding
 * toward the movement direction.
 */
t_tuple get_board_pos(const t_game_state *game, const entity_t *entity);
void update_entity_movement(t_game_state *game, entity_t *entity);

// Enemy helpers
void    enemy_init(t_game_state *game, enemy_t *enemy, t_tuple spawnpoint);
int     spawn_enemies(uint8_t *board, t_tuple player, int n, t_tuple out[MAX_ENEMIES]); //geração determinística
bool    enemy_can_move(t_game_state *game, enemy_t *enemy, direction_t dir);
void    choose_enemy_direction(t_game_state *game, enemy_t *enemy);
void    update_enemy_movement(t_game_state *game, enemy_t *enemy);
void    update_enemy_animation(enemy_t *enemy, uint32_t logical_ticks);

// Bomb helpers
void    bomb_init(bomb_t *bomb);
void    bomb_reset(bomb_t *bomb);
void    bomb_clear_explosion(bomb_t *bomb);
void    bomb_update(t_game_state *game);
void    bomb_begin_explosion(t_game_state *game);
void    bomb_update_explosion(t_game_state *game);
void    place_player_bomb(t_game_state *game, const player_t *player);
bool    explosion_collides(const t_game_state *game, int32_t cell_x, int32_t cell_y);

#endif /* LCOM_PROJECT_GAME_H */
