#include "game.h"
#include "application.h"
#include "board_generator.h"
#include "widget.h"
#include "gui.h"
#include "draw.h"
#include "assets_cache.h"
#include "../lib/keyboard/i8042.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BOARD_BASE_TILE_SIZE 16

// FUNÇÕES QUE VÃO CONDUZIR O JOGO

// todo: tirar o "struct"
int game_state_init(t_game_state *game, uint32_t width, uint32_t height, t_time time) {
    if (game == NULL || width == 0 || height == 0) return 1;

    game_state_destroy(game);

    game->width = width;
    game->height = height;
    game->logical_ticks = 0;
    game->is_paused = false;

    // Cache pre-computed tile size and map draw offsets
    int32_t max_tile_w = (int32_t)width / BOARD_COLS;
    int32_t max_tile_h = (int32_t)height / BOARD_ROWS;
    int32_t tile = max_tile_w < max_tile_h ? max_tile_w : max_tile_h;
    tile = (tile / BOARD_BASE_TILE_SIZE) * BOARD_BASE_TILE_SIZE;
    if (tile < BOARD_BASE_TILE_SIZE)
        tile = BOARD_BASE_TILE_SIZE;

    game->tile_size = tile;

    generateBoard((char *)game->board, time.day, time.month, time.year);
    set_date_seed(time.day, time.month, time.year);

    // Cache player sizes and animation offsets
    uint32_t pw = (tile * 8) / 12;
    uint32_t ph = pw;

    if (sprites_initialized && sprite_cache[SPRITE_PLAYER_1_STANDING].bytes != NULL) {
        uint32_t img_w = sprite_cache[SPRITE_PLAYER_1_STANDING].width;
        uint32_t img_h = sprite_cache[SPRITE_PLAYER_1_STANDING].height;
        ph = (img_h * pw) / img_w;
    }

   // --- PLAYER 1 ---

    game->players[0].pos = (t_tuple) {(game->tile_size) + (game->tile_size / 2), (game->tile_size) + (game->tile_size  / 2)};
    game->players[0].board_pos = (t_tuple) {1, 1};
    game->players[0].sprite_dir = PLAYER_STANDING;
    game->players[0].animation_phase = 0;
    game->players[0].is_moving = false;
    game->players[0].stack_count = 0;

    game->players[1].pos = (t_tuple) {0, 0};
    game->players[1].sprite_dir = PLAYER_STANDING;
    game->players[1].animation_phase = 0;
    game->players[1].is_moving = false;
    game->players[1].stack_count = 0; 

    game->bomb.active = false;
    game->bomb.board_pos = (t_tuple) {0, 0};
    game->bomb.placed_tick = 0;

    scale_all_game_sprites(game->tile_size, pw, ph, 2);

    return 0;
}

void game_state_reset(t_game_state *game)
{
    if (game == NULL) return;

    game->logical_ticks = 0;
    game->is_paused = false;
    game->bomb.active = false;
}

void game_state_destroy(t_game_state *game) {
    if (game == NULL) return;

    game->width = 0;
    game->height = 0;
    game->tile_size = 0;
    game->is_paused = true;
}

void game_state_update(t_ctx *ctx) {
    if (ctx == NULL) return;
    
    for (int i = 0; i < 2; i++) {
        player_t *player = &ctx->game.players[i];
        
        update_player_movement(&ctx->game, player);
        update_player_animation(player, ctx->game.logical_ticks);
    }

    if (ctx->game.bomb.active && ctx->game.logical_ticks - ctx->game.bomb.placed_tick >= BOMB_DURATION_TICKS) {
        ctx->game.bomb.active = false;
    }
}

void game_state_handle_click(t_game_state *game, int32_t x, int32_t y)
{
    // Clicks do jogo AQUI
}

void game_state_handle_key_press(t_game_state *game, uint8_t scancode) {
    if (game == NULL || game->is_paused) return;

    player_t *player = &game->players[0];
    bool is_make = IS_MAKE_CODE(scancode);
    uint8_t key_index = MAKE_FROM_BREAK(scancode);

    if (is_make && key_index == KEY_E) {
        place_player_bomb(game, player);
    }

    update_player_direction(player, key_index, is_make);
}

void draw_game_board(t_widget *self, hw_video_t *video, void *state) {
    if (self == NULL || state == NULL) {
        return;
    }

    t_ctx *ctx = (t_ctx *)state;

    int32_t tile = ctx->game.tile_size;
    int32_t board_width = BOARD_COLS * tile;
    int32_t board_height = BOARD_ROWS * tile;
    int32_t start_x = self->abs_x + ((int32_t)self->width - board_width) / 2;
    int32_t start_y = self->abs_y + ((int32_t)self->height - board_height) / 2;

    for (int y = 0; y < BOARD_ROWS; y++) {
        for (int x = 0; x < BOARD_COLS; x++) {

            int val = ctx->game.board[y * BOARD_COLS + x];

            // Get center coordinate of the tile
            int px = start_x + (x * tile) + (tile / 2);
            int py = start_y + (y * tile) + (tile / 2);

            if (val == 0) {
                int grass_type = decide_grass_sprite(ctx->game.board, BOARD_ROWS, BOARD_COLS, x, y);
                draw_grass(video, px, py, grass_type);
            } else if (val == 1) {
                int wall_sprite = decide_wall_sprite(ctx->game.board, BOARD_ROWS, BOARD_COLS, x, y);
                draw_wall(video, px, py, wall_sprite);
            } else if (val == 2) {
                // Brick
                draw_brick(video, px, py);
            } else {
                // Fallback for unknown values
                hw_vbe_draw_rect(video, px - (tile / 2), py - (tile / 2), tile, tile, 0x000000);
            }
        }
    }
    draw_bomb(&ctx->game.bomb, ctx->game.logical_ticks, video, start_x, start_y, ctx->game.tile_size);

    for (int i = 0; i < 2; i++) {
        draw_player(&ctx->game.players[i], video, start_x, start_y);
    }
}
