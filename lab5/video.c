#include <lcom/lcf.h>
#include "video.h"

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
