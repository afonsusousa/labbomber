#include <lcom/lcf.h>
#include "video.h"

int draw_pixel(uint16_t x, uint16_t y, uint32_t color) {
    uint16_t    hres = get_hres();
    uint16_t    vres = get_vres();
    unsigned    bpp = get_bytes_per_pixel();
    char        *vmem = get_video_mem();

    if (x >= hres || y >= vres) return (1);

    memcpy(vmem + (hres * y + x) * bpp, &color, bpp);
    
    return (0);
}

int draw_hline(uint16_t x, uint16_t y, uint16_t len, uint32_t color) {
    uint16_t    hres = get_hres();
    uint16_t    vres = get_vres();
    unsigned    bpp = get_bytes_per_pixel();
    char        *vmem = get_video_mem();
    char        *pos;

    if (x >= hres || y >= vres) return (0);
    if (x + len > hres) len = hres - x; // clip

    pos = vmem + (y * hres + x) * bpp;

    for (uint16_t i = 0; i < len; i++) {
        memcpy(pos, &color, bpp);
        pos += bpp;
    }

    return (0);
}

int draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
    uint16_t    hres = get_hres();
    uint16_t    vres = get_vres();
    unsigned    bpp = get_bytes_per_pixel();
    char        *vmem = get_video_mem();
    char        *pos;

    if (x >= hres || y >= vres) return (0);
    if (x + width > hres) width = hres - x; // clip
    if (y + height > vres) height = vres - y;

    for (uint16_t yy = 0; yy < height; yy++) {
        draw_hline(x, y + yy, width, color);
    }

    return (0);
}

int draw_xpm(uint8_t *map, xpm_image_t img, uint16_t x, uint16_t y) {
    uint16_t    hres = get_hres(), vres = get_vres();
    unsigned    bpp = get_bytes_per_pixel();
    char        *vmem = get_video_mem();
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
                memcpy(vmem + (draw_y * hres + draw_x) * bpp, &color, bpp);
        }
    }

    return (0);
}
