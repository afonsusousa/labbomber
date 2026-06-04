#include "view/game/draw_enemy.h"
#include "view/assets_cache.h"
#include "game/game.h"
#include "vbe.h"
#include <lcom/xpm.h>
#include <stdint.h>

int draw_enemy(enemy_t *enemy, hw_video_t *video, int32_t board_start_x, int32_t board_start_y) {
    if (enemy == NULL || !enemy->active || !sprites_initialized) return 1;

    // Blink if invincible or dying
    if (enemy->invincibility_timer > 0 && (enemy->invincibility_timer / 5) % 2 == 0) {
        return 0;
    }

    int current_phase = enemy->animation_phase % 4;
    int current_direction = enemy->sprite_dir % 4;

    int sprite_index = SPRITE_ENEMY_1_STANDING + (current_phase * 4) + current_direction;

    if (sprite_index >= SPRITE_CACHE_SIZE || scaled_sprite_cache[sprite_index].bytes == NULL)
        return 1;

    xpm_image_t img = scaled_sprite_cache[sprite_index];
    int32_t draw_x = board_start_x + enemy->pos.x;
    int32_t draw_y = board_start_y + enemy->pos.y;

    hw_vbe_draw_xpm(video, img.bytes, img, draw_x, draw_y);
    return 0;
}
