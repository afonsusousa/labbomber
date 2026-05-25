#include "game.h"
#include "vbe.h"
#include "assets_cache.h"
#include "draw.h"
#include <stddef.h>

void bomb_init(bomb_t *bomb) {
    if (bomb == NULL) return;

    bomb->active = false;
    bomb->state = BOMB_INACTIVE;
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
    bomb_clear_explosion(bomb);
}

void bomb_update(t_game_state *game) {
    if (!game) return;

    bomb_t *bomb = &game->bomb;

    if (!bomb->active) return;

    if (bomb->state == BOMB_FIRE) {
        bomb_update_explosion(game);
        return;
    }

    if (bomb->bomb_timer == 0) {
        bomb_begin_explosion(game);
        return;
    }

    bomb->bomb_timer--;

    if (bomb->bomb_timer == 0) {
        bomb_begin_explosion(game);
        return;
    }

    uint32_t elapsed = BOMB_DURATION_TICKS - bomb->bomb_timer;
    uint32_t phase = (elapsed * BOMB_FUSE_PHASES) / BOMB_DURATION_TICKS;

    switch (phase) {
        case 1:
        case 3:
        case 5:
            bomb->state = BOMB_BLINK;
            break;
        case 6:
            bomb->state = BOMB_EXPLODE;
            break;
        case 7:
            bomb->bomb_timer = 0;
            break;
        default:
            bomb->state = BOMB_PLACED;
            break;
    }
}

void place_player_bomb(t_game_state *game, const player_t *player) {
    if (!game || !player || game->bomb.active) return;

    game->bomb.active = true;
    game->bomb.state = BOMB_PLACED;
    game->bomb.board_pos = player->board_pos;
    game->bomb.bomb_timer = BOMB_DURATION_TICKS;
    bomb_clear_explosion(&game->bomb);
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

int draw_bomb(hw_video_t *video, t_game_state *game) {
    if (game == NULL || !sprites_initialized) return 1;

    bomb_t *bomb = &game->bomb;
    if (!bomb->active) return 1;

    if (bomb->state == BOMB_FIRE) {
        return draw_bomb_explosion(video, game);
    }

    int sprite_index = get_bomb_sprite_index(bomb);
    if (sprite_index >= SPRITE_CACHE_SIZE || scaled_sprite_cache[sprite_index].bytes == NULL) {
        return 1;
    }

    xpm_image_t img = scaled_sprite_cache[sprite_index];

    hw_vbe_draw_xpm(
        video,
        img.bytes,
        img,
        GET_X(game, game->bomb.board_pos.x),
        GET_Y(game, game->bomb.board_pos.y)
    );
    return 0;
}
