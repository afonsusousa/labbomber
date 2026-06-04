#include "view/game/draw_player.h"
#include "view/assets_cache.h"
#include "game/game.h"
#include "vbe.h"
#include <stdint.h>
#include <lcom/xpm.h>

int draw_player(player_t *player, hw_video_t *video, t_game_state *game) {
    if (player == NULL || !sprites_initialized || !player->active) return 1;

    if (player->lives == 0) {
        xpm_image_t img = scaled_sprite_cache[SPRITE_PLAYER_DEATH];
        hw_vbe_draw_xpm(
            video,
            img.bytes,
            img,
            game->start_x + player->pos.x,
            game->start_y + player->pos.y
        );
        return 0;
    }

    if (game->match_state == MATCH_WON) {
        xpm_image_t img = scaled_sprite_cache[SPRITE_PLAYER_WIN];
        hw_vbe_draw_xpm(
            video,
            img.bytes,
            img,
            game->start_x + player->pos.x,
            game->start_y + player->pos.y
        );
        return 0;
    }

    // Blink if invincible
    if (player->invincibility_timer > 0 && (player->invincibility_timer / 5) % 2 == 0) {
        return 0;
    }

    int current_phase = player->animation_phase % 4;
    int current_direction = player->sprite_dir % 4;

    int sprite_index = SPRITE_PLAYER_1_STANDING + (current_phase * 4) + current_direction;

    if (sprite_index >= SPRITE_CACHE_SIZE || scaled_sprite_cache[sprite_index].bytes == NULL) {
        return 1;
    }

    xpm_image_t img = scaled_sprite_cache[sprite_index];

    int32_t draw_y = game->start_y + player->pos.y;

    // Sneak animation
    if (!player->is_moving && player->animation_phase == 1) {
        uint32_t sneak_amount = img.height / 20;
        draw_y += (int32_t)sneak_amount;
    }

    hw_vbe_draw_xpm(
        video,
        img.bytes,
        img,
        game->start_x + player->pos.x,
        draw_y
    );

    if (game->is_multiplayer) {
        int hat_sprite = (player == &game->players[0]) ? SPRITE_PLAYER_HAT_1 :
                         (player == &game->players[1]) ? SPRITE_PLAYER_HAT_2 : -1;

        if (hat_sprite != -1) {
            xpm_image_t hat_img = scaled_sprite_cache[hat_sprite];
            if (hat_img.bytes != NULL) {
                hw_vbe_draw_xpm(
                    video,
                    hat_img.bytes,
                    hat_img,
                    game->start_x + player->pos.x,
                    draw_y - (int32_t)(img.height * 0.7) - 1
                );
            }
        }
    }

    return 0;
}
