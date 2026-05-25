#include "game.h"
#include "vbe.h"
#include "macros.h"
#include "assets_cache.h"
#include <stddef.h>
#include <stdint.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define IN_BOUNDS(x, y) ((x) >= 0 && (x) < BOARD_COLS && (y) >= 0 && (y) < BOARD_ROWS)

static const int32_t DIR_X[EXPLOSION_DIR_COUNT] = { 1, -1, 0, 0 };
static const int32_t DIR_Y[EXPLOSION_DIR_COUNT] = { 0, 0, 1, -1 };

void bomb_clear_explosion(bomb_t *bomb) {
    if (!bomb) return;
    bomb->explosion_timer = 0;
    bomb->explosion_current_radius = 0;
    bomb->explosion_blocked_steps = 0;
}

uint8_t bomb_explosion_frame(const bomb_t *bomb) {
    if (!bomb || bomb->explosion_timer >= BOMB_EXPLOSION_DURATION_TICKS) return 0;
    
    const uint32_t max_frame = 5;
    const uint32_t total_steps = (2 * max_frame) + 1; // 11
    uint32_t age = BOMB_EXPLOSION_DURATION_TICKS - bomb->explosion_timer;
    
    uint32_t step = (age * total_steps) / BOMB_EXPLOSION_DURATION_TICKS;
    if (step >= total_steps) step = total_steps - 1;

    return (uint8_t)(step <= max_frame ? step : (2 * max_frame) - step);
}

static void explosion_set_blocked_step(bomb_t *bomb, uint8_t dir, uint8_t value) {
    if (!bomb) return;
    bomb->explosion_blocked_steps &= (uint16_t)~(0x0F << (dir * 4));
    bomb->explosion_blocked_steps |= (uint16_t)((value & 0x0F) << (dir * 4));
}

static void bomb_compute_explosion_reach(t_game_state *game) {
    bomb_t *bomb = &game->bomb;
    bomb->explosion_blocked_steps = 0;

    for (uint8_t dir = 0; dir < EXPLOSION_DIR_COUNT; dir++) {
        for (uint8_t dist = 1; dist <= BOMB_EXPLOSION_RANGE; dist++) {
            int32_t x = bomb->board_pos.x + (DIR_X[dir] * dist);
            int32_t y = bomb->board_pos.y + (DIR_Y[dir] * dist);

            if (!IN_BOUNDS(x, y) || game->board[y * BOARD_COLS + x] == 1) {
                explosion_set_blocked_step(bomb, dir, dist - 1);
                break;
            }

            explosion_set_blocked_step(bomb, dir, dist);
            if (game->board[y * BOARD_COLS + x] == 2) break; // Stop at destructible block
        }
    }
}

static uint8_t explosion_reach(const bomb_t *bomb, uint8_t dir) {
    if (!bomb) return 0;
    uint8_t reach = (bomb->explosion_blocked_steps >> (dir * 4)) & 0x0F;
    return (bomb->explosion_current_radius < reach) ? bomb->explosion_current_radius : reach;
}

static void bomb_apply_explosion_contact(t_game_state *game, uint8_t radius) {
    if (radius == 0) return;

    for (uint8_t dir = 0; dir < EXPLOSION_DIR_COUNT; dir++) {
        int32_t x = game->bomb.board_pos.x + (DIR_X[dir] * radius);
        int32_t y = game->bomb.board_pos.y + (DIR_Y[dir] * radius);

        if (IN_BOUNDS(x, y) && explosion_collides(game, x, y) && game->board[y * BOARD_COLS + x] == 2) {
            game->board[y * BOARD_COLS + x] = 0;
        }
    }
}

void bomb_begin_explosion(t_game_state *game) {
    if (!game) return;

    bomb_t *bomb = &game->bomb;
    bomb_clear_explosion(bomb);

    bomb->active = true;
    bomb->state = BOMB_FIRE;
    bomb->bomb_timer = 0;
    bomb->explosion_timer = BOMB_EXPLOSION_DURATION_TICKS;
    bomb_compute_explosion_reach(game);
}

void bomb_update_explosion(t_game_state *game) {
    if (!game || !game->bomb.active || game->bomb.state != BOMB_FIRE) return;

    bomb_t *bomb = &game->bomb;
    if (bomb->explosion_timer == 0) {
        bomb_reset(bomb);
        return;
    }

    bomb->explosion_timer--;
    bomb->explosion_current_radius = (uint8_t)(bomb_explosion_frame(bomb) / 2);
    bomb_apply_explosion_contact(game, bomb->explosion_current_radius);

    if (bomb->explosion_timer == 0) bomb_reset(bomb);
}

