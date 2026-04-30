#include <lcom/lcf.h>
#include "video.h"

static uint16_t hres = 0;
static uint16_t vres = 0;
static unsigned bpp = 0;
static char     *vmem = NULL;
static unsigned bpl = 0;

static void init_globals() {
    if (vmem != NULL) return;
    hres = get_hres();
    vres = get_vres();
    bpp = get_bytes_per_pixel();
    vmem = get_video_mem();
    bpl = get_bytes_per_scanline();
}

static inline void draw_pixel(uint16_t x, uint16_t y, uint32_t color) {
    if (x >= hres || y >= vres) return;
    memcpy(vmem + y * bpl + x * bpp, &color, bpp);
}

static inline void draw_hline(uint16_t x, uint16_t y, uint16_t len, uint32_t color) {
    if (x >= hres || y >= vres) return;
    if (x + len > hres) len = hres - x; // clip

    for (uint16_t i = 0; i < len; i++) 
        draw_pixel(x + i, y, color);
}

int draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
    init_globals();

    if (x >= hres || y >= vres) return (1);
    if (x + width > hres) width = hres - x; // clip
    if (y + height > vres) height = vres - y;

    for (uint16_t yy = 0; yy < height; yy++)
        draw_hline(x, y + yy, width, color);

    return (0);
}

int draw_xpm(uint8_t *map, xpm_image_t img, uint16_t x, uint16_t y) {
    init_globals();
    uint32_t    transparent, color;
    uint8_t     *ptr = map;

    if (map == NULL) return (1);

    transparent = xpm_transparency_color(img.type);

    for (uint16_t i = 0; i < img.height; i++) {
        uint16_t draw_y = y + i;
        if (draw_y >= vres) break;

        for (uint16_t j = 0; j < img.width; j++) {
            color = 0;
            for (unsigned k = 0; k < bpp; k++) color |= (*ptr++) << (k * 8);

            uint16_t draw_x = x + j;
            if (draw_x >= hres) continue;

            if (color != transparent)
                memcpy(vmem + draw_y * bpl + draw_x * bpp, &color, bpp);
        }
    }

    return (0);
}
