#ifndef LCOM_PROJECT_DRAW_H
#define LCOM_PROJECT_DRAW_H

#include "../lib/mouse/mouse.h"
#include "../lib/vbe/vbe.h"
#include <stdint.h>
#include "game.h"

int draw_mouse(hw_mouse_t *mouse, hw_video_t *video);
int draw_text_cursor(hw_mouse_t *mouse, hw_video_t *video);
int draw_string(hw_video_t *video, const char *str, int32_t x, int32_t y, uint32_t bg_color);
void init_sprite_cache();

void set_date_seed(int day, int month, int year);
int draw_player(player_t *player, hw_video_t *video, int32_t board_start_x, int32_t board_start_y);
int draw_bomb(const bomb_t *bomb, uint32_t logical_ticks, hw_video_t *video, int32_t board_start_x, int32_t board_start_y, uint32_t tile_size);

/* Grass and terrain drawing */
void draw_grass(hw_video_t *video, int32_t x, int32_t y, int sprite_index);
void draw_wall(hw_video_t *video, int32_t x, int32_t y, int sprite_index);
void draw_brick(hw_video_t *video, int32_t x, int32_t y);

int decide_grass_sprite(const uint8_t *board, int rows, int cols, int x, int y);
int decide_wall_sprite(const uint8_t *board, int rows, int cols, int x, int y);

#endif /* LCOM_PROJECT_DRAW_H */
