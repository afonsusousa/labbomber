#ifndef LCOM_PROJECT_GAME_H
#define LCOM_PROJECT_GAME_H

#include <stdint.h>
#include <stdbool.h>
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

typedef struct {
    t_tuple pos;
    uint8_t animation_phase;
} enemy_t;

typedef struct {
    bool active;
    uint8_t state;
    t_tuple board_pos;
    uint32_t bomb_timer;
} bomb_t;

struct s_ctx;
struct s_time;

// Assuming these macros are defined in your game.h
#ifndef BOARD_ROWS
#define BOARD_ROWS 11
#endif

#ifndef BOARD_COLS
#define BOARD_COLS 17
#endif

#define GAME_TICKS_PER_SECOND 60
#define BOMB_DURATION_SECONDS 3
#define BOMB_DURATION_TICKS (BOMB_DURATION_SECONDS * GAME_TICKS_PER_SECOND)
#define BOMB_BLINK_TICKS (2 * GAME_TICKS_PER_SECOND)
#define BOMB_EXPLODE_TICKS GAME_TICKS_PER_SECOND

#define BOMB_INACTIVE 0
#define BOMB_PLACED 1
#define BOMB_BLINK 2
#define BOMB_EXPLODE 3

typedef struct s_game_state {

    uint32_t width;
    uint32_t height;

    uint32_t tile_size;

    uint8_t board[BOARD_ROWS * BOARD_COLS];

    player_t players[2];

    enemy_t enemy;
    
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

// Enemy helpers
void update_enemy_animation(enemy_t *enemy, uint32_t logical_ticks);

// Bomb helpers
void    bomb_init(bomb_t *bomb);
void    bomb_reset(bomb_t *bomb);
void    bomb_update(bomb_t *bomb);
void    place_player_bomb(t_game_state *game, const player_t *player);

#endif /* LCOM_PROJECT_GAME_H */
