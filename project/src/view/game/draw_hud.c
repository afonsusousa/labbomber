#include "view/game/draw_hud.h"
#include "view/assets_cache.h"
#include "game/game.h"
#include "core/macros.h"
#include "vbe.h"
#include <stdint.h>
#include <lcom/xpm.h>

static void draw_powerups_for_player(hw_video_t *video, player_t *player, int32_t x, int32_t y, bool align_right) {
    if (video == NULL || player == NULL) return;

    int sprites[3];
    int count = 0;

    if (GET_POWERUP_REACH(player->powerups) > 0) {
        sprites[count++] = SPRITE_PLAYER_HAT_1;
    }

    if (GET_POWERUP_COUNT(player->powerups) > 0) {
        sprites[count++] = SPRITE_PLAYER_HAT_2;
    }

    if (GET_POWERUP_DRAG(player->powerups) > 0) { 
        sprites[count++] = SPRITE_PLAYER_HAT_3;
    }

    if (count == 0) return;

    int32_t spacing = 6;
    int32_t total_width = 0;

    for (int i = 0; i < count; i++) {
        xpm_image_t img = scaled_sprite_cache[sprites[i]];

        if (img.bytes == NULL) continue;

        if (total_width > 0) total_width += spacing;
        total_width += img.width;
    }

    if (align_right) {
        x -= total_width;
    }

    for (int i = 0; i < count; i++) {
        xpm_image_t img = scaled_sprite_cache[sprites[i]];

        if (img.bytes == NULL) continue;

        hw_vbe_draw_xpm(video, img.bytes, img, x, y);
        x += img.width + spacing;
    }
}

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

void draw_player_powerups(hw_video_t *video, t_game_state *game) {
    if (video == NULL || game == NULL || !sprites_initialized) return;

    int32_t heart_size = 24;
    int32_t powerup_y = 30 + heart_size + 8;

    if (game->players[PLAYER_1].active) {
        draw_powerups_for_player(
            video,
            &game->players[PLAYER_1],
            40,
            powerup_y,
            false
        );
    }

    if (game->players[PLAYER_2].active) {
        draw_powerups_for_player(
            video,
            &game->players[PLAYER_2],
            video->screen_width - 20,
            powerup_y,
            true
        );
    }
}
