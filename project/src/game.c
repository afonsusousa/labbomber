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

static void _game_state_prepare_match(t_game_state *game, t_time time) {
    game->logical_ticks = 0;
    game->is_paused = false;
    generateBoard((char *)game->board, time.day, time.month, time.year);

    game->door_pos = door_spawnpoint_generator(game->board, game->click_count, time.day, time.month, time.year);
    game->door_active = false;
    game->door_open = false;

    set_date_seed(time.day, time.month, time.year);

    // --- PLAYER 1 ---
    t_tuple spawnpoint = spawnpoint_generator(game->board, game->click_count);
    player_init(game, &game->players[PLAYER_1], spawnpoint);
    game->players[PLAYER_1].alive = true;
    game->players[PLAYER_1].lives = 1;
    game->current_player = PLAYER_1;

    // --- PLAYER 2 ---
    player_init(game, &game->players[PLAYER_2], (t_tuple){0, 0});
    game->players[PLAYER_2].lives = 3;
    game->players[PLAYER_2].active = false; // Only one player for now

    // --- ENEMIES ---
    t_tuple spawn_out[MAX_ENEMIES];
    game->enemy_count = spawn_enemies(game->board, game->players[PLAYER_1].board_pos, 4, spawn_out);

    for (int i = 0; i < game->enemy_count; i++) {
        enemy_init(game, &game->enemies[i], spawn_out[i]);
    }

    for (int i = 0; i < MAX_BOMBS; i++) {
        bomb_init(&game->bomb[i]);
    }
    
    game->click_count = 0;
}

int game_state_init(t_game_state *game, uint32_t width, uint32_t height, t_time time) {
    if (game == NULL || width == 0 || height == 0) return 1;

    game_state_destroy(game);

    game->width = width;
    game->height = height;

    // Cache pre-computed tile size and map draw offsets
    int32_t max_tile_w = (int32_t)width / BOARD_COLS;
    int32_t max_tile_h = (int32_t)height / BOARD_ROWS;
    int32_t tile = max_tile_w < max_tile_h ? max_tile_w : max_tile_h;
    tile = (tile / BOARD_BASE_TILE_SIZE) * BOARD_BASE_TILE_SIZE;
    if (tile < BOARD_BASE_TILE_SIZE)
        tile = BOARD_BASE_TILE_SIZE;

    game->tile_size = tile;

    int32_t board_width = BOARD_COLS * tile;
    int32_t board_height = BOARD_ROWS * tile;
    game->start_x = (width - board_width) / 2;
    game->start_y = (height - board_height) / 2;

    _game_state_prepare_match(game, time);
    game->score = 0;

    scale_all_game_sprites(game->tile_size, game->players[PLAYER_1].size.x, game->players[PLAYER_1].size.y, MAX_PLAYERS);

    return 0;
}

void game_state_reset(t_game_state *game, t_time time)
{
    if (game == NULL) return;
    _game_state_prepare_match(game, time);
    game->score = 0;
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

    t_game_state *game = &ctx->game;

    if (game->is_paused) return;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        player_t *player = &game->players[i];
        if (!player->active) continue;

        if (player_collides_with_enemy(game, player)) {
            update_player_lives(player, -1);
        }
    }

    for (int i = 0; i < MAX_PLAYERS; i++) {
        player_t *player = &game->players[i];
        if (!player->active) continue;

        update_player_movement(game, player);
        update_player_animation(player, game->logical_ticks);
    }

    for (int i = 0; i < game->enemy_count; i++) {
        enemy_t *enemy = &game->enemies[i];
        if (!enemy->active) continue;

        update_enemy_movement(game, enemy);
        update_enemy_animation(enemy, game->logical_ticks);
    }

    for (int i = 0; i < MAX_BOMBS; i++) {
        bomb_update(game, &game->bomb[i]);
    }
    player_bomb_count(game);

    int enemies_alive = 0;
    for (int i = 0; i < game->enemy_count; i++) {
        if (game->enemies[i].active) enemies_alive++;
    }
    if (game->enemy_count > 0 && enemies_alive == 0 && game->door_active) {
        game->door_open = true;
    }

    if (!game->players[PLAYER_1].alive && !game->is_paused) {
        game->is_paused = true;
        game->players[PLAYER_1].invincibility_timer = GAME_TICKS_PER_SECOND * 2;
    }
}

