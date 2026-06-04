#ifndef DRAW_MAP_H
#define DRAW_MAP_H

#include "core/hardware.h"
#include <stdint.h>
#include <stdbool.h>

void draw_grass(hw_video_t *video, int32_t x, int32_t y, int sprite_index);
void draw_wall(hw_video_t *video, int32_t x, int32_t y, int sprite_index);
void draw_brick(hw_video_t *video, int32_t x, int32_t y);
void draw_door(hw_video_t *video, int32_t x, int32_t y, bool open);

#endif // DRAW_MAP_H
