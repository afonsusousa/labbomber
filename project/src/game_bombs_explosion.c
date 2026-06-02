#include "game.h"
#include "vbe.h"
#include "macros.h"
#include "assets_cache.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define IN_BOUNDS(x, y) ((x) >= 0 && (x) < BOARD_COLS && (y) >= 0 && (y) < BOARD_ROWS)
#define BOMB_REACH(bomb, dir) (bomb->radius <  bomb->reach[dir]) ? bomb->radius :  bomb->reach[dir] 

static const int32_t DIR_X[EXPLOSION_DIR_COUNT] = { 1, -1, 0, 0 };
static const int32_t DIR_Y[EXPLOSION_DIR_COUNT] = { 0, 0, 1, -1 };

void bomb_clear_explosion(bomb_t *bomb) {
    if (!bomb) return;
    bomb->explosion_timer = 0;
    bomb->radius = 0;
    memset(bomb->reach, 0, sizeof(bomb->reach));
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

static void bomb_compute_reach(t_game_state *game) {
    bomb_t *bomb = &game->bomb;
    memset(bomb->reach, 0, sizeof(bomb->reach));

    for (uint8_t dir = 0; dir < EXPLOSION_DIR_COUNT; dir++) {
        for (uint8_t dist = 1; dist <= BOMB_EXPLOSION_RANGE; dist++) {
            int32_t x = bomb->board_pos.x + (DIR_X[dir] * dist);
            int32_t y = bomb->board_pos.y + (DIR_Y[dir] * dist);

            if (!IN_BOUNDS(x, y) || game->board[y * BOARD_COLS + x] == TILE_TYPE_WALL) {
                bomb->reach[dir] = dist - 1;
                break;
            }

            bomb->reach[dir] = dist;
            if (game->board[y * BOARD_COLS + x] == TILE_TYPE_BRICK) break; // Stop at destructible block
        }
    }
}

static void bomb_apply_explosion_contact(t_game_state *game, uint8_t radius) {
    if (radius == 0) return;

    for (uint8_t dir = 0; dir < EXPLOSION_DIR_COUNT; dir++) {
        uint8_t reach = game->bomb.reach[dir];
        
        if (radius >= reach && reach > 0) {
            int32_t x = game->bomb.board_pos.x + (DIR_X[dir] * reach);
            int32_t y = game->bomb.board_pos.y + (DIR_Y[dir] * reach);

            if (IN_BOUNDS(x, y) && game->board[y * BOARD_COLS + x] == TILE_TYPE_BRICK) {
                game->board[y * BOARD_COLS + x] = TILE_TYPE_GRASS;
            }
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
    bomb_compute_reach(game);
}

void bomb_update_explosion(t_game_state *game) {
    if (!game || !game->bomb.active || game->bomb.state != BOMB_FIRE) return;

    bomb_t *bomb = &game->bomb;
    if (bomb->explosion_timer == 0) {
        bomb_reset(bomb);
        return;
    }

    bomb->explosion_timer--;
    bomb->radius = (uint8_t)(bomb_explosion_frame(bomb) / 2);
    bomb_apply_explosion_contact(game, bomb->radius);

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

static void bomb_draw_explosion_ray(hw_video_t *video, const t_game_state *game, uint8_t dir, uint8_t rotation, uint8_t frame) {
    const bomb_t *bomb = &game->bomb;
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

int draw_bomb_explosion(hw_video_t *video, t_game_state *game) {
    if (!game || !sprites_initialized || !game->bomb.active) return 1;

    const bomb_t *bomb = &game->bomb;
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
        bomb_draw_explosion_ray(video, game, EXPLOSION_DIR_RIGHT, XPM_ROTATE_180, frame);
        bomb_draw_explosion_ray(video, game, EXPLOSION_DIR_LEFT, XPM_ROTATE_0, frame);
        bomb_draw_explosion_ray(video, game, EXPLOSION_DIR_DOWN, XPM_ROTATE_270, frame);
        bomb_draw_explosion_ray(video, game, EXPLOSION_DIR_UP, XPM_ROTATE_90, frame);
    }
    
    return 0;
}

bool explosion_collides(const t_game_state *game, int32_t cell_x, int32_t cell_y) {
    if (!game) return false;

    t_tuple explosion_pos = { GET_X(game, cell_x), GET_Y(game, cell_y) };
    t_tuple explosion_size = { (int32_t)game->tile_size, (int32_t)game->tile_size };

    for (int i = 0; i < MAX_PLAYERS; i++) {
        const player_t *p = &game->players[i];
        if (!p->active) continue;

        if (entity_overlaps(p->pos, p->size, explosion_pos, explosion_size)) return true;
    }

    for (int i = 0; i < game->enemy_count; i++) {
        const enemy_t *e = &game->enemies[i];
        if (!e->active) continue;

        if (entity_overlaps(e->pos, e->size, explosion_pos, explosion_size)) return true;
    }

    return false;
}
