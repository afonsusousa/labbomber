#include "game.h"
#include "application.h"
#include "board_generator.h"
#include "widget.h"
#include "gui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// FUNÇÕES QUE VÃO CONDUZIR O JOGO

// todo: tirar o "struct"
int game_state_init(t_game_state *game, uint32_t width, uint32_t height, t_time time) {
    if (game == NULL || width == 0 || height == 0) {
        return 1;
    }

    game_state_destroy(game);

    game->width = width;
    game->height = height;
    game->logical_ticks = 0;
    game->is_paused = false;

    generateBoard((char *)game->board, time.day, time.month, time.year);
    return 0;
}

void game_state_reset(t_game_state *game) {
    if (game == NULL) {
        return;
    }
    game->logical_ticks = 0;
    game->is_paused = false;
}

void game_state_destroy(t_game_state *game) {
    if (game == NULL) {
        return;
    }
    game->width = 0;
    game->height = 0;
}

void game_state_update(t_ctx *ctx) {
    (void)ctx;
    // update da logica do jogo AQUI
}

void game_state_handle_click(t_game_state *game, int32_t x, int32_t y) {
    // Clicks do jogo AQUI
}

void game_state_handle_key_press(t_game_state *gane, int8_t scancode) {
    // Keyboard
}

void draw_game_board(t_widget *self, hw_video_t *video, void *state) {
    if (self == NULL || state == NULL) {
        return;
    }

    t_ctx *ctx = (t_ctx*)state;

    // Get the absolute position of the widget
    int32_t start_x = self->abs_x;
    int32_t start_y = self->abs_y;

    // 1. Calculate the maximum square tile size that fits
    uint32_t max_tile_w = self->width / BOARD_COLS;
    uint32_t max_tile_h = self->height / BOARD_ROWS;
    uint32_t tile = (max_tile_w < max_tile_h) ? max_tile_w : max_tile_h;

    // 2. Calculate offsets to center the board within the widget
    uint32_t total_board_w = tile * BOARD_COLS;
    uint32_t total_board_h = tile * BOARD_ROWS;
    int32_t offset_x = (self->width - total_board_w) / 2;
    int32_t offset_y = (self->height - total_board_h) / 2;

    start_x += offset_x;
    start_y += offset_y;

    for (int y = 0; y < BOARD_ROWS; y++) {
        for (int x = 0; x < BOARD_COLS; x++) {

            int val = ctx->game.board[y * BOARD_COLS + x];

            int px = start_x + (x * tile);
            int py = start_y + (y * tile);

            if (val == 1) {
                hw_vbe_draw_rect(video, px, py, tile, tile, 0xAAAAAA);
            } else if (val == 2) {
                hw_vbe_draw_rect(video, px, py, tile, tile, 0x884400);
            } else {
                hw_vbe_draw_rect(video, px, py, tile, tile, 0x000000);
            }
        }
    }
}
