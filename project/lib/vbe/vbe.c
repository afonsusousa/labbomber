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
    mr.mr_limit = mr.mr_base + vram_size;

    if (sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr) != 0) {
        printf("Failed mapping memory through sys_privctl.\n");
        return 1;
    }

    video->frame_buffer = vm_map_phys(SELF, (void *)mr.mr_base, vram_size);
    if (video->frame_buffer == MAP_FAILED) {
        printf("Failed mapping video memory.\n");
        return 1;
    }

    video->double_buffer = malloc(vram_size);
    if (video->double_buffer == NULL) {
        printf("Failed allocating double buffer.\n");
        return 1;
    }
    hw_vbe_clear_screen(video, 0);
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

static inline uint32_t extract_color(const uint8_t *pixel_data, unsigned bytes_per_pixel) {
    uint32_t color = 0;
    
    for (unsigned k = 0; k < bytes_per_pixel; k++) {
        color |= (pixel_data[k]) << (k * 8);
    }
    
    return color;
}

int hw_vbe_draw_pixel(hw_video_t *video, uint16_t x, uint16_t y, uint32_t color) {
    if (x >= video->screen_width || y >= video->screen_height) return 0;

    uint8_t *pixel_ptr = video->double_buffer 
                        + (y * video->bytes_per_scanline) 
                        + (x * video->bytes_per_pixel);

    memcpy(pixel_ptr, &color, video->bytes_per_pixel);

    return 0;
}

int hw_vbe_draw_hline(hw_video_t *video, uint16_t x, uint16_t y, uint16_t length, uint32_t color) {
    if (video == NULL || video->double_buffer == NULL) return 1;

    for (uint16_t offset = 0; offset < length; offset++) {
        if (x + offset >= video->screen_width) break;
        hw_vbe_draw_pixel(video, x + offset, y, color);
    }

    return 0;
}

int hw_vbe_draw_vline(hw_video_t *video, uint16_t x, uint16_t y, uint16_t length, uint32_t color) {
    if (video == NULL || video->double_buffer == NULL) return 1;

    for (uint16_t offset = 0; offset < length; offset++) {
        if (y + offset >= video->screen_height) break;
        hw_vbe_draw_pixel(video, x, y + offset, color);
    }

    return 0;
}

int hw_vbe_draw_rect(hw_video_t *video, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
    if (video == NULL || video->double_buffer == NULL) return 1;
    if (width == 0 || height == 0) return 0;

    hw_vbe_draw_hline(video, x, y, width, color);

    if (height > 1) {
        hw_vbe_draw_hline(video, x, y + height - 1, width, color);
    }

    if (height > 2) {
        hw_vbe_draw_vline(video, x, y + 1, height - 2, color);

        if (width > 1) {
            hw_vbe_draw_vline(video, x + width - 1, y + 1, height - 2, color);
        }
    } else if (width > 1) {
        hw_vbe_draw_vline(video, x + width - 1, y, height, color);
    }

    return 0;
}

int hw_vbe_draw_xpm(hw_video_t *video, uint8_t *map, xpm_image_t img, uint16_t x, uint16_t y) {
    uint32_t    transparent, color = 0;
    uint16_t    draw_x, draw_y;
    uint8_t     *ptr; 

    if ((ptr = map) == NULL) return 1;

    transparent = xpm_transparency_color(img.type);

    for (uint16_t i = 0; i < img.height; i++) {
        draw_y = y + i;
        
        if (draw_y >= video->screen_height) break;

        for (uint16_t j = 0; j < img.width; j++) {
            draw_x = x + j;

            if (color != transparent) {
                hw_vbe_draw_pixel(
                    video,
                    draw_x,
                    draw_y,
                    extract_color(ptr, video->bytes_per_pixel)
                );
            }
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
    if (video->frame_buffer && video->double_buffer) {
        uint8_t *temp = video->frame_buffer;
        video->frame_buffer = video->double_buffer;
        video->double_buffer = temp;
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
