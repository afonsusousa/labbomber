#include "vbe.h"
#include <minix/syslib.h>
#include <minix/drivers.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

static int map_video_memory(hw_video_t *video, uint16_t mode) {
    vbe_mode_info_t         vmi;
    struct minix_mem_range  mr;
    unsigned int            vram_base, vram_size;

    if (vbe_get_mode_info(mode, &vmi) != 0) {
        printf("Failed to get VBE mode\n");
        return 1;
    }

    video->screen_width         = vmi.XResolution;
    video->screen_height        = vmi.YResolution;
    video->bytes_per_pixel      = (vmi.BitsPerPixel + 7) / 8;
    video->bytes_per_scanline   = vmi.BytesPerScanLine;

    vram_base = vmi.PhysBasePtr;
    vram_size = video->bytes_per_scanline * video->screen_height;

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

    video->fast_buffer = calloc(sizeof(char), vram_size);

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
    
    if (sys_int86(&reg) != 0) {
        printf("Set VBE mode failed (sys_int86)\n");
        return 1;
    }
    
    return 0;
}

int hw_vbe_init(hw_video_t *video, uint16_t mode) {
    if (video == NULL) return 1;
    
    if (map_video_memory(video, mode) != 0) return 1;
    if (set_vbe_mode(mode) != 0) return 1;
    
    return 0;
}

int hw_vbe_draw_pixel(hw_video_t *video, int32_t x, int32_t y, uint32_t color) {
    if (video == NULL || video->fast_buffer == NULL) return 1;
    if (x < 0 || y < 0 || (uint32_t)x >= video->screen_width || (uint32_t)y >= video->screen_height) return 0;

    uint8_t *pixel_ptr = video->fast_buffer 
                        + (y * video->bytes_per_scanline) 
                        + (x * video->bytes_per_pixel);

    if (video->bytes_per_pixel == 2) {
        *(uint16_t *)pixel_ptr = (uint16_t)color;
    } else if (video->bytes_per_pixel == 4) {
        *(uint32_t *)pixel_ptr = color;
    } else {
        memcpy(pixel_ptr, &color, video->bytes_per_pixel);
    }

    return 0;
}

int hw_vbe_draw_hline(hw_video_t *video, int32_t x, int32_t y, uint16_t length, uint32_t color) {
    if (video == NULL || video->fast_buffer == NULL) return 1;
    if (y < 0 || (uint32_t)y >= video->screen_height) return 0;

    int32_t x_start = (x < 0) ? 0 : x;
    int32_t x_end = x + length;
    if (x_end > (int32_t)video->screen_width) x_end = video->screen_width;
    if (x_start >= x_end) return 0;

    uint32_t draw_len = x_end - x_start;
    uint8_t *pixel_ptr = video->fast_buffer + (y * video->bytes_per_scanline) + (x_start * video->bytes_per_pixel);

    for (uint32_t i = 0; i < draw_len; i++) {
        if (video->bytes_per_pixel == 2) {
            *((uint16_t*)pixel_ptr) = (uint16_t)color;
        } else {
            *((uint32_t*)pixel_ptr) = color;
        }
        pixel_ptr += video->bytes_per_pixel;
    }

    return 0;
}

int hw_vbe_draw_vline(hw_video_t *video, int32_t x, int32_t y, uint16_t length, uint32_t color) {
    if (video == NULL || video->fast_buffer == NULL) return 1;

    if (x < 0 || (uint32_t)x >= video->screen_width) return 0;

    int32_t y_start = (y < 0) ? 0 : y;
    int32_t y_end = y + length;
    if (y_end > (int32_t)video->screen_height) y_end = video->screen_height;

    if (y_start >= y_end) return 0;

    uint8_t *pixel_ptr = video->fast_buffer 
                        + (y_start * video->bytes_per_scanline) 
                        + (x * video->bytes_per_pixel);

    uint32_t stride = video->bytes_per_scanline;
    uint8_t bpp = video->bytes_per_pixel;

    for (int32_t i = y_start; i < y_end; i++) {
        if (bpp == 2) {
            *(uint16_t *)pixel_ptr = (uint16_t)color;
        } else {
            *(uint32_t *)pixel_ptr = color;
        }
        
        // Move the pointer exactly one row down
        pixel_ptr += stride;
    }

    return 0;
}

