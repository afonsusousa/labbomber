#ifndef LCOM_PROJECT_GAME_H
#define LCOM_PROJECT_GAME_H

#include <stdint.h>
#include <stdbool.h>

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

    // Simulation state belongs to the game
    uint32_t logical_ticks;
    bool is_paused;
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
