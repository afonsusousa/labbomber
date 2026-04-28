#include <lcom/lcf.h>

static char             *video_mem;         /* frame-buffer VM address */
static vbe_mode_info_t  vmi;                /* VBE mode info */
static unsigned         bytes_per_pixel;    /* Number of VRAM bytes per pixel */

int init_framebuffer(uint16_t mode)
{
    int                     r;
    unsigned int            vram_base; /* VRAM's physical addresss */
    unsigned int            vram_size; /* VRAM's size, but you can use the frame-buffer size */
    struct minix_mem_range  mr;

    if (vbe_get_mode_info(mode, &vmi) != OK) return 1;

    bytes_per_pixel = (vmi.BitsPerPixel + 7) / 8;

    vram_base = vmi.PhysBasePtr;
    vram_size = vmi.XResolution * vmi.YResolution * bytes_per_pixel;

    mr.mr_base = (phys_bytes)vram_base;
    mr.mr_limit = mr.mr_base + vram_size;

    if ((r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)) != OK)
        panic("sys_privctl (ADD_MEM) failed: %d\n", r);

    video_mem = vm_map_phys(SELF, (void *)mr.mr_base, vram_size);

    if (video_mem == MAP_FAILED)
        panic("couldn't map video memory");

    return 0;
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
