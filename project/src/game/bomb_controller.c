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

