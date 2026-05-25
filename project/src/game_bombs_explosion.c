#include "game.h"
#include "vbe.h"
#include "macros.h"
#include "assets_cache.h"
#include <stddef.h>
#include <stdint.h>

void bomb_clear_explosion(bomb_t *bomb) {
    if (bomb == NULL) return;

    bomb->explosion_timer = 0;
    bomb->explosion_current_radius = 0;
    bomb->explosion_blocked_steps = 0;
}

uint8_t bomb_explosion_frame(const bomb_t *bomb) {
    const uint8_t max_frame = 5;
    const uint32_t total_steps = (2 * max_frame) + 1;

    uint32_t age = 0;
    if (bomb != NULL && bomb->explosion_timer < BOMB_EXPLOSION_DURATION_TICKS) {
        age = BOMB_EXPLOSION_DURATION_TICKS - bomb->explosion_timer;
    }

    uint32_t step = (age * total_steps) / ((uint32_t)BOMB_EXPLOSION_DURATION_TICKS);
    if (step >= total_steps) step = total_steps - 1;

    if (step <= max_frame) return (uint8_t)step;
    return (uint8_t)((2 * max_frame) - step);
}

static void explosion_set_blocked_step(bomb_t *bomb, uint8_t dir, uint8_t value) {
    if (bomb == NULL) return;
    bomb->explosion_blocked_steps &= (uint16_t)~(0x0F << (dir * 4));
    bomb->explosion_blocked_steps |= (uint16_t)((value & 0x0F) << (dir * 4));
}

static void bomb_compute_explosion_reach(t_game_state *game) {
    bomb_t *bomb = &game->bomb;
    const int32_t dx[EXPLOSION_DIR_COUNT] = { 1, -1, 0, 0 };
    const int32_t dy[EXPLOSION_DIR_COUNT] = { 0, 0, 1, -1 };

    bomb->explosion_blocked_steps = 0;

    for (uint8_t dir = 0; dir < EXPLOSION_DIR_COUNT; dir++) {
        for (uint8_t distance = 1; distance <= BOMB_EXPLOSION_RANGE; distance++) {
            int32_t x = bomb->board_pos.x + (dx[dir] * distance);
            int32_t y = bomb->board_pos.y + (dy[dir] * distance);

            if (x < 0 || y < 0 || x >= BOARD_COLS || y >= BOARD_ROWS) {
                explosion_set_blocked_step(bomb, dir, (uint8_t)(distance - 1));
                break;
            }

            uint8_t tile = game->board[y * BOARD_COLS + x];
            if (tile == 1) {
                explosion_set_blocked_step(bomb, dir, (uint8_t)(distance - 1));
                break;
            }

            explosion_set_blocked_step(bomb, dir, (uint8_t)distance);
            if (tile == 2) break;
        }
    }
}

static uint8_t explosion_reach(const bomb_t *bomb, uint8_t dir) {
    if (bomb == NULL) return 0;
    uint8_t reach = (uint8_t)((bomb->explosion_blocked_steps >> (dir * 4)) & 0x0F);
    return (bomb->explosion_current_radius < reach) ? bomb->explosion_current_radius : reach;
}


static void bomb_apply_explosion_contact(t_game_state *game, uint8_t radius) {
    if (radius == 0) return;

    bomb_t *bomb = &game->bomb;

    const int32_t dx[EXPLOSION_DIR_COUNT] = { 1, -1, 0, 0 };
    const int32_t dy[EXPLOSION_DIR_COUNT] = { 0, 0, 1, -1 };

    for (uint8_t dir = 0; dir < EXPLOSION_DIR_COUNT; dir++) {
        int32_t x = bomb->board_pos.x + (dx[dir] * radius);
        int32_t y = bomb->board_pos.y + (dy[dir] * radius);

        if (x < 0 || y < 0 || x >= BOARD_COLS || y >= BOARD_ROWS) continue;

        if (!explosion_collides(game, x, y)) continue;
        if (game->board[y * BOARD_COLS + x] != 2) continue;

        game->board[y * BOARD_COLS + x] = 0;
    }
}

void bomb_begin_explosion(t_game_state *game) {
    if (game == NULL) return;

    bomb_t *bomb = &game->bomb;
    bomb_clear_explosion(bomb);

    bomb->active = true;
    bomb->state = BOMB_FIRE;
    bomb->bomb_timer = 0;
    bomb->explosion_timer = BOMB_EXPLOSION_DURATION_TICKS;
    bomb->explosion_current_radius = 0;
    bomb_compute_explosion_reach(game);
}

void bomb_update_explosion(t_game_state *game) {
    if (game == NULL) return;

    bomb_t *bomb = &game->bomb;
    if (!bomb->active || bomb->state != BOMB_FIRE) return;

    if (bomb->explosion_timer == 0) {
        bomb_reset(bomb);
        return;
    }

    bomb->explosion_timer--;
    bomb->explosion_current_radius = (uint8_t)(bomb_explosion_frame(bomb) / 2);
    bomb_apply_explosion_contact(game, bomb->explosion_current_radius);

    if (bomb->explosion_timer == 0) {
        bomb_reset(bomb);
    }
}

static int explosion_center_sprite_index(uint8_t frame) {
    if (frame >= (sizeof(explosion_center_sprites) / sizeof(explosion_center_sprites[0]))) {
        frame = (uint8_t)(sizeof(explosion_center_sprites) / sizeof(explosion_center_sprites[0]) - 1);
    }

    return explosion_center_sprites[frame];
}

