#include "view/game/draw_map.h"
#include "view/assets_cache.h"
#include "vbe.h"
#include <stdint.h>
#include <stdbool.h>
#include <lcom/xpm.h>

void draw_grass(hw_video_t *video, int32_t x, int32_t y, int sprite_index) {
    if (!sprites_initialized || scaled_sprite_cache[sprite_index].bytes == NULL) return;

    xpm_image_t img = scaled_sprite_cache[sprite_index];
    hw_vbe_draw_xpm(video, img.bytes, img, x, y);
}

void draw_wall(hw_video_t *video, int32_t x, int32_t y, int sprite_index) {
    if (!sprites_initialized || scaled_sprite_cache[sprite_index].bytes == NULL) {
        hw_vbe_draw_rect(video, x - 8, y - 8, 16, 16, 0x666666);
        return;
    }

    xpm_image_t img = scaled_sprite_cache[sprite_index];
    hw_vbe_draw_xpm(video, img.bytes, img, x, y);
}

void draw_brick(hw_video_t *video, int32_t x, int32_t y) {
    if (!sprites_initialized || scaled_sprite_cache[SPRITE_BRICK].bytes == NULL) {
        hw_vbe_draw_rect(video, x - 8, y - 8, 16, 16, 0x884400);
        return;
    }

    xpm_image_t img = scaled_sprite_cache[SPRITE_BRICK];
    hw_vbe_draw_xpm(video, img.bytes, img, x, y);
}

void draw_door(hw_video_t *video, int32_t x, int32_t y, bool open) {
    int sprite = open ? SPRITE_DOOR_OPEN : SPRITE_DOOR_CLOSED;
    if (!sprites_initialized || scaled_sprite_cache[sprite].bytes == NULL) {
        hw_vbe_draw_rect(video, x - 8, y - 8, 16, 16, 0x00AA00);
        return;
    }

    xpm_image_t img = scaled_sprite_cache[sprite];
    hw_vbe_draw_xpm(video, img.bytes, img, x, y);
}
