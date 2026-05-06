#include "game.h"
#include "application.h"
#include "widget.h"
#include "gui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// FUNÇÕES QUE VÃO CONDUZIR O JOGO
int game_state_init(t_game_state *game, uint32_t width, uint32_t height) {
    if (game == NULL || width == 0 || height == 0) {
        return 1;
    }

    game_state_destroy(game);
    game->pixel_buffer = (uint16_t*)calloc((size_t)width * height, sizeof(uint16_t));
    if (game->pixel_buffer == NULL) {
        game->width = 0;
        game->height = 0;
        return 1;
    }

    game->width = width;
    game->height = height;
    game->logical_ticks = 0;
    game->is_paused = false;
    return 0;
}

void game_state_reset(t_game_state *game) {
    if (game == NULL || game->pixel_buffer == NULL) {
        return;
    }

    memset(game->pixel_buffer, 0, sizeof(uint16_t) * game->width * game->height);
    game->logical_ticks = 0;
    game->is_paused = false;
}

void game_state_destroy(t_game_state *game) {
    if (game == NULL) {
        return;
    }

    free(game->pixel_buffer);
    game->pixel_buffer = NULL;
    game->width = 0;
    game->height = 0;
}

void game_state_update(t_ctx *ctx) {
    (void)ctx;
}
