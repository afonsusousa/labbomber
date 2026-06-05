#include "view/game/draw_game.h"
#include "view/game/draw_map.h"
#include "view/game/draw_bombs.h"
#include "view/game/draw_enemy.h"
#include "view/game/draw_player.h"
#include "view/game/draw_hud.h"
#include "game/game.h"
#include "game/map_helpers.h"
#include "view/assets_cache.h"
#include "core/application.h"
#include "vbe.h"
#include <stdio.h>
#include <lcom/xpm.h>

void draw_game_board(t_widget *self, hw_video_t *video, void *state) {
    if (self == NULL || state == NULL) return;

    t_game_state *game = GAME(state);

    // Draw Map
    for (int y = 0; y < BOARD_ROWS; y++) {
        for (int x = 0; x < BOARD_COLS; x++) {
            int val = game->board[y * BOARD_COLS + x];
            if (val == TILE_TYPE_WALL) {
                int wall_sprite = decide_wall_sprite(game->board, BOARD_ROWS, BOARD_COLS, x, y);
                draw_wall(video, GET_X(game, x), GET_Y(game, y), wall_sprite);
            } else if (val == TILE_TYPE_BRICK) {
                draw_brick(video, GET_X(game, x), GET_Y(game, y));
            } else if (val == TILE_TYPE_DOOR) {
                draw_door(video, GET_X(game, x), GET_Y(game, y), game->door_open);
            } else {
                int grass_type = decide_grass_sprite(game->board, BOARD_ROWS, BOARD_COLS, x, y);
                draw_grass(video, GET_X(game, x), GET_Y(game, y), grass_type);

                // powerups on top of the grass
                if (val == TILE_TYPE_POWERUP_REACH) {
                    xpm_image_t img = scaled_sprite_cache[SPRITE_PLAYER_HAT_1];
                    if (img.bytes) hw_vbe_draw_xpm(video, img.bytes, img, GET_X(game, x), GET_Y(game, y));
                } else if (val == TILE_TYPE_POWERUP_COUNT) {
                    xpm_image_t img = scaled_sprite_cache[SPRITE_PLAYER_HAT_2];
                    if (img.bytes) hw_vbe_draw_xpm(video, img.bytes, img, GET_X(game, x), GET_Y(game, y));
                }
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

    uint32_t elapsed = game->logical_ticks / GAME_TICKS_PER_SECOND;
    uint32_t remaining = elapsed >= game->time_limit ? 0 : game->time_limit - elapsed;
    uint32_t mins = remaining / 60;
    uint32_t secs = remaining % 60;

    char timer_buf[8];
    snprintf(timer_buf, sizeof(timer_buf), "%02u:%02u", mins, secs);

    int32_t tx = self->abs_x + (self->width / 2) - 30;
    int32_t ty = self->abs_y + 30;

    for (int i = 0; timer_buf[i] != '\0'; i++) {
        if (timer_buf[i] == ':') {
            xpm_image_t doispontos = scaled_sprite_cache[DOIS_PONTOS];
            if (doispontos.bytes != NULL) {
                hw_vbe_draw_xpm(video, doispontos.bytes, doispontos, tx, ty);
                tx += doispontos.width + 2;
            }
            continue;
        }

        xpm_image_t img = scaled_sprite_cache[NUMBER_0 + (timer_buf[i] - '0')];
        if (img.bytes == NULL) {
            tx += 10;
            continue;
        }
        hw_vbe_draw_xpm(video, img.bytes, img, tx, ty);
        tx += img.width + 2;
    }
}
