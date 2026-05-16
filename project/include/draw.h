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
int draw_player(player_t *player, hw_video_t *video, uint32_t size);

// Player movement helpers
void get_player_start_position(int player_id, uint32_t width, uint32_t height, int32_t *out_x, int32_t *out_y);
void update_player_movement(player_t *player, int32_t start_x, int32_t start_y);
void update_player_animation(player_t *player, uint32_t logical_ticks);

/* Grass and terrain drawing */
void draw_grass(hw_video_t *video, int32_t x, int32_t y, int sprite_index, uint32_t size);
void draw_wall(hw_video_t *video, int32_t x, int32_t y, int sprite_index, uint32_t size);
void draw_brick(hw_video_t *video, int32_t x, int32_t y, uint32_t size);

int decide_grass_sprite(const uint8_t *board, int rows, int cols, int x, int y);
int decide_wall_sprite(const uint8_t *board, int rows, int cols, int x, int y);

#endif /* LCOM_PROJECT_DRAW_H */
