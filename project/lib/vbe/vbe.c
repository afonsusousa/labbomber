#include "vbe.h"
#include "../../include/macros.h"
#include <minix/syslib.h>
#include <minix/drivers.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

// Inline helper to resolve color endianness and alignment instantly
static inline uint32_t get_pixel_color(uint8_t *src, uint8_t bpp) {
    if (bpp == 4) return *(uint32_t *)src;
    if (bpp == 2) return *(uint16_t *)src;
    // 24-bit fallback (ensures correct byte order)
    return src[0] | (src[1] << 8) | (src[2] << 16);
}

static int map_video_memory(hw_video_t *video, uint16_t mode) {
    vbe_mode_info_t vmi;
    struct minix_mem_range mr;

    if (vbe_get_mode_info(mode, &vmi) != 0) {
        printf("Failed to get VBE mode info\n");
        return 1;
    }

    video->screen_width       = vmi.XResolution;
    video->screen_height      = vmi.YResolution;
    video->bytes_per_pixel    = (vmi.BitsPerPixel + 7) / 8;
    video->bytes_per_scanline = vmi.BytesPerScanLine;

    unsigned int vram_base = vmi.PhysBasePtr;
    unsigned int vram_size = video->bytes_per_scanline * video->screen_height;

    mr.mr_base  = (phys_bytes)vram_base;
    mr.mr_limit = mr.mr_base + (vram_size * 2);

    if (sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr) != 0) {
        printf("Failed mapping memory through sys_privctl.\n");
        return 1;
    }

    video->frame_buffer = vm_map_phys(SELF, (void *)mr.mr_base, vram_size * 2);
    if (video->frame_buffer == MAP_FAILED) {
        printf("Failed mapping video memory.\n");
        return 1;
    }

    video->fast_buffer = calloc(1, vram_size);
    if (!video->fast_buffer) {
        printf("Failed to allocate fast_buffer.\n");
        return 1;
    }

    video->double_buffer = video->frame_buffer + vram_size;
    memset(video->frame_buffer, 0, vram_size * 2);

    return 0;
}

static int set_vbe_mode(uint16_t mode) {
    reg86_t reg;
    memset(&reg, 0, sizeof(reg86_t));
    
    reg.intno   = BIOS_VID_INT;
    reg.ah      = VBE_CALL;
    reg.al      = VBE_SET_MODE;
    reg.bx      = mode | LINEAR_FRAMEBUFR;
    
    if (sys_int86(&reg) != 0) return 1;
    return 0;
}

int hw_vbe_init(hw_video_t *video, uint16_t mode) {
    if (video == NULL) return 1;
    if (map_video_memory(video, mode) != 0) return 1;
    if (set_vbe_mode(mode) != 0) return 1;
    return 0;
}

int hw_vbe_draw_pixel(hw_video_t *video, int32_t x, int32_t y, uint32_t color) {
    if (!video || !video->fast_buffer) return 1;
    if (x < 0 || y < 0 || (uint32_t)x >= video->screen_width || (uint32_t)y >= video->screen_height) return 0;

    uint8_t *pixel_ptr = video->fast_buffer + (y * video->bytes_per_scanline) + (x * video->bytes_per_pixel);

    if (video->bytes_per_pixel == 4) *(uint32_t *)pixel_ptr = color;
    else if (video->bytes_per_pixel == 2) *(uint16_t *)pixel_ptr = (uint16_t)color;
    else memcpy(pixel_ptr, &color, video->bytes_per_pixel);

    return 0;
}

int hw_vbe_draw_hline(hw_video_t *video, int32_t x, int32_t y, uint16_t length, uint32_t color) {
    if (!video || !video->fast_buffer) return 1;
    if (y < 0 || (uint32_t)y >= video->screen_height || length == 0) return 0;

    int32_t x_start = (x < 0) ? 0 : x;
    int32_t x_end = x + length;
    if (x_end > (int32_t)video->screen_width) x_end = video->screen_width;
    if (x_start >= x_end) return 0;

    uint32_t draw_len = x_end - x_start;
    uint8_t *dst = video->fast_buffer + (y * video->bytes_per_scanline) + (x_start * video->bytes_per_pixel);

    // Optimized tight loop for continuous memory filling
    if (video->bytes_per_pixel == 4) {
        uint32_t *p = (uint32_t *)dst;
        while (draw_len--) *p++ = color;
    } else if (video->bytes_per_pixel == 2) {
        uint16_t *p = (uint16_t *)dst;
        while (draw_len--) *p++ = (uint16_t)color;
    } else {
        while (draw_len--) {
            memcpy(dst, &color, video->bytes_per_pixel);
            dst += video->bytes_per_pixel;
        }
    }
    return 0;
}

