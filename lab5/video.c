#include <lcom/lcf.h>
#include "video.h"

static char *video_mem;
static vbe_mode_info_t vmi;
static unsigned bytes_per_pixel;
static unsigned h_res;
static unsigned v_res;

int vg_init_mode(uint16_t mode) {
    reg86_t reg;
    struct minix_mem_range mr;
    int r;

    if (vbe_get_mode_info(mode, &vmi) != 0)
        return 1;

    h_res = vmi.XResolution;
    v_res = vmi.YResolution;
    bytes_per_pixel = (vmi.BitsPerPixel + 7) / 8;

    unsigned int vram_base = vmi.PhysBasePtr;
    unsigned int vram_size = h_res * v_res * bytes_per_pixel;

    mr.mr_base = vram_base;
    mr.mr_limit = vram_base + vram_size;

    if ((r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)) != OK)
        panic("sys_privctl failed");

    video_mem = vm_map_phys(SELF, (void *)mr.mr_base, vram_size);

    if (video_mem == MAP_FAILED)
        panic("vm_map_phys failed");

    memset(&reg, 0, sizeof(reg));
    reg.intno = 0x10;
    reg.ah = 0x4F;
    reg.al = 0x02;
    reg.bx = mode | BIT(14);

    if (sys_int86(&reg) != OK)
        return 1;

    return 0;
}

int draw_pixel(uint16_t x, uint16_t y, uint32_t color) {
    if (x >= h_res || y >= v_res) return 1;

    unsigned index = (y * h_res + x) * bytes_per_pixel;

    video_mem[index] = (uint8_t) color;

    return 0;
}

int draw_hline(uint16_t x, uint16_t y, uint16_t len, uint32_t color) {
    return 0;
}

int draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
    return 0;
}

int draw_xpm(uint8_t *map, xpm_image_t img, uint16_t x, uint16_t y) {
    return 0;
}