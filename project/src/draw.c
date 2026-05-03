#include "draw.h"
#include "letters.h"
#include <lcom/xpm.h>

static xpm_row_t const my_mouse_xpm[] = {
"12 19 3 1",
"  c None",
". c #000000",
"X c #FFFFFF",
".           ",
"..          ",
".X.         ",
".XX.        ",
".XXX.       ",
".XXXX.      ",
".XXXXX.     ",
".XXXXXX.    ",
".XXXXXXX.   ",
".XXXXXXXX.  ",
".XXXXXXXXX. ",
".XXXXXXXXXX.",
".XXXXXXX....",
".XX...XX.   ",
".X.   .X.   ",
"..     ..   ",
".      ..   ",
"        .   ",
"        .   "
};

int draw_mouse(hw_mouse_t *mouse, hw_video_t *video) {
    static bool loaded = false;
    static xpm_image_t img;
    static uint8_t *map = NULL;

    if (!loaded) {
        map = xpm_load((xpm_map_t) my_mouse_xpm, XPM_5_6_5, &img);
        loaded = true;
    }

    if (map != NULL) {
        hw_vbe_draw_xpm(video, map, img, mouse->x, mouse->y);
    } else {
        // Fallback to old square if xpm fails
        hw_vbe_draw_rect(video, mouse->x, mouse->y, 4, 4, 0xFFFFFF);
    }

    return 0;
}

int draw_string(hw_video_t *video, const char *str, uint16_t x, uint16_t y, uint32_t bg_color) {
    if (str == NULL) return 1;

    for (int i = 0; str[i] != '\0'; i++) {
        const xpm_row_t* xpm = get_letter_xpm(str[i]);
        if (xpm != NULL) {
            xpm_image_t img;
            uint8_t *map = xpm_load((xpm_map_t)xpm, XPM_5_6_5, &img);
            if (map != NULL) {
                uint32_t transparent = xpm_transparency_color(img.type);
                uint8_t *ptr = map;

                for (uint16_t h = 0; h < img.height; h++) {
                    for (uint16_t w = 0; w < img.width; w++) {
                        uint32_t color = 0;
                        for (unsigned k = 0; k < video->bytes_per_pixel; k++)
                            color |= (ptr[k]) << (k * 8);
                        uint16_t draw_x = x + (i * LETTER_W) + w;
                        uint16_t draw_y = y + h;

                        if (color != transparent)
                            hw_vbe_draw_pixel(video, draw_x, draw_y, color);
                        else
                            hw_vbe_draw_pixel(video, draw_x, draw_y, bg_color);
                        ptr += video->bytes_per_pixel;
                    }
                }
            }
        }
    }
    return 0;
}
