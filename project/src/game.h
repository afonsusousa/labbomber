#ifndef LCOM_PROJECT_GAME_H
#define LCOM_PROJECT_GAME_H

#include <stdint.h>
#include <stdbool.h>

#include "state.h"
typedef struct s_game_state {
    uint16_t *pixel_buffer;
    uint32_t width;
    uint32_t height;
} t_game_state;

void init_game(t_state *gui);
void gui_reset_game(t_state *gui);

#endif /* LCOM_PROJECT_GAME_H */
