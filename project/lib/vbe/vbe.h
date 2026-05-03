#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <machine/int86.h>
#include <lcom/vbe.h>
#include <lcom/xpm.h>
#include "../utils/utils.h"

typedef struct {
    uint16_t    screen_width;
    uint16_t    screen_height;
    uint8_t     bytes_per_pixel;
    uint8_t     *frame_buffer;   // this is the frame that will be flip flopped
    uint8_t     *double_buffer;
    unsigned    bytes_per_scanline;
} hw_video_t;

#define BIOS_VID_INT        0x10
#define VBE_CALL            0x4F
#define VBE_SET_MODE        0x02
#define LINEAR_FRAMEBUFR    BIT(14)
#define MINIX_TEXT_MODE     0x03
#define BIOS_SET_VID_MODE   0x00

#define VBE_MODE_105        0x105     // 1024x768, Indexed, 8 bpp
#define VBE_MODE_110        0x110     // 640x480, Direct color, 15 bpp
#define VBE_MODE_115        0x115     // 800x600, Direct color, 24 bpp
#define VBE_MODE_11A        0x11A     // 1280x1024, Direct color, 16 bpp
#define VBE_MODE_14C        0x14C     // 1152x864, Direct color, 32 bpp

int     hw_vbe_init(hw_video_t *video, uint16_t mode);
int     hw_vbe_clear_screen(hw_video_t *video, uint32_t color);
int     hw_vbe_draw_pixel(hw_video_t *video, int32_t x, int32_t y, uint32_t color);
int     hw_vbe_draw_hline(hw_video_t *video, int32_t x, int32_t y, uint16_t length, uint32_t color);
int     hw_vbe_draw_vline(hw_video_t *video, int32_t x, int32_t y, uint16_t length, uint32_t color);
int     hw_vbe_draw_rect(hw_video_t *video, int32_t x, int32_t y, uint16_t width, uint16_t height, uint32_t color);
int     hw_vbe_draw_xpm(hw_video_t *video, uint8_t *map, xpm_image_t img, int32_t x, int32_t y);
void    hw_vbe_flip_buffer(hw_video_t *video);
int     hw_vbe_exit();