int hw_vbe_draw_vline(hw_video_t *video, int32_t x, int32_t y, uint16_t length, uint32_t color) {
    if (!video || !video->fast_buffer) return 1;
    if (x < 0 || (uint32_t)x >= video->screen_width || length == 0) return 0;

    int32_t y_start = (y < 0) ? 0 : y;
    int32_t y_end = y + length;
    if (y_end > (int32_t)video->screen_height) y_end = video->screen_height;
    if (y_start >= y_end) return 0;

    uint8_t *pixel_ptr = video->fast_buffer + (y_start * video->bytes_per_scanline) + (x * video->bytes_per_pixel);
    uint32_t stride = video->bytes_per_scanline;
    uint32_t count = y_end - y_start;

    // Strided pointer jumping
    if (video->bytes_per_pixel == 4) {
        while (count--) { *(uint32_t *)pixel_ptr = color; pixel_ptr += stride; }
    } else if (video->bytes_per_pixel == 2) {
        while (count--) { *(uint16_t *)pixel_ptr = (uint16_t)color; pixel_ptr += stride; }
    } else {
        while (count--) { memcpy(pixel_ptr, &color, video->bytes_per_pixel); pixel_ptr += stride; }
    }
    return 0;
}

int hw_vbe_draw_rect(hw_video_t *video, int32_t x, int32_t y, uint16_t width, uint16_t height, uint32_t color) {
    if (!video || !video->fast_buffer || width == 0 || height == 0) return 1;

    int32_t y_start = (y < 0) ? 0 : y;
    int32_t y_end = y + height;
    if (y_end > (int32_t)video->screen_height) y_end = video->screen_height;
    if (y_start >= y_end) return 0;

    // Draw the very first line
    hw_vbe_draw_hline(video, x, y_start, width, color);

    if (y_end - y_start == 1) return 0;

    // Memcpy Optimization: Copy the first line downwards
    int32_t x_start = (x < 0) ? 0 : x;
    int32_t x_end = x + width;
    if (x_end > (int32_t)video->screen_width) x_end = video->screen_width;
    if (x_start >= x_end) return 0;

    uint32_t copy_bytes = (x_end - x_start) * video->bytes_per_pixel;
    uint8_t *src_line = video->fast_buffer + (y_start * video->bytes_per_scanline) + (x_start * video->bytes_per_pixel);

    for (int32_t i = y_start + 1; i < y_end; i++) {
        uint8_t *dst_line = video->fast_buffer + (i * video->bytes_per_scanline) + (x_start * video->bytes_per_pixel);
        memcpy(dst_line, src_line, copy_bytes);
    }
    return 0;
}

int hw_vbe_draw_xpm(hw_video_t *video, uint8_t *map, xpm_image_t img, int32_t x, int32_t y) {
    if (!video || !video->fast_buffer || !map) return 1;

    x -= img.width / 2;
    y -= img.height / 2;

    uint32_t trans = xpm_transparency_color(img.type);
    uint8_t bpp = video->bytes_per_pixel;

    int32_t x0 = (x < 0) ? -x : 0;
    int32_t y0 = (y < 0) ? -y : 0;
    int32_t x1 = (x + (int32_t)img.width > video->screen_width) ? (int32_t)video->screen_width - x : img.width;
    int32_t y1 = (y + (int32_t)img.height > video->screen_height) ? (int32_t)video->screen_height - y : img.height;

    for (int32_t i = y0; i < y1; i++) {
        uint8_t *src_ptr = map + (i * img.width * bpp) + (x0 * bpp);
        uint8_t *dst_ptr = video->fast_buffer + ((y + i) * video->bytes_per_scanline) + ((x + x0) * bpp);

        for (int32_t j = x0; j < x1; j++) {
            uint32_t color = get_pixel_color(src_ptr, bpp);
            if (color != trans) {
                if (bpp == 4) *(uint32_t *)dst_ptr = color;
                else if (bpp == 2) *(uint16_t *)dst_ptr = (uint16_t)color;
                else memcpy(dst_ptr, &color, bpp);
            }
            src_ptr += bpp;
            dst_ptr += bpp;
        }
    }
    return 0;
}

