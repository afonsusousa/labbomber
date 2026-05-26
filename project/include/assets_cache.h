/* assets_cache.h - sprite cache externs and initialization */
#ifndef ASSETS_CACHE_H
#define ASSETS_CACHE_H

#include <stddef.h>
#include <lcom/xpm.h>
#include <stdbool.h>

#define SPRITE_CACHE_SIZE 256

void init_sprite_cache(void);
void scale_cached_sprite(int index, uint32_t target_w, uint32_t target_h, uint8_t bpp);
void scale_all_game_sprites(uint32_t tile_size, uint32_t player_w, uint32_t player_h, uint8_t bpp);

extern xpm_image_t sprite_cache[SPRITE_CACHE_SIZE];
extern xpm_image_t scaled_sprite_cache[SPRITE_CACHE_SIZE]; // New scaled cache
extern bool sprites_initialized;

/* Common sprite indices */
#define SPRITE_BOMB1                128
#define SPRITE_BOMB2                129
#define SPRITE_MOUSE_POINTER        130
#define SPRITE_MOUSE_TEXT           131
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
#define SPRITE_PLAYER_1_STANDING    161
#define SPRITE_PLAYER_1_LEFT        162
#define SPRITE_PLAYER_1_RIGHT       163
#define SPRITE_PLAYER_1_BACK        164
#define SPRITE_PLAYER_2_STANDING    165
#define SPRITE_PLAYER_2_LEFT        166
#define SPRITE_PLAYER_2_RIGHT       167
#define SPRITE_PLAYER_2_BACK        168
#define SPRITE_PLAYER_3_STANDING    169
#define SPRITE_PLAYER_3_LEFT        170
#define SPRITE_PLAYER_3_RIGHT       171
#define SPRITE_PLAYER_3_BACK        172
#define SPRITE_PLAYER_4_STANDING    173
#define SPRITE_PLAYER_4_LEFT        174
#define SPRITE_PLAYER_4_RIGHT       175
#define SPRITE_PLAYER_4_BACK        176
#define SPRITE_ENEMY_1_STANDING     177
#define SPRITE_ENEMY_1_LEFT         178
#define SPRITE_ENEMY_1_RIGHT        179
#define SPRITE_ENEMY_1_BACK         180
#define SPRITE_ENEMY_2_STANDING     181
#define SPRITE_ENEMY_2_LEFT         182
#define SPRITE_ENEMY_2_RIGHT        183
#define SPRITE_ENEMY_2_BACK         184
#define SPRITE_ENEMY_3_STANDING     185
#define SPRITE_ENEMY_3_LEFT         186
#define SPRITE_ENEMY_3_RIGHT        187
#define SPRITE_ENEMY_3_BACK         188
#define SPRITE_ENEMY_4_STANDING     189
#define SPRITE_ENEMY_4_LEFT         190
#define SPRITE_ENEMY_4_RIGHT        191
#define SPRITE_ENEMY_4_BACK         192

static const int explosion_center_sprites[] = {
    SPRITE_EXPLOSION_1_CENTER,
    SPRITE_EXPLOSION_2_CENTER,
    SPRITE_EXPLOSION_3_CENTER,
    SPRITE_EXPLOSION_4_CENTER,
    SPRITE_EXPLOSION_5_CENTER,
    SPRITE_EXPLOSION_6_CENTER,
};

static const int explosion_arm_sprites[] = {
    -1,
    -1,
    SPRITE_EXPLOSION_3_ARM,
    SPRITE_EXPLOSION_4_ARM,
    SPRITE_EXPLOSION_5_ARM,
    SPRITE_EXPLOSION_6_ARM,
};

static const int explosion_hand_sprites[] = {
    -1,
    -1,
    SPRITE_EXPLOSION_3_HAND,
    SPRITE_EXPLOSION_4_HAND,
    SPRITE_EXPLOSION_5_HAND,
    SPRITE_EXPLOSION_6_HAND,
};

#endif /* ASSETS_CACHE_H */
