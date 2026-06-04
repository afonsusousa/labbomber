#include "view/game/draw_hud.h"
#include "view/assets_cache.h"
#include "game/game.h"
#include "vbe.h"
#include <stdint.h>
#include <lcom/xpm.h>

void draw_player_hearts(hw_video_t *video, t_game_state *game) {
    if (game == NULL || !sprites_initialized) return;

    const int heart_size = 24;
    const int heart_spacing = 4;
    scale_cached_sprite(SPRITE_HEART, heart_size, heart_size, 2);
    xpm_image_t heart = scaled_sprite_cache[SPRITE_HEART];

    if (game->players[PLAYER_1].active) {
        for (int i = 0; i < game->players[PLAYER_1].lives && i < 5; i++) {
            int32_t hx = 20 + i * (heart_size + heart_spacing);
            int32_t hy = 20;
            hw_vbe_draw_xpm(video, heart.bytes, heart, hx + heart_size/2, hy + heart_size/2);
        }
    }

    if (game->players[PLAYER_2].active) {
        uint32_t screen_w = video->screen_width;
        for (int i = 0; i < game->players[PLAYER_2].lives && i < 5; i++) {
            int32_t hx = screen_w - 20 - heart_size - i * (heart_size + heart_spacing);
            int32_t hy = 20;
            hw_vbe_draw_xpm(video, heart.bytes, heart, hx + heart_size/2, hy + heart_size/2);
        }
    }
}
