#ifndef DRAW_BOMBS_H
#define DRAW_BOMBS_H

#include "game/game.h"
#include "core/hardware.h"

int draw_bomb(hw_video_t *video, t_game_state *game);
int draw_bomb_explosion(hw_video_t *video, t_game_state *game, const bomb_t *bomb);

#endif // DRAW_BOMBS_H
