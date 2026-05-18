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

typedef struct {
    int32_t x;
    int32_t y;
    player_direction_t direction;
    uint8_t animation_phase; // 0-3 for the 4 directional sprites
    bool is_moving;
    uint8_t pause_counter;   // TEMP:     frames to pause at edges
} player_t;

struct s_ctx;
struct s_time;

// Assuming these macros are defined in your game.h
#ifndef BOARD_ROWS
#define BOARD_ROWS 11
#endif

#ifndef BOARD_COLS
#define BOARD_COLS 17
#endif

typedef struct s_game_state {
    
    uint32_t width;
    uint32_t height;
    
    uint8_t board[BOARD_ROWS * BOARD_COLS];

    player_t players[2];

    uint32_t logical_ticks;
    bool is_paused;
    
    bool key_w;
    bool key_a;
    bool key_d;
    bool key_s;
} t_game_state;

int     game_state_init(t_game_state *game, uint32_t width, uint32_t height, struct s_time time);
void    game_state_reset(t_game_state *game);
void    game_state_destroy(t_game_state *game);
void    game_state_update(struct s_ctx *ctx);
void    game_state_handle_click(t_game_state *game, int32_t x, int32_t y);
void    game_state_handle_key_press(t_game_state *gane, int8_t scancode);

void    gui_show_game_view(struct s_ctx *ctx);
void    gui_reset_game_view(struct s_ctx *ctx);

#endif /* LCOM_PROJECT_GAME_H */
