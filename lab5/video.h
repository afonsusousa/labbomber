#ifndef VIDEO_H
#define VIDEO_H

#include <lcom/lcf.h>

int vg_init_mode(uint16_t mode);

int draw_pixel(uint16_t x, uint16_t y, uint32_t color);
int draw_hline(uint16_t x, uint16_t y, uint16_t len, uint32_t color);
int draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);

int draw_xpm(uint8_t *map, xpm_image_t img, uint16_t x, uint16_t y);

#endif