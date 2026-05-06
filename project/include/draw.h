#ifndef LCOM_PROJECT_DRAW_H
#define LCOM_PROJECT_DRAW_H

#include "../lib/mouse/mouse.h"
#include "../lib/vbe/vbe.h"

int draw_mouse(hw_mouse_t *mouse, hw_video_t *video);
int draw_text_cursor(hw_mouse_t *mouse, hw_video_t *video);
int draw_string(hw_video_t *video, const char *str, int32_t x, int32_t y, uint32_t bg_color);
void init_sprite_cache();
void draw_board(hw_video_t *video);

#endif /* LCOM_PROJECT_DRAW_H */
