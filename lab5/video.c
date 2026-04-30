#include <lcom/lcf.h>
#include "video.h"

static char             *video_mem;         /* frame-buffer VM address */
static vbe_mode_info_t  vmi;                /* VBE mode info */
static unsigned         bytes_per_pixel;    /* Number of VRAM bytes per pixel */

int is_valid_mode(uint16_t mode)
{
    switch (mode) {
        case VBE_MODE_105:
        case VBE_MODE_110:
        case VBE_MODE_115:
        case VBE_MODE_14C:
        case VBE_MODE_11A:
            return (1);
        default:
            return (0);
    }
}

reg86_t vbe_reg() {
    reg86_t r86;
    memset(&r86, 0, sizeof(reg86_t));
    r86.intno = BIOS_VID_INT;
    return (r86);
}

int vg_init_mode(uint16_t mode) {
    if (!is_valid_mode(mode)) return 1;
    
    reg86_t reg = vbe_reg();

    reg.ah = VBE_CALL;
    reg.al = VBE_SET_MODE;
    reg.bx = mode | LINEAR_FRAMEBUFR;
    
    if (sys_int86(&reg) != 0) {
        printf("Error setting video mode\n");
        return 1;
    }

    return 0;
}

int vg_init_mem(uint16_t mode)
{
    int                     r;
    unsigned int            vram_base; /* VRAM's physical addresss */
    unsigned int            vram_size; /* VRAM's size, but you can use the frame-buffer size */
    struct minix_mem_range  mr;

    if (vbe_get_mode_info(mode, &vmi) != OK) return 1;

    bytes_per_pixel = (vmi.BitsPerPixel + 7) / 8;

    vram_base = vmi.PhysBasePtr;
    vram_size = vmi.BytesPerScanLine * vmi.YResolution;

    mr.mr_base = (phys_bytes)vram_base;
    mr.mr_limit = mr.mr_base + vram_size;

    if ((r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)) != OK)
        panic("sys_privctl (ADD_MEM) failed: %d\n", r);

    video_mem = vm_map_phys(SELF, (void *)mr.mr_base, vram_size);

    if (video_mem == MAP_FAILED)
        panic("couldn't map video memory");

    return 0;
}

void *(vg_init)(uint16_t mode) {
    if (vg_init_mem(mode) != 0) return NULL;
    if (vg_init_mode(mode) != 0) return NULL;
    return video_mem;
}

char* get_video_mem() {
    return video_mem;
}

vbe_mode_info_t get_vmi() {
    return vmi;
}

unsigned get_bytes_per_pixel() {
    return bytes_per_pixel;
}

uint16_t get_hres() {
    return vmi.XResolution;
}

uint16_t get_vres() {
    return vmi.YResolution;
}

unsigned get_bytes_per_scanline() {
    return vmi.BytesPerScanLine;
}
