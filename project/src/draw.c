#include "draw.h"
#include "glyphs.h"
#include <lcom/xpm.h>
#include <stdlib.h>
#include <stdbool.h>

// --- Sprite Cache Definitions ---
#define SPRITE_CACHE_SIZE 256 
#define SPRITE_MOUSE_POINTER 130
#define SPRITE_MOUSE_TEXT    131

static xpm_image_t sprite_cache[SPRITE_CACHE_SIZE]; 
bool sprites_initialized = false;

// --- Initialization ---

void init_sprite_cache() {
    for (int i = 32; i < 127; i++) {
        const xpm_row_t* xpm = get_letter_xpm(i);
        if (xpm != NULL) {
            xpm_load((xpm_map_t)xpm, XPM_5_6_5, &sprite_cache[i]);
        } else {
            sprite_cache[i].bytes = NULL; 
        }
    }

    xpm_load((xpm_map_t)my_mouse_xpm, XPM_5_6_5, &sprite_cache[SPRITE_MOUSE_POINTER]);
    xpm_load((xpm_map_t)text_cursor_xpm, XPM_5_6_5, &sprite_cache[SPRITE_MOUSE_TEXT]);

    sprites_initialized = true;
}

int draw_mouse(hw_mouse_t *mouse, hw_video_t *video) {
    if (!sprites_initialized || sprite_cache[SPRITE_MOUSE_POINTER].bytes == NULL) {
        hw_vbe_draw_rect(video, mouse->x, mouse->y, 4, 4, 0xFFFFFF);
        return 1;
    }

    xpm_image_t img = sprite_cache[SPRITE_MOUSE_POINTER];
    hw_vbe_draw_xpm(video, img.bytes, img, mouse->x, mouse->y);
    return 0;
}

int draw_text_cursor(hw_mouse_t *mouse, hw_video_t *video) {
    if (!sprites_initialized || sprite_cache[SPRITE_MOUSE_TEXT].bytes == NULL) {
        hw_vbe_draw_vline(video, mouse->x, mouse->y - 8, 16, 0x000000);
        return 1;
    }

    xpm_image_t img = sprite_cache[SPRITE_MOUSE_TEXT];
    hw_vbe_draw_xpm(video, img.bytes, img, mouse->x - (img.width / 2), mouse->y - (img.height / 2));
    return 0;
}

int draw_string(hw_video_t *video, const char *str, int32_t x, int32_t y, uint32_t bg_color) {
    if (str == NULL || !sprites_initialized) return 1;

    for (int i = 0; str[i] != '\0'; i++) {
        unsigned char char_idx = (unsigned char)str[i];
        if (char_idx < 32 || char_idx > 126 || sprite_cache[char_idx].bytes == NULL)
            continue;
        xpm_image_t img = sprite_cache[char_idx];
        hw_vbe_draw_xpm(video, img.bytes, img, x + (i * LETTER_W), y);
    }

    return 0;
}

// NICE -  ISTO VAI PARA A GAME.C na parte de DRAW, o board em si vai para o t_game_state na game.h
// + o generateBoard vai para a init_game()
#include "vbe.h"
#include "rtc.h"
#include "board_generator.h"
void draw_board(hw_video_t *video) {
    static char board[11 * 17];
    static bool generated = false;

    if (!generated) {
        hw_rtc_t rtc;
        hw_rtc_init(&rtc);
        hw_rtc_get_time(&rtc);

        generateBoard(board, rtc.day, rtc.month, rtc.year);
        generated = true;
    }

    int tile = 32;

    for (int y = 0; y < 11; y++) {
        for (int x = 0; x < 17; x++) {

            int val = board[y*17 + x];

            int px = x * tile;
            int py = y * tile;

            if (val == 1)
                hw_vbe_draw_rect(video, px, py, tile, tile, 0xAAAAAA);
            else if (val == 2)
                hw_vbe_draw_rect(video, px, py, tile, tile, 0x884400);
            else
                hw_vbe_draw_rect(video, px, py, tile, tile, 0x000000);
        }
    }
}
