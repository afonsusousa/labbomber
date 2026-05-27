#ifndef LIB_VBE_VBE_H
#define LIB_VBE_VBE_H
#include <stdint.h>
#include <stdbool.h>
#include <machine/int86.h>
#include <lcom/vbe.h>
#include <lcom/xpm.h>
#include "../utils/utils.h"
#include <string.h>

typedef struct {
    uint16_t    screen_width;
    uint16_t    screen_height;
    uint8_t     bytes_per_pixel;
    uint8_t     *frame_buffer;   // this is the frame that will be flip flopped
    uint8_t     *double_buffer;
    uint8_t     *fast_buffer;
    unsigned    bytes_per_scanline;
} hw_video_t;

// Inline helper to resolve color endianness and alignment instantly
static inline uint32_t vbe_get_pixel_color(uint8_t *src, uint8_t bpp) {
    if (bpp == 4) return *(uint32_t *)src;
    if (bpp == 2) return *(uint16_t *)src;
    // 24-bit fallback (ensures correct byte order)
    return src[0] | (src[1] << 8) | (src[2] << 16);
}

// Inline helper to set pixel color with proper bpp handling
static inline void vbe_set_pixel_color(uint8_t *dst, uint32_t color, uint8_t bpp) {
    if (bpp == 4) *(uint32_t *)dst = color;
    else if (bpp == 2) *(uint16_t *)dst = (uint16_t)color;
    else memcpy(dst, &color, bpp);
}

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
int     hw_vbe_draw_rotated_xpm(hw_video_t *video, uint8_t *map, xpm_image_t img, int32_t x, int32_t y, uint8_t rotation);
int     hw_vbe_draw_scaled_xpm(hw_video_t *video, uint8_t *map, xpm_image_t img, int32_t x, int32_t y, uint32_t target_width, uint32_t target_height);
void    hw_vbe_flip_buffer(hw_video_t *video);

/**
 * @brief Scales an image from a source buffer to a destination buffer.
 * 
 * @param src Pointer to the source image data.
 * @param dst Pointer to the destination buffer (must be pre-allocated).
 * @param src_w Source image width.
 * @param src_h Source image height.
 * @param dst_w Destination image width.
 * @param dst_h Destination image height.
 * @param bpp Bytes per pixel.
 * @return int 0 on success, non-zero otherwise.
 */
int vbe_scale_img(uint8_t *src, uint8_t *dst, uint32_t src_w, uint32_t src_h, uint32_t dst_w, uint32_t dst_h, uint8_t bpp);


#endif /* LIB_VBE_VBE_H */
