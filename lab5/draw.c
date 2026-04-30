#include <lcom/lcf.h>
#include "video.h"

int draw_pixel(uint16_t x, uint16_t y, uint32_t color) {
    uint16_t    hres = get_hres();
    uint16_t    vres = get_vres();
    unsigned    bpp = get_bytes_per_pixel();
    char        *vmem = get_video_mem();
<<<<<<< HEAD

    if (x >= hres || y >= vres) return (1);

    memcpy(vmem + (hres * y + x) * bpp, &color, bpp);
=======
    unsigned    bpl = get_bytes_per_scanline();

    if (x >= hres || y >= vres) return (1);

    memcpy(vmem + y * bpl + x * bpp, &color, bpp);
>>>>>>> lab5-pretty
    
    return (0);
}

int draw_hline(uint16_t x, uint16_t y, uint16_t len, uint32_t color) {
    uint16_t    hres = get_hres();
    uint16_t    vres = get_vres();
<<<<<<< HEAD
    unsigned    bpp = get_bytes_per_pixel();
    char        *vmem = get_video_mem();
    char        *pos;
=======
>>>>>>> lab5-pretty

    if (x >= hres || y >= vres) return (0);
    if (x + len > hres) len = hres - x; // clip

<<<<<<< HEAD
    pos = vmem + (y * hres + x) * bpp;

    for (uint16_t i = 0; i < len; i++) {
        memcpy(pos, &color, bpp);
        pos += bpp;
    }
=======
    for (uint16_t i = 0; i < len; i++) 
        draw_pixel(x + i, y, color);
>>>>>>> lab5-pretty

    return (0);
}

int draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
    uint16_t    hres = get_hres();
    uint16_t    vres = get_vres();
<<<<<<< HEAD
    unsigned    bpp = get_bytes_per_pixel();
    char        *vmem = get_video_mem();
    char        *pos;
=======
>>>>>>> lab5-pretty

    if (x >= hres || y >= vres) return (0);
    if (x + width > hres) width = hres - x; // clip
    if (y + height > vres) height = vres - y;

<<<<<<< HEAD
    for (uint16_t yy = 0; yy < height; yy++) {
        pos = vmem + ((y + yy) * hres + x) * bpp;
        for (uint16_t xx = 0; xx < width; xx++) {
            memcpy(pos, &color, bpp);
            pos += bpp;
        }
    }
=======
    for (uint16_t yy = 0; yy < height; yy++)
        draw_hline(x, y + yy, width, color);
>>>>>>> lab5-pretty

    return (0);
}

int draw_xpm(uint8_t *map, xpm_image_t img, uint16_t x, uint16_t y) {
    uint16_t    hres = get_hres(), vres = get_vres();
    unsigned    bpp = get_bytes_per_pixel();
    char        *vmem = get_video_mem();
    uint32_t    transparent, color;
    uint8_t     *ptr = map;
<<<<<<< HEAD
=======
    unsigned    bpl = get_bytes_per_scanline();
>>>>>>> lab5-pretty

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
<<<<<<< HEAD
                memcpy(vmem + (draw_y * hres + draw_x) * bpp, &color, bpp);
=======
                memcpy(vmem + draw_y * bpl + draw_x * bpp, &color, bpp);
>>>>>>> lab5-pretty
        }
    }

    return (0);
}
