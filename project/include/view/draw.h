#ifndef LCOM_PROJECT_DRAW_H
#define LCOM_PROJECT_DRAW_H

#include "mouse.h"
#include "vbe.h"
#include <stdint.h>
#include "game/game.h"

int draw_mouse(hw_video_t *video, hw_mouse_t *mouse);
int draw_text_cursor(hw_video_t *video, hw_mouse_t *mouse);
int draw_string(hw_video_t *video, const char *str, int32_t x, int32_t y, uint32_t bg_color);
void init_sprite_cache();

void replace_color(uint8_t* map, uint32_t width, uint32_t height, uint8_t bpp, uint32_t original_color, uint32_t new_color);

#endif /* LCOM_PROJECT_DRAW_H */
