#ifndef LCOM_PROJECT_DRAW_H
#define LCOM_PROJECT_DRAW_H

#include "../lib/mouse/mouse.h"
#include "../lib/vbe/vbe.h"
#include <stdint.h>
#include "game.h"

int draw_mouse(hw_video_t *video, hw_mouse_t *mouse);
int draw_text_cursor(hw_video_t *video, hw_mouse_t *mouse);
int draw_string(hw_video_t *video, const char *str, int32_t x, int32_t y, uint32_t bg_color);
void init_sprite_cache();

void set_date_seed(int day, int month, int year);
int draw_player(player_t *player, hw_video_t *video, t_game_state *game);
int draw_enemy(enemy_t *enemy, hw_video_t *video, int32_t board_start_x, int32_t board_start_y);
int draw_bomb(hw_video_t *video, t_game_state *game);
int draw_bomb_explosion(hw_video_t *video, t_game_state *game, const bomb_t *bomb);

/* Grass and terrain drawing */
void draw_grass(hw_video_t *video, int32_t x, int32_t y, int sprite_index);
void draw_wall(hw_video_t *video, int32_t x, int32_t y, int sprite_index);
void draw_brick(hw_video_t *video, int32_t x, int32_t y);
void draw_door(hw_video_t *video, int32_t x, int32_t y, bool open);

int decide_grass_sprite(const uint8_t *board, int rows, int cols, int x, int y);
int decide_wall_sprite(const uint8_t *board, int rows, int cols, int x, int y);
void replace_color(uint8_t* map, uint32_t width, uint32_t height, uint8_t bpp, uint32_t original_color, uint32_t new_color);

#endif /* LCOM_PROJECT_DRAW_H */
