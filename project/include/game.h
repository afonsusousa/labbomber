#ifndef LCOM_PROJECT_GAME_H
#define LCOM_PROJECT_GAME_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    PLAYER_STANDING = 0,
    PLAYER_LEFT,
    PLAYER_RIGHT,
    PLAYER_BACK
} player_direction_t;

typedef struct s_tuple {
    int32_t x;
    int32_t y;
} t_tuple;

typedef struct {
    t_tuple pos;
    t_tuple board_pos;
    player_direction_t dir;

    player_direction_t sprite_dir;
    uint8_t animation_phase; // 0-3 for the 4 directional sprites
    bool is_moving;

    uint8_t movement_stack[4];
    uint8_t stack_count;
} player_t;

#define BOMB_EXPLOSION_RANGE 2

typedef struct {
    bool active;
    uint8_t state;
    t_tuple board_pos;
    uint32_t bomb_timer;
    uint32_t explosion_timer;
    uint8_t explosion_current_radius;
    uint16_t explosion_blocked_steps;
} bomb_t;

struct s_ctx;
struct s_time;

#ifndef BOARD_ROWS
#define BOARD_ROWS 11
#endif

#ifndef BOARD_COLS
#define BOARD_COLS 17
#endif

#define GAME_TICKS_PER_SECOND 60
#define BOMB_DURATION_TICKS ((7 * GAME_TICKS_PER_SECOND) / 2)
#define BOMB_EXPLOSION_DURATION_TICKS (GAME_TICKS_PER_SECOND / 2)
#define BOMB_FUSE_PHASES 8

#define BOMB_INACTIVE 0
#define BOMB_PLACED 1
#define BOMB_BLINK 2
#define BOMB_EXPLODE 3
#define BOMB_FIRE 4

#define MAX_PLAYERS 2
#define PLAYER_1 0
#define PLAYER_2 1

#define TILE_TYPE_GRASS 0
#define TILE_TYPE_WALL 1
#define TILE_TYPE_BRICK 2

#define GET_X(game, value) ((game)->start_x + (value) * (game)->tile_size + (game)->tile_size / 2)
#define GET_Y(game, value) ((game)->start_y + (value) * (game)->tile_size + (game)->tile_size / 2)

typedef struct s_game_state {

    uint32_t width;
    uint32_t height;

    uint32_t tile_size;

    int32_t start_x;
    int32_t start_y;

    uint8_t board[BOARD_ROWS * BOARD_COLS];

    player_t players[MAX_PLAYERS];
    bomb_t bomb;

    uint32_t logical_ticks;
    bool is_paused;

    uint32_t click_count;

} t_game_state;

int     game_state_init(t_game_state *game, uint32_t width, uint32_t height, struct s_time time);
void    game_state_reset(t_game_state *game);
void    game_state_destroy(t_game_state *game);
void    game_state_update(struct s_ctx *ctx);
void    game_state_handle_click(t_game_state *game, int32_t x, int32_t y);
void    game_state_handle_key_press(t_game_state *gane, uint8_t scancode);

void    gui_show_game_view(struct s_ctx *ctx);
void    gui_reset_game_view(struct s_ctx *ctx);

// Player movement helpers
void    get_player_start_position(int player_id, uint32_t width, uint32_t height, int32_t *out_x, int32_t *out_y);
void    update_player_movement(t_game_state *game, player_t *player);
void    update_player_animation(player_t *player, uint32_t logical_ticks);
void    update_player_direction(player_t *player, uint8_t scancode, bool is_make);

// Bomb helpers
void    bomb_init(bomb_t *bomb);
void    bomb_reset(bomb_t *bomb);
void    bomb_clear_explosion(bomb_t *bomb);
void    bomb_update(t_game_state *game);
void    bomb_begin_explosion(t_game_state *game);
void    bomb_update_explosion(t_game_state *game);
void    place_player_bomb(t_game_state *game, const player_t *player);

bool explosion_collides(const t_game_state *game, int32_t cell_x, int32_t cell_y);

#endif /* LCOM_PROJECT_GAME_H */
