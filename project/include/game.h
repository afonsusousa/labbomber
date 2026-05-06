#ifndef LCOM_PROJECT_GAME_H
#define LCOM_PROJECT_GAME_H

#include <stdint.h>
#include <stdbool.h>

struct s_ctx;

typedef struct s_game_state {
    uint16_t *pixel_buffer;
    uint32_t width;
    uint32_t height;
    
    // Simulation state belongs to the game
    uint32_t logical_ticks;
    bool is_paused;
} t_game_state;

int     game_state_init(t_game_state *game, uint32_t width, uint32_t height);
void    game_state_reset(t_game_state *game);
void    game_state_destroy(t_game_state *game);
void    game_state_update(struct s_ctx *ctx);
void    gui_show_game_view(struct s_ctx *ctx);
void    gui_reset_game_view(struct s_ctx *ctx);

#endif /* LCOM_PROJECT_GAME_H */
