#ifndef LCOM_PROJECT_GAME_H
#define LCOM_PROJECT_GAME_H

#include <stdint.h>
#include <stdbool.h>

#include "gui.h"

//mudar tudo para acomodar o jogo
typedef struct s_game_state {
    uint16_t *pixel_buffer;
    uint32_t width;
    uint32_t height;
} t_game_state;

void init_game(t_gui *gui);
void gui_reset_game(t_gui *gui);

#endif /* LCOM_PROJECT_GAME_H */
