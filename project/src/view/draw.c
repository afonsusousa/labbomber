#include "view/draw.h"
#include "view/assets_cache.h"
#include "view/glyphs.h"
#include "vbe.h"
#include <lcom/xpm.h>
#include <stdlib.h>
#include <string.h>

int draw_mouse(hw_video_t *video, hw_mouse_t *mouse) {
    if (!sprites_initialized || sprite_cache[SPRITE_MOUSE_POINTER].bytes == NULL) {
        hw_vbe_draw_rect(video, mouse->x, mouse->y, 4, 4, 0xFFFFFF);
        return 0;
    }

    xpm_image_t img = sprite_cache[SPRITE_MOUSE_POINTER];
    hw_vbe_draw_xpm(video, img.bytes, img, mouse->x + (img.width / 2), mouse->y + (img.height / 2));
    return 0;
}

int draw_text_cursor(hw_video_t *video, hw_mouse_t *mouse) {
    if (!sprites_initialized || sprite_cache[SPRITE_MOUSE_TEXT].bytes == NULL) {
        hw_vbe_draw_vline(video, mouse->x, mouse->y - 8, 16, 0x000000);
        return 1;
    }

    xpm_image_t img = sprite_cache[SPRITE_MOUSE_TEXT];
    hw_vbe_draw_xpm(video, img.bytes, img, mouse->x, mouse->y);
    return 0;
}

#define LETTER_W 11

int draw_string(hw_video_t *video, const char *str, int32_t x, int32_t y, uint32_t color) {
    if (str == NULL || !sprites_initialized) return 1;

    for (int i = 0; str[i] != '\0'; i++) {
        unsigned char char_idx = (unsigned char)str[i];
        if (char_idx < 32 || char_idx > 126 || sprite_cache[char_idx].bytes == NULL)
            continue;

        xpm_image_t img = sprite_cache[char_idx];

        int32_t letter_center_x = x + (i * LETTER_W) + (img.width / 2);
        int32_t letter_center_y = y + (img.height / 2);

        hw_vbe_draw_xpm(video, img.bytes, img, letter_center_x, letter_center_y);
    }
    return 0;
}