static int explosion_side_sprite_index(uint8_t frame, bool is_tip) {
    if (frame >= (sizeof(explosion_arm_sprites) / sizeof(explosion_arm_sprites[0]))) {
        frame = (uint8_t)(sizeof(explosion_arm_sprites) / sizeof(explosion_arm_sprites[0]) - 1);
    }

    return is_tip ? explosion_hand_sprites[frame] : explosion_arm_sprites[frame];
}

static void bomb_draw_tile(hw_video_t *video, const xpm_image_t *img, int32_t x, int32_t y, uint8_t rotation) {
    if (rotation == XPM_ROTATE_0) {
        hw_vbe_draw_xpm(video, img->bytes, *img, x, y);
    } else {
        hw_vbe_draw_rotated_xpm(video, img->bytes, *img, x, y, rotation);
    }
}

static void bomb_draw_explosion_ray(hw_video_t *video, const t_game_state *game, int32_t board_start_x, int32_t board_start_y,
                                    uint32_t tile_size, int32_t dx, int32_t dy, uint8_t dir, uint8_t rotation,
                                    uint8_t frame) {
    const bomb_t *bomb = &game->bomb;
    uint8_t reach = explosion_reach(bomb, dir);

    for (uint8_t distance = 1; distance <= reach; distance++) {
        int sprite_index = explosion_side_sprite_index(frame, distance == reach);
        if (sprite_index < 0 || sprite_index >= SPRITE_CACHE_SIZE) continue;
        if (scaled_sprite_cache[sprite_index].bytes == NULL) continue;

        xpm_image_t img = scaled_sprite_cache[sprite_index];
        uint32_t draw_width = (rotation & 1) ? img.height : img.width;
        uint32_t draw_height = (rotation & 1) ? img.width : img.height;
        int32_t draw_x = board_start_x + ((bomb->board_pos.x + (dx * distance)) * (int32_t)tile_size) + ((int32_t)tile_size / 2) - ((int32_t)draw_width / 2);
        int32_t draw_y = board_start_y + ((bomb->board_pos.y + (dy * distance)) * (int32_t)tile_size) + ((int32_t)tile_size / 2) - ((int32_t)draw_height / 2);

        bomb_draw_tile(video, &img, draw_x, draw_y, rotation);
    }
}

int draw_bomb_explosion(hw_video_t *video, t_game_state *game, int32_t board_start_x, int32_t board_start_y, uint32_t tile_size) {
    if (game == NULL || !sprites_initialized) return 1;

    const bomb_t *bomb = &game->bomb;
    if (!bomb->active) return 1;

    uint8_t frame = bomb_explosion_frame(bomb);
    int center_index = explosion_center_sprite_index(frame);
    if (center_index >= 0 && center_index < SPRITE_CACHE_SIZE && scaled_sprite_cache[center_index].bytes != NULL) {
        xpm_image_t img = scaled_sprite_cache[center_index];
        int32_t draw_x = board_start_x + (bomb->board_pos.x * (int32_t)tile_size) + ((int32_t)tile_size / 2) - (img.width / 2);
        int32_t draw_y = board_start_y + (bomb->board_pos.y * (int32_t)tile_size) + ((int32_t)tile_size / 2) - (img.height / 2);
        hw_vbe_draw_xpm(video, img.bytes, img, draw_x, draw_y);
    }

    if (bomb->explosion_current_radius == 0) return 0;

    bomb_draw_explosion_ray(video, game, board_start_x, board_start_y, tile_size, 1, 0, EXPLOSION_DIR_RIGHT, XPM_ROTATE_180, frame);
    bomb_draw_explosion_ray(video, game, board_start_x, board_start_y, tile_size, -1, 0, EXPLOSION_DIR_LEFT, XPM_ROTATE_0, frame);
    bomb_draw_explosion_ray(video, game, board_start_x, board_start_y, tile_size, 0, 1, EXPLOSION_DIR_DOWN, XPM_ROTATE_270, frame);
    bomb_draw_explosion_ray(video, game, board_start_x, board_start_y, tile_size, 0, -1, EXPLOSION_DIR_UP, XPM_ROTATE_90, frame);
    return 0;
}

bool explosion_collides(const t_game_state *game, int32_t cell_x, int32_t cell_y) {
    if (game == NULL) return false;

    const bomb_t *bomb = &game->bomb;
    if (!bomb->active || bomb->state != BOMB_FIRE) return false;

    if (cell_x == bomb->board_pos.x && cell_y == bomb->board_pos.y) return true;

    uint8_t radius = bomb->explosion_current_radius;
    if (radius == 0) return false;

    int32_t dx = cell_x - bomb->board_pos.x;
    int32_t dy = cell_y - bomb->board_pos.y;
    if (dx != 0 && dy != 0) return false;

    uint8_t dir;
    uint32_t dist;
    if (dx > 0) { dir = EXPLOSION_DIR_RIGHT; dist = (uint32_t)dx; }
    else if (dx < 0) { dir = EXPLOSION_DIR_LEFT; dist = (uint32_t)(-dx); }
    else if (dy > 0) { dir = EXPLOSION_DIR_DOWN; dist = (uint32_t)dy; }
    else { dir = EXPLOSION_DIR_UP; dist = (uint32_t)(-dy); }

    if (dist == 0) return true;
    if (dist > radius) return false;

    uint8_t reach = explosion_reach(bomb, dir);
    return dist <= reach;
}

