#include "draw.h"
#include "assets_cache.h"
#include "game.h"
#include "vbe.h"
#include <lcom/xpm.h>
#include <stdint.h>

int draw_enemy(enemy_t *enemy, hw_video_t *video, int32_t board_start_x, int32_t board_start_y) {
    if (enemy == NULL || !sprites_initialized) return 1;

    int current_phase = enemy->animation_phase % 4;

    int sprite_index = SPRITE_ENEMY_1_STANDING + current_phase;

    if (sprite_index >= SPRITE_CACHE_SIZE || scaled_sprite_cache[sprite_index].bytes == NULL) {
        return 1;
    }

    xpm_image_t img = scaled_sprite_cache[sprite_index];

    int32_t draw_x = board_start_x + enemy->pos.x - (img.width / 2);
    int32_t draw_y = board_start_y + enemy->pos.y - (img.height / 2);

    hw_vbe_draw_xpm(video, img.bytes, img, draw_x, draw_y);

    return 0;
}

void update_enemy_animation(enemy_t *enemy, uint32_t logical_ticks) {
    if (enemy == NULL) return;

    if (logical_ticks % 30 == 0) {
        enemy->animation_phase = (enemy->animation_phase + 1) % 4;
    }
}