// sister right here expects centered coordinates
int hw_vbe_draw_rotated_xpm(hw_video_t *video, uint8_t *map, xpm_image_t img, int32_t center_x, int32_t center_y, uint8_t rotation) {
    if (!video || !video->fast_buffer || !map) return 1;

    // if not rotating use standard faster top-left drawing function
    if (rotation == XPM_ROTATE_0) {
        return hw_vbe_draw_xpm(video, map, img, center_x, center_y);
    }

    uint32_t trans = xpm_transparency_color(img.type);
    uint8_t bpp = video->bytes_per_pixel;
    uint8_t rot = rotation & 3;

    int32_t pivot_x = (int32_t)img.width / 2;
    int32_t pivot_y = (int32_t)img.height / 2;

    for (uint32_t sy = 0; sy < img.height; sy++) {
        for (uint32_t sx = 0; sx < img.width; sx++) {
            uint8_t *src_ptr = map + ((sy * img.width + sx) * bpp);
            
            uint32_t color = 0;
            if (bpp == 4) color = *(uint32_t *)src_ptr;
            else if (bpp == 2) color = *(uint16_t *)src_ptr;
            else memcpy(&color, src_ptr, bpp);

            if (color == trans) continue;

            int32_t vx = (int32_t)sx - pivot_x;
            int32_t vy = (int32_t)sy - pivot_y;
            int32_t dx, dy;

            switch (rot) {
                case XPM_ROTATE_90:  dx = center_x - vy; dy = center_y + vx; break;
                case XPM_ROTATE_180: dx = center_x - vx; dy = center_y - vy; break;
                case XPM_ROTATE_270: dx = center_x + vy; dy = center_y - vx; break;
                default:             dx = center_x + vx; dy = center_y + vy; break;
            }

            if (dx < 0 || dy < 0 || (uint32_t)dx >= video->screen_width || (uint32_t)dy >= video->screen_height) continue;

            uint8_t *dst_ptr = video->fast_buffer + (dy * video->bytes_per_scanline) + (dx * bpp);
            
            if (bpp == 4) *(uint32_t *)dst_ptr = color;
            else if (bpp == 2) *(uint16_t *)dst_ptr = (uint16_t)color;
            else memcpy(dst_ptr, &color, bpp);
        }
    }

    return 0;
}

int hw_vbe_draw_scaled_xpm(hw_video_t *video, uint8_t *map, xpm_image_t img, int32_t x, int32_t y, uint32_t target_width, uint32_t target_height) {
    if (!video || !video->fast_buffer || !map || target_width == 0 || target_height == 0) return 1;

    uint32_t trans = xpm_transparency_color(img.type);
    uint8_t bpp = video->bytes_per_pixel;

    int32_t y_start = (y < 0) ? -y : 0;
    int32_t y_end = (y + (int32_t)target_height > video->screen_height) ? (int32_t)video->screen_height - y : (int32_t)target_height;
    int32_t x_start = (x < 0) ? -x : 0;
    int32_t x_end = (x + (int32_t)target_width > video->screen_width) ? (int32_t)video->screen_width - x : (int32_t)target_width;

    for (int32_t sy = y_start; sy < y_end; sy++) {
        uint32_t src_y = (sy * img.height) / target_height;
        uint8_t *src_row = map + (src_y * img.width * bpp);
        uint8_t *dst_row = video->fast_buffer + ((y + sy) * video->bytes_per_scanline) + (x * bpp);

        for (int32_t sx = x_start; sx < x_end; sx++) {
            uint32_t src_x = (sx * img.width) / target_width;
            uint8_t *src_ptr = src_row + (src_x * bpp);
            uint32_t color = get_pixel_color(src_ptr, bpp);

            if (color != trans) {
                uint8_t *dst_ptr = dst_row + (sx * bpp);
                if (bpp == 4) *(uint32_t *)dst_ptr = color;
                else if (bpp == 2) *(uint16_t *)dst_ptr = (uint16_t)color;
                else memcpy(dst_ptr, &color, bpp);
            }
        }
    }
    return 0;
}

int hw_vbe_clear_screen(hw_video_t *video, uint32_t color) {
    if (!video || !video->fast_buffer) return 1;
    
    uint32_t vram_size = video->bytes_per_scanline * video->screen_height;

    if (color == 0x000000) {
        memset(video->fast_buffer, 0, vram_size);
        return 0;
    }

    hw_vbe_draw_hline(video, 0, 0, video->screen_width, color);
    uint8_t *first_line = video->fast_buffer;

    for (uint32_t i = 1; i < video->screen_height; i++) {
        memcpy(video->fast_buffer + (i * video->bytes_per_scanline), first_line, video->bytes_per_scanline);
    }
    return 0;
}

void hw_vbe_flip_buffer(hw_video_t *video) {
    if (!video || !video->fast_buffer || !video->frame_buffer) return;

    static int display_start_y = 0;
    uint32_t vram_size = video->bytes_per_scanline * video->screen_height;
    
    // Copy rendered content to the hidden page of the double buffer
    memcpy(video->double_buffer, video->fast_buffer, vram_size);

    // Toggle hardware view
    display_start_y = (display_start_y == 0) ? video->screen_height : 0;

    reg86_t reg;
    memset(&reg, 0, sizeof(reg));
    reg.intno = BIOS_VID_INT;
    reg.ax = 0x4F07;
    reg.bx = 0x0000;
    reg.cx = 0;
    reg.dx = (uint16_t)display_start_y;
    
    if (sys_int86(&reg) != 0) return;

    // Swap pointers for next frame
    if (display_start_y == 0) video->double_buffer = video->frame_buffer + vram_size;
    else video->double_buffer = video->frame_buffer;
}

int vbe_exit() {
    reg86_t reg;
    memset(&reg, 0, sizeof(reg86_t));
    
    reg.intno = BIOS_VID_INT;
    reg.ah = BIOS_SET_VID_MODE; 
    reg.al = MINIX_TEXT_MODE;   
    
    return sys_int86(&reg);
}
