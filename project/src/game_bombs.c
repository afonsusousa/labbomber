#include "draw.h"
#include "assets_cache.h"
#include "game.h"
#include "vbe.h"
#include <lcom/xpm.h>
#include <stdint.h>

void bomb_init(bomb_t *bomb) {
    if (bomb == NULL) return;

    bomb->active = false;
    bomb->state = BOMB_INACTIVE;
    bomb->board_pos = (t_tuple) {0, 0};
    bomb->bomb_timer = 0;
}

void bomb_reset(bomb_t *bomb) {
    if (bomb == NULL) return;

    bomb->active = false;
    bomb->state = BOMB_INACTIVE;
    bomb->bomb_timer = 0;
}

void bomb_update(bomb_t *bomb) {
    if (bomb == NULL || !bomb->active) return;

    if (bomb->bomb_timer == 0) {
        bomb_reset(bomb);
        return;
    }

    bomb->bomb_timer--;

    if (bomb->bomb_timer == 0) {
        bomb_reset(bomb);
    } else if (bomb->bomb_timer <= BOMB_EXPLODE_TICKS) {
        bomb->state = BOMB_EXPLODE;
    } else if (bomb->bomb_timer <= BOMB_BLINK_TICKS) {
        bomb->state = BOMB_BLINK;
    } else {
        bomb->state = BOMB_PLACED;
    }
}

void place_player_bomb(t_game_state *game, const player_t *player) {
    if (game == NULL || player == NULL || game->bomb.active) return;

    game->bomb.active = true;
    game->bomb.state = BOMB_PLACED;
    game->bomb.board_pos = player->board_pos;
    game->bomb.bomb_timer = BOMB_DURATION_TICKS;
}

static int get_bomb_sprite_index(const bomb_t *bomb) {
    switch (bomb->state) {
        case BOMB_BLINK:
            return SPRITE_BOMB2;
        case BOMB_EXPLODE:
            return SPRITE_BOMB3;
        case BOMB_PLACED:
        default:
            return SPRITE_BOMB1;
    }
}

int draw_bomb(hw_video_t *video, const bomb_t *bomb, int32_t x, int32_t y) {
    if (bomb == NULL || !bomb->active || !sprites_initialized) return 1;

    int sprite_index = get_bomb_sprite_index(bomb);

    if (sprite_index >= SPRITE_CACHE_SIZE || scaled_sprite_cache[sprite_index].bytes == NULL) {
        return 1;
    }

    xpm_image_t img = scaled_sprite_cache[sprite_index];
    int32_t draw_x = x - (img.width / 2);
    int32_t draw_y = y - (img.height / 2);

    hw_vbe_draw_xpm(video, img.bytes, img, draw_x, draw_y);
    return 0;
}
