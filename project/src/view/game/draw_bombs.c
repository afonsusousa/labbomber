#include "view/game/draw_bombs.h"
#include "game/bomb_controller.h"
#include "view/assets_cache.h"
#include "game/game.h"
#include "core/macros.h"
#include "vbe.h"
#include <stdint.h>
#include <lcom/xpm.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define BOMB_REACH(bomb, dir) (bomb->radius < bomb->reach[dir]) ? bomb->radius : bomb->reach[dir]

static const int32_t DIR_X[EXPLOSION_DIR_COUNT] = { 1, -1, 0, 0 };
static const int32_t DIR_Y[EXPLOSION_DIR_COUNT] = { 0, 0, 1, -1 };

static int get_bomb_sprite_index(const bomb_t *bomb) {
    switch (bomb->state) {
        case BOMB_BLINK:   return SPRITE_BOMB2;
        case BOMB_EXPLODE: return SPRITE_BOMB3;
        case BOMB_PLACED:
        default:           return SPRITE_BOMB1;
    }
}

static int explosion_center_sprite_index(uint8_t frame) {
    uint8_t max_frame = ARRAY_SIZE(explosion_center_sprites) - 1;
    return explosion_center_sprites[frame > max_frame ? max_frame : frame];
}

static int explosion_side_sprite_index(uint8_t frame, bool is_tip) {
    uint8_t max_frame = ARRAY_SIZE(explosion_arm_sprites) - 1;
    frame = frame > max_frame ? max_frame : frame;
    return is_tip ? explosion_hand_sprites[frame] : explosion_arm_sprites[frame];
}

static void bomb_draw_explosion_ray(hw_video_t *video, const t_game_state *game, const bomb_t *bomb, uint8_t dir, uint8_t rotation, uint8_t frame) {
    uint8_t reach = BOMB_REACH(bomb, dir);

    for (uint8_t dist = 1; dist <= reach; dist++) {
        int sprite_index = explosion_side_sprite_index(frame, dist == reach);
        if (sprite_index < 0 || sprite_index >= SPRITE_CACHE_SIZE) continue;

        xpm_image_t img = scaled_sprite_cache[sprite_index];
        if (!img.bytes) continue;

        hw_vbe_draw_rotated_xpm(
            video,
            img.bytes,
            img,
            GET_X(game, bomb->board_pos.x + (DIR_X[dir] * dist)),
            GET_Y(game, bomb->board_pos.y + (DIR_Y[dir] * dist)),
            rotation
        );
    }
}

int draw_bomb_explosion(hw_video_t *video, t_game_state *game, const bomb_t *bomb) {
    if (!game || !bomb || !sprites_initialized || !bomb->active) return 1;

    uint8_t frame = bomb_explosion_frame(bomb);
    int center_index = explosion_center_sprite_index(frame);

    if (center_index >= 0 && center_index < SPRITE_CACHE_SIZE && scaled_sprite_cache[center_index].bytes) {
        xpm_image_t img = scaled_sprite_cache[center_index];
        hw_vbe_draw_xpm(
            video,
            img.bytes,
            img,
            GET_X(game, bomb->board_pos.x),
            GET_Y(game, bomb->board_pos.y)
        );
    }

    if (bomb->radius > 0) {
        bomb_draw_explosion_ray(video, game, bomb, EXPLOSION_DIR_RIGHT, XPM_ROTATE_180, frame);
        bomb_draw_explosion_ray(video, game, bomb, EXPLOSION_DIR_LEFT,  XPM_ROTATE_0,   frame);
        bomb_draw_explosion_ray(video, game, bomb, EXPLOSION_DIR_DOWN,  XPM_ROTATE_270, frame);
        bomb_draw_explosion_ray(video, game, bomb, EXPLOSION_DIR_UP,    XPM_ROTATE_90,  frame);
    }

    return 0;
}

int draw_bomb(hw_video_t *video, t_game_state *game) {
    if (game == NULL || !sprites_initialized) return 1;

    int status = 0;
    for (int i = 0; i < MAX_BOMBS; i++) {
        bomb_t *bomb = &game->bomb[i];
        if (!bomb->active) continue;

        if (bomb->state == BOMB_FIRE) {
            if (draw_bomb_explosion(video, game, bomb) != 0) status = 1;
            continue;
        }

        int sprite_index = get_bomb_sprite_index(bomb);
        if (sprite_index >= SPRITE_CACHE_SIZE || scaled_sprite_cache[sprite_index].bytes == NULL) {
            status = 1;
            continue;
        }

        xpm_image_t img = scaled_sprite_cache[sprite_index];
        hw_vbe_draw_xpm(
            video,
            img.bytes,
            img,
            GET_X(game, bomb->board_pos.x),
            GET_Y(game, bomb->board_pos.y)
        );
    }
    return status;
}
