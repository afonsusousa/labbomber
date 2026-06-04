#ifndef DRAW_ENEMY_H
#define DRAW_ENEMY_H

#include "game/game.h"
#include "core/hardware.h"

int draw_enemy(enemy_t *enemy, hw_video_t *video, int32_t board_start_x, int32_t board_start_y);

#endif // DRAW_ENEMY_H
