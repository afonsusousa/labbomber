#ifndef _DRAW_H_
#define _DRAW_H_

#include "../lib/mouse/mouse.h"
#include "../lib/vbe/vbe.h"

int draw_mouse(hw_mouse_t *mouse, hw_video_t *video);
int draw_string(hw_video_t *video, const char *str, int32_t x, int32_t y, uint32_t bg_color);
void draw_board(hw_video_t *video);

#endif /* _DRAW_H_ */
