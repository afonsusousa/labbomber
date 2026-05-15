#ifndef LCOM_PROJECT_DRAW_H
#define LCOM_PROJECT_DRAW_H

#include "../lib/mouse/mouse.h"
#include "../lib/vbe/vbe.h"
#include <stdint.h>

int draw_mouse(hw_mouse_t *mouse, hw_video_t *video);
int draw_text_cursor(hw_mouse_t *mouse, hw_video_t *video);
int draw_string(hw_video_t *video, const char *str, int32_t x, int32_t y, uint32_t bg_color);
void init_sprite_cache();
void draw_board(hw_video_t *video);

// Grass and terrain drawing
void draw_grass(hw_video_t *video, int32_t x, int32_t y, int type, uint32_t size);
void draw_wall(hw_video_t *video, int32_t x, int32_t y, int sprite_index, uint32_t size);
void draw_brick(hw_video_t *video, int32_t x, int32_t y, uint32_t size);

// Sprite decision functions
int decide_grass_sprite(const uint8_t *board, int rows, int cols, int x, int y);
int decide_wall_sprite(const uint8_t *board, int rows, int cols, int x, int y);

#endif /* LCOM_PROJECT_DRAW_H */
