#include "game/game.h"
#include "core/application.h"
#include "game/board_generator.h"
#include "game/entity_controller.h"
#include "game/player_controller.h"
#include "game/enemy_controller.h"
#include "game/bomb_controller.h"
#include "gui/widget.h"
#include "gui/gui.h"
#include "view/game/draw_game.h"
#include "view/assets_cache.h"
#include "i8042.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BOARD_BASE_TILE_SIZE 16

// FUNÇÕES QUE VÃO CONDUZIR O JOGO

static void _game_state_prepare_match(t_game_state *game, t_time time, bool is_multiplayer) {
    game->logical_ticks = 0;
    game->match_state = MATCH_RUNNING;
    game->enemies_to_kill = is_multiplayer ? 600 : 400;
    game->last_enemy_spawn_ticks = 0;
    game->is_frozen = false;
    game->players[PLAYER_1].invincibility_timer = 0;
    game->players[PLAYER_2].invincibility_timer = 0;
    game->animation_timer = 0;
    game->multiplayer_last_contact_ticks = 0;

    unsigned int first_seed = time.year * 10000 + time.month * 100 + time.day;
    srand(first_seed);

    generateBoard((char *)game->board);

    uint32_t second_seed;

    if (is_multiplayer) {
        second_seed = game->enemy_seed;
    } else {
        uint32_t total_seconds = time.hours * 3600 + time.minutes * 60 + time.seconds;

        second_seed = time.year * 1000000 + time.month * 10000 + time.day * 100 + total_seconds / 10;
    }

    srand(second_seed);

    game->door_pos = door_spawnpoint_generator(game->board);
    game->door_open = false;

    // RESET PLAYERS
    for (int i = 0; i < MAX_PLAYERS; i++) {
        player_init(game, &game->players[i], (t_tuple){0, 0});
        game->players[i].lives = 3;
        game->players[i].active = false;
    }

    t_tuple spawn_out[MAX_ENEMIES];

    // MULTIPLAYER
    if (is_multiplayer) {
        player_init(game, &game->players[PLAYER_1], (t_tuple){1, 1});
        game->players[PLAYER_1].lives = 3;
        game->current_player = PLAYER_1;

        player_init(game, &game->players[PLAYER_2], (t_tuple){BOARD_COLS - 2, BOARD_ROWS - 2});
        game->players[PLAYER_2].lives = 3;
        game->players[PLAYER_2].active = true;

        game->enemy_count = spawn_enemies_multiplayer(game->board, 5, spawn_out);
        for (int i = 0; i < game->enemy_count; i++) {
            enemy_init(game, &game->enemies[i], spawn_out[i]);
        }
    } 

    // SINGLEPLAYER
    else {
        t_tuple spawnpoint = spawnpoint_generator(game->board, game->click_count);
        player_init(game, &game->players[PLAYER_1], spawnpoint);
        game->players[PLAYER_1].lives = 3;
        game->current_player = PLAYER_1;

        game->enemy_count = spawn_enemies_singleplayer(game->board, game->players[PLAYER_1].board_pos, 3, spawn_out);
        for (int i = 0; i < game->enemy_count; i++) {
            enemy_init(game, &game->enemies[i], spawn_out[i]);
        }
    }

    for (int i = 0; i < MAX_BOMBS; i++) {
        bomb_init(&game->bomb[i]);
    }

    game->click_count = 0;
}

int game_state_init(t_game_state *game, uint32_t width, uint32_t height, t_time time, bool is_multiplayer) {
    if (game == NULL || width == 0 || height == 0) return 1;

    game_state_destroy(game);

    game->width = width;
    game->height = height;

    int32_t max_tile_w = (int32_t)width / BOARD_COLS;
    int32_t max_tile_h = (int32_t)height / BOARD_ROWS;
    int32_t tile = max_tile_w < max_tile_h ? max_tile_w : max_tile_h;
    tile = (tile / BOARD_BASE_TILE_SIZE) * BOARD_BASE_TILE_SIZE;
    if (tile < BOARD_BASE_TILE_SIZE)
        tile = BOARD_BASE_TILE_SIZE;

    game->tile_size = tile;
    game->is_multiplayer = is_multiplayer;

    int32_t board_width = BOARD_COLS * tile;
    int32_t board_height = BOARD_ROWS * tile;
    game->start_x = (width - board_width) / 2;
    game->start_y = (height - board_height) / 2;

    _game_state_prepare_match(game, time, is_multiplayer);
    game->score = 0;
    game->time_limit = 180; // seconds

    scale_all_game_sprites(game->tile_size, game->players[PLAYER_1].size.x, game->players[PLAYER_1].size.y, MAX_PLAYERS);

    return 0;
}

void game_state_reset(t_game_state *game, t_time time, bool is_multiplayer) {
    if (game == NULL) return;
    game->is_multiplayer = is_multiplayer;
    _game_state_prepare_match(game, time, is_multiplayer);
    game->score = 0;
}

void game_state_destroy(t_game_state *game) {
    if (game == NULL) return;

    game->width = 0;
    game->height = 0;
    game->tile_size = 0;
}