int hw_vbe_draw_rect(hw_video_t *video, int32_t x, int32_t y, uint16_t width, uint16_t height, uint32_t color) {
    if (video == NULL || video->fast_buffer == NULL) return 1;
    if (width == 0 || height == 0) return 0;

    int32_t y_start = (y < 0) ? 0 : y;
    int32_t y_end = y + height;
    if (y_end > (int32_t)video->screen_height) y_end = video->screen_height;

    for (int32_t i = y_start; i < y_end; i++) {
        hw_vbe_draw_hline(video, x, i, width, color);
    }

    return 0;
}

int hw_vbe_draw_xpm(hw_video_t *video, uint8_t *map, xpm_image_t img, int32_t x, int32_t y) {
    if (video == NULL || video->fast_buffer == NULL || map == NULL) return 1;

    uint32_t transparent = xpm_transparency_color(img.type);
    uint8_t bpp = video->bytes_per_pixel;

    int32_t y_start = (y < 0) ? -y : 0;
    int32_t y_end = img.height;
    if (y + (int32_t)img.height > (int32_t)video->screen_height) {
        y_end = (int32_t)video->screen_height - y;
    }

    for (int32_t i = y_start; i < y_end; i++) {
        int32_t draw_y = y + i;
        
        uint8_t *vram_row_ptr = video->fast_buffer + (draw_y * video->bytes_per_scanline);
        uint8_t *map_ptr = map + (i * img.width * bpp);

        for (uint16_t j = 0; j < img.width; j++) {
            int32_t draw_x = x + j;

            if (draw_x >= 0 && (uint32_t)draw_x < video->screen_width) {
                uint32_t color;
                if (bpp == 2) color = *(uint16_t *)map_ptr;
                else color = *(uint32_t *)map_ptr;

                if (color != transparent) {
                    uint8_t *pixel_ptr = vram_row_ptr + (draw_x * bpp);
                    if (bpp == 2) *(uint16_t *)pixel_ptr = (uint16_t)color;
                    else *(uint32_t *)pixel_ptr = color;
                }
            }
            map_ptr += bpp;
        }
    }

    return 0;
}

int hw_vbe_clear_screen(hw_video_t *video, uint32_t color) {
    if (!video->double_buffer) return 1;
    
    uint32_t vram_size = video->bytes_per_scanline * video->screen_height;

    if (color == 0x000000) {
        memset(video->double_buffer, 0, vram_size);
    } else {
        for (uint32_t i = 0; i < video->screen_width * video->screen_height; i++) {
            memcpy(
                video->double_buffer + (i * video->bytes_per_pixel),
                &color,
                video->bytes_per_pixel
            );
        }
    }
    return 0;
}

void hw_vbe_flip_buffer(hw_video_t *video) {
    if (video == NULL || video->fast_buffer == NULL || video->frame_buffer == NULL) return;

    static int display_start_y = 0;
    uint32_t vram_size = video->bytes_per_scanline * video->screen_height;
    
    memcpy(video->double_buffer, video->fast_buffer, vram_size);

    display_start_y = (display_start_y == 0) ? video->screen_height : 0;

    reg86_t reg;
    memset(&reg, 0, sizeof(reg));
    reg.intno = BIOS_VID_INT;
    reg.ax = 0x4F07;  // VBE function: Set Display Start
    reg.bx = 0x0000;
    reg.cx = 0;
    reg.dx = (uint16_t)display_start_y; // Topmost line
    
    if (sys_int86(&reg) != 0) {
        return;
    }

    if (display_start_y == 0) {
        video->double_buffer = video->frame_buffer + vram_size;
    } else {
        video->double_buffer = video->frame_buffer;
    }
}

int vbe_exit() {
    reg86_t reg;
    memset(&reg, 0, sizeof(reg86_t));
    
    reg.intno = BIOS_VID_INT;
    reg.ah = BIOS_SET_VID_MODE; // 0x00
    reg.al = MINIX_TEXT_MODE;   // 0x03
    
    if (sys_int86(&reg) != 0) {
        return 1;
    }
    
    return 0;
}
