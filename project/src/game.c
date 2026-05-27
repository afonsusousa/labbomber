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
    //game->click_count = 0; tem de estar depois escolher as coords do player
    game->debug_mode = false;
    generateBoard((char *)game->board, time.day, time.month, time.year);
    set_date_seed(time.day, time.month, time.year);

   // --- PLAYER 1 ---
   t_tuple spawnpoint = spawnpoint_generator(game->board, game->click_count);

   game->click_count = 0; // aqui

    game->players[PLAYER_1].pos = (t_tuple) {
        (spawnpoint.x * game->tile_size) + (game->tile_size / 2),
        (spawnpoint.y * game->tile_size) + (game->tile_size / 2)
    };

    game->players[PLAYER_1].board_pos = spawnpoint;
    game->players[PLAYER_1].sprite_dir = DIR_DOWN;
    game->players[PLAYER_1].animation_phase = 0;
    game->players[PLAYER_1].is_moving = false;
    game->players[PLAYER_1].stack_count = 0;

    // --- PLAYER 2 ---
    game->players[PLAYER_2].pos = (t_tuple) {0, 0};
    game->players[PLAYER_2].sprite_dir = DIR_DOWN;
    game->players[PLAYER_2].animation_phase = 0;
    game->players[PLAYER_2].is_moving = false;
    game->players[PLAYER_2].stack_count = 0;

    // --- ENEMIES ---
    t_tuple spawn_out[MAX_ENEMIES];
    game->enemy_count = spawn_enemies(game->board, game->players[PLAYER_1].board_pos, 4, spawn_out);

    for (int i = 0; i < game->enemy_count; i++) {

        enemy_t *enemy = &game->enemies[i];

        enemy->pos.x = (spawn_out[i].x * game->tile_size) + (game->tile_size / 2);
        enemy->pos.y = (spawn_out[i].y * game->tile_size) + (game->tile_size / 2);

        enemy->board_pos.x = spawn_out[i].x;
        enemy->board_pos.y = spawn_out[i].y;

        enemy->active = true;
        enemy->is_moving = true;

        direction_t valid_dirs[4];

        int count = get_valid_directions(game->board, enemy->board_pos, valid_dirs);

        if (count > 0) {
            direction_t dir = valid_dirs[rand() % count];
            enemy->dir = dir;
            enemy->sprite_dir = dir;
        }

        else {
            enemy->dir = DIR_UP;
            enemy->sprite_dir = DIR_UP;
            enemy->is_moving = false;
        }

        enemy->animation_phase = 0;
    }

    bomb_init(&game->bomb);
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

    // Cache player sizes and animation offsets
    uint32_t pw = (tile * 8) / 12;
    uint32_t ph = pw;

    if (sprites_initialized && sprite_cache[SPRITE_PLAYER_1_STANDING].bytes != NULL) {
        uint32_t img_w = sprite_cache[SPRITE_PLAYER_1_STANDING].width;
        uint32_t img_h = sprite_cache[SPRITE_PLAYER_1_STANDING].height;
        ph = (img_h * pw) / img_w;
    }

    game->player_w = pw;
    game->player_h = ph;

    _game_state_prepare_match(game, time);

    scale_all_game_sprites(game->tile_size, pw, ph, MAX_PLAYERS);

    return 0;
}

void game_state_reset(t_game_state *game, t_time time)
{
    if (game == NULL) return;
    _game_state_prepare_match(game, time);
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

    if (player_collides_with_enemy(&ctx->game, &ctx->game.players[0])) {
        ctx->game.debug_mode = true;
    } else {
        ctx->game.debug_mode = false;
    }

    for (int i = 0; i < MAX_PLAYERS; i++) {
        player_t *player = &ctx->game.players[i];

        update_player_movement(&ctx->game, player);
        update_player_animation(player, ctx->game.logical_ticks);
    }

    for (int i = 0; i < ctx->game.enemy_count; i++) {
        enemy_t *enemy = &ctx->game.enemies[i];

        update_enemy_movement(&ctx->game, enemy);
        update_enemy_animation(enemy, ctx->game.logical_ticks);
    }

    bomb_update(&ctx->game);
}

void game_state_handle_click(t_game_state *game, int32_t x, int32_t y)
{
    // Clicks do jogo AQUI
}

void game_state_handle_key_press(t_game_state *game, uint8_t scancode) {
    if (game == NULL || game->is_paused) return;

    player_t *player = &game->players[PLAYER_1];
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

    t_game_state *game = GAME(state);

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
            } else {
                hw_vbe_draw_rect(video, GET_X(game, x) - (game->tile_size / 2), GET_Y(game, y) - (game->tile_size / 2), game->tile_size, game->tile_size, 0x000000);
            }
        }
    }

    draw_bomb(video, game);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        draw_player(&game->players[i], video, game);
    }

    for (int i = 0; i < game->enemy_count; i++) {
        draw_enemy(&game->enemies[i], video, game->start_x, game->start_y);
    }
}