void game_state_handle_click(t_game_state *game, int32_t x, int32_t y) {
    (void)game; (void)x; (void)y;
}

#define KEY_E 0x12

void game_state_handle_key_press(t_game_state *game, uint8_t scancode) {
    if (game == NULL) return;

    bool is_make = !(scancode & 0x80);
    uint8_t key_index = scancode & 0x7F;

    if (game->current_player >= MAX_PLAYERS) return;
    player_t *player = &game->players[game->current_player];

    if (is_make && key_index == KEY_E) {
        place_player_bomb(game, player);
    }

    update_player_direction(player, key_index, is_make);
}

void draw_player_hearts(hw_video_t *video, t_game_state *game) {
    if (game == NULL || !sprites_initialized) return;

    const int heart_size = 24;
    const int heart_spacing = 4;
    scale_cached_sprite(SPRITE_HEART, heart_size, heart_size, 2);
    xpm_image_t heart = scaled_sprite_cache[SPRITE_HEART];

    if (game->players[PLAYER_1].active) {
        for (int i = 0; i < game->players[PLAYER_1].lives && i < 5; i++) {
            int32_t hx = 20 + i * (heart_size + heart_spacing);
            int32_t hy = 20;
            hw_vbe_draw_xpm(video, heart.bytes, heart, hx + heart_size/2, hy + heart_size/2);
        }
    }

    if (game->players[PLAYER_2].active) {
        uint32_t screen_w = video->screen_width;
        for (int i = 0; i < game->players[PLAYER_2].lives && i < 5; i++) {
            int32_t hx = screen_w - 20 - heart_size - i * (heart_size + heart_spacing);
            int32_t hy = 20;
            hw_vbe_draw_xpm(video, heart.bytes, heart, hx + heart_size/2, hy + heart_size/2);
        }
    }
}

void draw_game_board(t_widget *self, hw_video_t *video, void *state) {
    if (self == NULL || state == NULL) {
        return;
    }

    t_game_state *game = GAME(state);
    
    // Draw Map
    for (int y = 0; y < BOARD_ROWS; y++) {
        for (int x = 0; x < BOARD_COLS; x++) {
            int val = game->board[y * BOARD_COLS + x];
            if (val == TILE_TYPE_GRASS) {
                int grass_type = decide_grass_sprite(game->board, BOARD_ROWS, BOARD_COLS, x, y);
                draw_grass(video, GET_X(game, x), GET_Y(game, y), grass_type);
            } else if (val == TILE_TYPE_WALL) {
                int wall_sprite = decide_wall_sprite(game->board, BOARD_ROWS, BOARD_COLS, x, y);
                draw_wall(video, GET_X(game, x), GET_Y(game, y), wall_sprite);
            } else if (val == TILE_TYPE_BRICK) {
                draw_brick(video, GET_X(game, x), GET_Y(game, y));
            } else if (val == TILE_TYPE_DOOR) {
                draw_door(video, GET_X(game, x), GET_Y(game, y), game->door_open);
            } else {
                hw_vbe_draw_rect(video, GET_X(game, x) - (game->tile_size / 2), GET_Y(game, y) - (game->tile_size / 2), game->tile_size, game->tile_size, 0x000000);
            }
        }
    }

    draw_bomb(video, game);

    for (int i = 0; i < game->enemy_count; i++) {
        draw_enemy(&game->enemies[i], video, game->start_x, game->start_y);
    }

    for (int i = 0; i < MAX_PLAYERS; i++) {
        draw_player(&game->players[i], video, game);
    }
    draw_player_hearts(video, game);
}