void game_state_update(t_ctx *ctx) {
    if (ctx == NULL) return;

    t_game_state *game = &ctx->game;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        player_t *player = &game->players[i];

        if (!player->active) continue;

        if (player->invincibility_timer > 0) {
            player->invincibility_timer--;
        } else if (player_collides_with_enemy(game, player)) {
            update_player_lives(player, -1);

            if (player->lives > 0) {
                player->invincibility_timer = INVINCIBILITY_TICKS;
            }
        }

        if (player->lives > 0) {
            update_player_movement(game, player);
            update_player_animation(player, game->logical_ticks);

            // Handle Power-up Pickup
            uint8_t tile = game->board[player->board_pos.y * BOARD_COLS + player->board_pos.x];
            if (tile == TILE_TYPE_POWERUP_REACH) {
                uint8_t current = GET_POWERUP_REACH(player->powerups);
                if (current < 3) SET_POWERUP_REACH(player->powerups, current + 1);
                game->board[player->board_pos.y * BOARD_COLS + player->board_pos.x] = TILE_TYPE_GRASS;
            } else if (tile == TILE_TYPE_POWERUP_COUNT) {
                uint8_t current = GET_POWERUP_COUNT(player->powerups);
                if (current < 3) SET_POWERUP_COUNT(player->powerups, current + 1);
                game->board[player->board_pos.y * BOARD_COLS + player->board_pos.x] = TILE_TYPE_GRASS;
            }
        }
    }

    for (int i = 0; i < game->enemy_count; i++) {
        enemy_t *enemy = &game->enemies[i];

        if (!enemy->active) continue;

        update_enemy_movement(game, enemy);
        update_enemy_animation(game, enemy, game->logical_ticks);
    }

    for (int i = 0; i < MAX_BOMBS; i++) {
        bomb_update(game, &game->bomb[i]);
    }

    // Every 10 seconds spawn a new enemy if there is space and the door isn't open
    uint32_t enemy_spawn_interval = GAME_TICKS_PER_SECOND * 10;

    if (game->match_state == MATCH_RUNNING &&
        game->logical_ticks >= enemy_spawn_interval &&
        game->logical_ticks - game->last_enemy_spawn_ticks >= enemy_spawn_interval) {

        game->last_enemy_spawn_ticks = game->logical_ticks;

        int free_idx = -1;
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (!game->enemies[i].active) {
                free_idx = i;
                break;
            }
        }

        if (free_idx != -1) {
            t_tuple spawn;

            if (spawn_new_enemy(game, &spawn)) {
                enemy_init(game, &game->enemies[free_idx], spawn);

                if (free_idx >= game->enemy_count) {
                    game->enemy_count = (uint8_t)(free_idx + 1);
                }
            }
        }
    }

    player_bomb_count(game);

    if (game->match_state == MATCH_RUNNING) {
        uint32_t elapsed = game->logical_ticks / GAME_TICKS_PER_SECOND;

        if (elapsed >= game->time_limit) {
            for (int i = 0; i < MAX_PLAYERS; i++) {
                player_t *player = &game->players[i];
                if (player->active) {
                    player->lives = 0;
                    player->invincibility_timer = 0;
                }
            }
            game->match_state = MATCH_LOST;
            game->animation_timer = GAME_TICKS_PER_SECOND * 5;
            return;
        }

        int players_alive = 0;

        for (int i = 0; i < MAX_PLAYERS; i++) {
            player_t *player = &game->players[i];

            if (!player->active) continue;
            if (player->lives > 0) players_alive++;
        }

        if (players_alive == 0) {
            game->match_state = MATCH_LOST;
            game->animation_timer = GAME_TICKS_PER_SECOND * 5;
            return;
        }

        bool enough_enemies_killed = game->score >= game->enemies_to_kill;
        bool players_ready = !game->is_multiplayer || (players_alive <= 1);

        if (enough_enemies_killed && players_ready) {
            game->door_open = true;

            for (int i = 0; i < MAX_PLAYERS; i++) {
                player_t *player = &game->players[i];

                if (!player->active || player->lives <= 0) continue;

                if (player->board_pos.x == game->door_pos.x && player->board_pos.y == game->door_pos.y) {
                    game->match_state = MATCH_WON;
                    game->animation_timer = GAME_TICKS_PER_SECOND * 3;
                    return;
                }
            }
        }
    }
}

void game_state_handle_click(t_game_state *game, int32_t x, int32_t y) {
    (void)game; (void)x; (void)y;
}

void game_state_handle_key_press(t_game_state *game, uint8_t scancode) {
    if (game == NULL) return;
    game_state_handle_player_key(game, game->current_player, scancode);
}

void game_state_handle_player_key(t_game_state *game, uint8_t player_id, uint8_t scancode) {
    if (game == NULL) return;

    bool is_make = !(scancode & 0x80);
    uint8_t key_index = scancode & 0x7F;

    if (player_id >= MAX_PLAYERS) return;
    player_t *player = &game->players[player_id];
    if (!player->active) return;

    if (is_make && key_index == KEY_SPACE) {
        uint8_t previous_player = game->current_player;
        game->current_player = player_id;
        place_player_bomb(game, player);
        game->current_player = previous_player;
    }

    update_player_direction(player, key_index, is_make);
}


