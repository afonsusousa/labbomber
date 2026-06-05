#ifndef DRAW_HUD_H
#define DRAW_HUD_H

#include "game/game.h"
#include "core/hardware.h"

void draw_player_hearts(hw_video_t *video, t_game_state *game);
void draw_player_powerups(hw_video_t *video, t_game_state *game);

#endif // DRAW_HUD_H