// DRAWING CODE
static int explosion_center_sprite_index(uint8_t frame) {
    uint8_t max_frame = ARRAY_SIZE(explosion_center_sprites) - 1;
    return explosion_center_sprites[frame > max_frame ? max_frame : frame];
}

static int explosion_side_sprite_index(uint8_t frame, bool is_tip) {
    uint8_t max_frame = ARRAY_SIZE(explosion_arm_sprites) - 1;
    frame = frame > max_frame ? max_frame : frame;
    return is_tip ? explosion_hand_sprites[frame] : explosion_arm_sprites[frame];
}

static void bomb_draw_explosion_ray(hw_video_t *video, const t_game_state *game, int32_t board_start_x, 
                                    int32_t board_start_y, uint32_t tile_size, uint8_t dir, 
                                    uint8_t rotation, uint8_t frame) {
    const bomb_t *bomb = &game->bomb;
    uint8_t reach = explosion_reach(bomb, dir);

    for (uint8_t dist = 1; dist <= reach; dist++) {
        int sprite_index = explosion_side_sprite_index(frame, dist == reach);
        if (sprite_index < 0 || sprite_index >= SPRITE_CACHE_SIZE) continue;
        
        xpm_image_t img = scaled_sprite_cache[sprite_index];
        if (!img.bytes) continue;

        int32_t center_x = board_start_x + ((bomb->board_pos.x + (DIR_X[dir] * dist)) * tile_size) + (tile_size / 2);
        int32_t center_y = board_start_y + ((bomb->board_pos.y + (DIR_Y[dir] * dist)) * tile_size) + (tile_size / 2);

        hw_vbe_draw_rotated_xpm(video, img.bytes, img, center_x, center_y, rotation);
    }
}

int draw_bomb_explosion(hw_video_t *video, t_game_state *game, int32_t board_start_x, int32_t board_start_y, uint32_t tile_size) {
    if (!game || !sprites_initialized || !game->bomb.active) return 1;

    const bomb_t *bomb = &game->bomb;
    uint8_t frame = bomb_explosion_frame(bomb);
    int center_index = explosion_center_sprite_index(frame);
    
    if (center_index >= 0 && center_index < SPRITE_CACHE_SIZE && scaled_sprite_cache[center_index].bytes) {
        xpm_image_t img = scaled_sprite_cache[center_index];
        int32_t draw_x = board_start_x + (bomb->board_pos.x * tile_size) + (tile_size / 2) - (img.width / 2);
        int32_t draw_y = board_start_y + (bomb->board_pos.y * tile_size) + (tile_size / 2) - (img.height / 2);
        hw_vbe_draw_xpm(video, img.bytes, img, draw_x, draw_y);
    }

    if (bomb->explosion_current_radius > 0) {
        bomb_draw_explosion_ray(video, game, board_start_x, board_start_y, tile_size, EXPLOSION_DIR_RIGHT, XPM_ROTATE_180, frame);
        bomb_draw_explosion_ray(video, game, board_start_x, board_start_y, tile_size, EXPLOSION_DIR_LEFT, XPM_ROTATE_0, frame);
        bomb_draw_explosion_ray(video, game, board_start_x, board_start_y, tile_size, EXPLOSION_DIR_DOWN, XPM_ROTATE_270, frame);
        bomb_draw_explosion_ray(video, game, board_start_x, board_start_y, tile_size, EXPLOSION_DIR_UP, XPM_ROTATE_90, frame);
    }
    
    return 0;
}

bool explosion_collides(const t_game_state *game, int32_t cell_x, int32_t cell_y) {
    if (!game || !game->bomb.active || game->bomb.state != BOMB_FIRE) return false;

    const bomb_t *bomb = &game->bomb;
    int32_t dx = cell_x - bomb->board_pos.x;
    int32_t dy = cell_y - bomb->board_pos.y;

    if (dx == 0 && dy == 0) return true; // Center collision
    if (bomb->explosion_current_radius == 0 || (dx != 0 && dy != 0)) return false; // Diagonal

    uint8_t dir = (dx > 0) ? EXPLOSION_DIR_RIGHT : (dx < 0) ? EXPLOSION_DIR_LEFT : (dy > 0) ? EXPLOSION_DIR_DOWN : EXPLOSION_DIR_UP;
    uint32_t dist = (dx != 0) ? (dx > 0 ? dx : -dx) : (dy > 0 ? dy : -dy);

    return (dist <= bomb->explosion_current_radius) && (dist <= explosion_reach(bomb, dir));
}
