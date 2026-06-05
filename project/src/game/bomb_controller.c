#include "game/bomb_controller.h"
#include "game/player_controller.h"
#include "game/entity_controller.h"
#include "models/board.h"
#include <stddef.h>
#include <math.h>

void bomb_init(bomb_t *bomb) {
    if (bomb == NULL) return;

    bomb->active = false;
    bomb->state = BOMB_INACTIVE;
    bomb->player_id = PLAYER_1;
    bomb->board_pos = (t_tuple) {0, 0};
    bomb->bomb_timer = 0;
    bomb->explosion_timer = 0;
    bomb_clear_explosion(bomb);
}

void bomb_reset(bomb_t *bomb) {
    if (bomb == NULL) return;

    bomb->active = false;
    bomb->state = BOMB_INACTIVE;
    bomb->bomb_timer = 0;
    bomb->player_id = PLAYER_1;
    bomb_clear_explosion(bomb);
}

void bomb_update(t_game_state *game, bomb_t *bomb) {
    if (!game || !bomb) return;

    if (!bomb->active) return;

    if (bomb->state == BOMB_FIRE) {
        bomb_update_explosion(game, bomb);
        return;
    }

    if (bomb->bomb_timer == 0) {
        bomb_begin_explosion(game, bomb);
        return;
    }

    bomb->bomb_timer--;

    if (bomb->bomb_timer == 0) {
        bomb_begin_explosion(game, bomb);
        return;
    }

    uint32_t elapsed = BOMB_DURATION_TICKS - bomb->bomb_timer;
    uint32_t phase = (elapsed * BOMB_FUSE_PHASES) / BOMB_DURATION_TICKS;

    if (phase >= 6) {
        if (phase == 6) {
            bomb->state = BOMB_EXPLODE;
        } else {
            bomb->bomb_timer = 0;
        }
    } else {
        uint32_t blink_speed = (phase < 4) ? 15 : 8; 

        if ((elapsed / blink_speed) % 2 == 0) {
            bomb->state = BOMB_PLACED;
        } else {
            bomb->state = BOMB_BLINK;
        }
    }

}

void place_player_bomb(t_game_state *game, player_t *player) {
    if (!game || !player) return;

    if (game->current_player >= MAX_PLAYERS) return;
    if (player->bomb_available == 0) return;

    for (int i = 0; i < MAX_BOMBS; i++) {
        bomb_t *bomb = &game->bomb[i];
        if (!bomb->active) continue;
        if (bomb->board_pos.x == player->board_pos.x && bomb->board_pos.y == player->board_pos.y) return;
    }

    for (int i = 0; i < MAX_BOMBS; i++) {
        bomb_t *bomb = &game->bomb[i];
        if (bomb->active) continue;

        bomb->active = true;
        bomb->state = BOMB_PLACED;
        bomb->player_id = game->current_player;
        bomb->board_pos = player->board_pos;
        bomb->bomb_timer = BOMB_DURATION_TICKS;
        bomb_clear_explosion(bomb);
        if (player->bomb_available > 0) player->bomb_available--;
        return;
    }
}

static int32_t _pixel_to_tile_x(const t_game_state *game, int32_t px) {
    return (px - game->start_x) / game->tile_size;
}

static int32_t _pixel_to_tile_y(const t_game_state *game, int32_t py) {
    return (py - game->start_y) / game->tile_size;
}

int8_t bomb_drag_start(t_game_state *game, int32_t px, int32_t py) {
    if (game == NULL || game->tile_size == 0) return -1;
    if (game->current_player >= MAX_PLAYERS) return -1;

    player_t *player = &game->players[game->current_player];

    if (!GET_POWERUP_DRAG(player->powerups)) return -1;

    int32_t tx = _pixel_to_tile_x(game, px);
    int32_t ty = _pixel_to_tile_y(game, py);

    if (tx < 0 || ty < 0 || tx >= BOARD_COLS || ty >= BOARD_ROWS) return -1;

    game->dragged_bomb_idx = -1;

    for (int i = 0; i < MAX_BOMBS; i++) {
        bomb_t *bomb = &game->bomb[i];

        if (!bomb->active || bomb->state == BOMB_FIRE) continue;
        if (bomb->player_id != game->current_player) continue;

        if (bomb->board_pos.x == tx && bomb->board_pos.y == ty) {
            game->dragged_bomb_idx = i;
            return i;
        }
    }

    return -1;
}

void bomb_drag_move(t_game_state *game, int32_t px, int32_t py) {
    if (game == NULL || game->tile_size == 0) return;
    if (game->dragged_bomb_idx < 0 || game->dragged_bomb_idx >= MAX_BOMBS) return;

    bomb_t *bomb = &game->bomb[game->dragged_bomb_idx];

    if (!bomb->active || bomb->state == BOMB_FIRE) return;
    if (bomb->player_id != game->current_player) return;

    int32_t tx = _pixel_to_tile_x(game, px);
    int32_t ty = _pixel_to_tile_y(game, py);

    if (tx < 0 || ty < 0 || tx >= BOARD_COLS || ty >= BOARD_ROWS) return;

    uint8_t tile = game->board[BOARD_IDX(tx, ty)];

    if (tile != TILE_TYPE_GRASS &&
        tile != TILE_TYPE_DOOR &&
        tile != TILE_TYPE_POWERUP_REACH &&
        tile != TILE_TYPE_POWERUP_COUNT &&
        tile != TILE_TYPE_POWERUP_DRAG) {
        return;
    }

    for (int i = 0; i < MAX_BOMBS; i++) {
        if (i == game->dragged_bomb_idx) continue;

        bomb_t *other = &game->bomb[i];

        if (!other->active || other->state == BOMB_FIRE) continue;

        if (other->board_pos.x == tx && other->board_pos.y == ty) return;
    }

    bomb->board_pos.x = tx;
    bomb->board_pos.y = ty;
}

void bomb_drag_end(t_game_state *game) {
    if (game == NULL) return;
    game->dragged_bomb_idx = -1;
}
