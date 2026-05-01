#include "draw.h"

int draw_mouse(hw_mouse_t *mouse, hw_video_t *video) {
    hw_vbe_draw_rect(video, mouse->x, mouse->y, 4, 4, 0xFFFFFF);
    return (0);
}
