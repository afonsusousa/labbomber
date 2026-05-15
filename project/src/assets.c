#include "assets_cache.h"
#include "assets.h"
#include "glyphs.h"
#include <lcom/xpm.h>
#include <stdlib.h>
#include <stdbool.h>

/* Sprite indices (moved from draw.c) */
#define SPRITE_BOMB1                128
#define SPRITE_BOMB2                129
#define SPRITE_MOUSE_POINTER 130
#define SPRITE_MOUSE_TEXT    131
#define SPRITE_BOMB3                132
#define SPRITE_BRICK                133
#define SPRITE_EXPLOSION_1_CENTER   134
#define SPRITE_EXPLOSION_2_CENTER   135
#define SPRITE_EXPLOSION_3_ARM      136
#define SPRITE_EXPLOSION_3_CENTER   137
#define SPRITE_EXPLOSION_3_HAND     138
#define SPRITE_EXPLOSION_4_ARM      139
#define SPRITE_EXPLOSION_4_CENTER   140
#define SPRITE_EXPLOSION_4_HAND     141
#define SPRITE_EXPLOSION_5_ARM      142
#define SPRITE_EXPLOSION_5_CENTER   143
#define SPRITE_EXPLOSION_5_HAND     144
#define SPRITE_EXPLOSION_6_ARM      145
#define SPRITE_EXPLOSION_6_CENTER   146
#define SPRITE_EXPLOSION_6_HAND     147
#define SPRITE_FLOWER1              148
#define SPRITE_FLOWER2              149
#define SPRITE_FLOWER3              150
#define SPRITE_GRASS_LEFT_BORDER    151
#define SPRITE_GRASS_LEFT           152
#define SPRITE_GRASS_TOP_BORDER     153
#define SPRITE_GRASS_TOP_LEFT_BORDER 154
#define SPRITE_GRASS_TOP_LEFT       155
#define SPRITE_GRASS_TOP            156
#define SPRITE_GRASS                157
#define SPRITE_WALL1                158
#define SPRITE_WALL2                159
#define SPRITE_WALL3                160

/* Sprite cache storage */
xpm_image_t sprite_cache[SPRITE_CACHE_SIZE];
bool sprites_initialized = false;

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

    /* Load all game assets */
    xpm_load((xpm_map_t)bomb1, XPM_5_6_5, &sprite_cache[SPRITE_BOMB1]);
    xpm_load((xpm_map_t)bomb2, XPM_5_6_5, &sprite_cache[SPRITE_BOMB2]);
    xpm_load((xpm_map_t)bomb3, XPM_5_6_5, &sprite_cache[SPRITE_BOMB3]);
    xpm_load((xpm_map_t)brick, XPM_5_6_5, &sprite_cache[SPRITE_BRICK]);
    xpm_load((xpm_map_t)explosion_1_center, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_1_CENTER]);
    xpm_load((xpm_map_t)explosion_2_center, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_2_CENTER]);
    xpm_load((xpm_map_t)explosion_3_arm, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_3_ARM]);
    xpm_load((xpm_map_t)explosion_3_center, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_3_CENTER]);
    xpm_load((xpm_map_t)explosion_3_hand, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_3_HAND]);
    xpm_load((xpm_map_t)explosion_4_arm, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_4_ARM]);
    xpm_load((xpm_map_t)explosion_4_center, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_4_CENTER]);
    xpm_load((xpm_map_t)explosion_4_hand, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_4_HAND]);
    xpm_load((xpm_map_t)explosion_5_arm, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_5_ARM]);
    xpm_load((xpm_map_t)explosion_5_center, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_5_CENTER]);
    xpm_load((xpm_map_t)explosion_5_hand, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_5_HAND]);
    xpm_load((xpm_map_t)explosion_6_arm, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_6_ARM]);
    xpm_load((xpm_map_t)explosion_6_center, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_6_CENTER]);
    xpm_load((xpm_map_t)explosion_6_hand, XPM_5_6_5, &sprite_cache[SPRITE_EXPLOSION_6_HAND]);
    xpm_load((xpm_map_t)flower1, XPM_5_6_5, &sprite_cache[SPRITE_FLOWER1]);
    xpm_load((xpm_map_t)flower2, XPM_5_6_5, &sprite_cache[SPRITE_FLOWER2]);
    xpm_load((xpm_map_t)flower3, XPM_5_6_5, &sprite_cache[SPRITE_FLOWER3]);
    xpm_load((xpm_map_t)grass_left_broder, XPM_5_6_5, &sprite_cache[SPRITE_GRASS_LEFT_BORDER]);
    xpm_load((xpm_map_t)grass_left, XPM_5_6_5, &sprite_cache[SPRITE_GRASS_LEFT]);
    xpm_load((xpm_map_t)grass_top_border, XPM_5_6_5, &sprite_cache[SPRITE_GRASS_TOP_BORDER]);
    xpm_load((xpm_map_t)grass_top_left_border, XPM_5_6_5, &sprite_cache[SPRITE_GRASS_TOP_LEFT_BORDER]);
    xpm_load((xpm_map_t)grass_top_left, XPM_5_6_5, &sprite_cache[SPRITE_GRASS_TOP_LEFT]);
    xpm_load((xpm_map_t)grass_top, XPM_5_6_5, &sprite_cache[SPRITE_GRASS_TOP]);
    xpm_load((xpm_map_t)grass, XPM_5_6_5, &sprite_cache[SPRITE_GRASS]);
    xpm_load((xpm_map_t)wall1, XPM_5_6_5, &sprite_cache[SPRITE_WALL1]);
    xpm_load((xpm_map_t)wall2, XPM_5_6_5, &sprite_cache[SPRITE_WALL2]);
    xpm_load((xpm_map_t)wall3, XPM_5_6_5, &sprite_cache[SPRITE_WALL3]);

    sprites_initialized = true;
}
