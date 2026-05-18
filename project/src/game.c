#include "game.h"
#include "application.h"
#include "board_generator.h"
#include "widget.h"
#include "gui.h"
#include "draw.h"
#include "../lib/keyboard/i8042.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BOARD_BASE_TILE_SIZE 16

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
    game->key_w = false;
    game->key_a = false;
    game->key_d = false;
    game->key_s = false;

    generateBoard((char *)game->board, time.day, time.month, time.year);
    set_date_seed(time.day, time.month, time.year);
    
    game->players[0].x = 50;
    game->players[0].y = 50;
    game->players[0].direction = PLAYER_STANDING;
    game->players[0].animation_phase = 0;
    game->players[0].is_moving = false;
    
    game->players[1].x = width - 100;
    game->players[1].y = height - 100;
    game->players[1].direction = PLAYER_STANDING;
    game->players[1].animation_phase = 0;
    game->players[1].is_moving = false;
    
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
    if (ctx == NULL) return;
    
    for (int i = 0; i < 2; i++) {
        player_t *player = &ctx->game.players[i];
        
        if (player->pause_counter > 0) {
            player->pause_counter--;
            player->is_moving = false;
        } else {
            int32_t start_x = 50, start_y = 50;
            update_player_movement(player, start_x, start_y);
        }
        update_player_animation(player, ctx->game.logical_ticks);
    }
}

void game_state_handle_click(t_game_state *game, int32_t x, int32_t y) {
    // Clicks do jogo AQUI
}

// Helper function to determine direction based on currently pressed keys
static player_direction_t get_current_direction(t_game_state *game) {
    // Check for diagonal combinations first
    if (game->key_w && game->key_a) {
        return PLAYER_BACK_LEFT;
    } else if (game->key_w && game->key_d) {
        return PLAYER_BACK_RIGHT;
    } else if (game->key_s && game->key_a) {
        return PLAYER_STANDING_LEFT;
    } else if (game->key_s && game->key_d) {
        return PLAYER_STANDING_RIGHT;
    }
    
    // Check for single directions
    if (game->key_w) {
        return PLAYER_BACK;
    } else if (game->key_a) {
        return PLAYER_LEFT;
    } else if (game->key_d) {
        return PLAYER_RIGHT;
    } else if (game->key_s) {
        return PLAYER_STANDING;
    }
    
    return PLAYER_STANDING;
}

void game_state_handle_key_press(t_game_state *game, int8_t scancode) {
    if (game == NULL)
        return;
    
    if (game->is_paused)
        return;
    
    player_t *player = &game->players[0];
    
    bool is_make = IS_MAKE_CODE(scancode);
    uint8_t key_index = MAKE_FROM_BREAK(scancode);
    
    if (is_make) {
        // Key press - update key state
        if (key_index == KEY_W) {
            game->key_w = true;
        } else if (key_index == KEY_A) {
            game->key_a = true;
        } else if (key_index == KEY_D) {
            game->key_d = true;
        } else if (key_index == KEY_S) {
            game->key_s = true;
        }
        player->is_moving = true;
    } else {
        // Key release - update key state
        if (key_index == KEY_W) {
            game->key_w = false;
        } else if (key_index == KEY_A) {
            game->key_a = false;
        } else if (key_index == KEY_D) {
            game->key_d = false;
        } else if (key_index == KEY_S) {
            game->key_s = false;
        }
        
        // Only stop moving if no movement keys are pressed
        if (!game->key_w && !game->key_a && 
            !game->key_d && !game->key_s) {
            player->is_moving = false;
        }
    }
    
    // Update direction based on current key state
    player->direction = get_current_direction(game);
}

void draw_game_board(t_widget *self, hw_video_t *video, void *state) {
    if (self == NULL || state == NULL) {
        return;
    }

    t_ctx *ctx = (t_ctx*)state;

    int32_t start_x = self->abs_x;
    int32_t start_y = self->abs_y;

    int32_t max_tile_w = (int32_t)self->width / BOARD_COLS;
    int32_t max_tile_h = (int32_t)self->height / BOARD_ROWS;
    int32_t tile = max_tile_w < max_tile_h ? max_tile_w : max_tile_h;
    tile = (tile / BOARD_BASE_TILE_SIZE) * BOARD_BASE_TILE_SIZE;
    if (tile < BOARD_BASE_TILE_SIZE) {
        tile = BOARD_BASE_TILE_SIZE;
    }

    const int32_t total_board_w = BOARD_COLS * tile;
    const int32_t total_board_h = BOARD_ROWS * tile;
    int32_t offset_x = ((int32_t)self->width - total_board_w) / 2;
    int32_t offset_y = ((int32_t)self->height - total_board_h) / 2;

    start_x += offset_x;
    start_y += offset_y;

    for (int y = 0; y < BOARD_ROWS; y++) {
        for (int x = 0; x < BOARD_COLS; x++) {

            int val = ctx->game.board[y * BOARD_COLS + x];

            int px = start_x + (x * tile);
            int py = start_y + (y * tile);

            if (val == 0) {
                int grass_type = decide_grass_sprite(ctx->game.board, BOARD_ROWS, BOARD_COLS, x, y);
                draw_grass(video, px, py, grass_type, tile);
            } else if (val == 1) {
                int wall_sprite = decide_wall_sprite(ctx->game.board, BOARD_ROWS, BOARD_COLS, x, y);
                draw_wall(video, px, py, wall_sprite, tile);
            } else if (val == 2) {
                // Brick
                draw_brick(video, px, py, tile);
            } else {
                // Fallback for unknown values
                hw_vbe_draw_rect(video, px, py, tile, tile, 0x000000);
            }
        }
    }
    draw_player(ctx->game.players, video, tile);
}
