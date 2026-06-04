#ifndef MODELS_ENTITY_H
#define MODELS_ENTITY_H

#include "models/types.h"
#include "core/macros.h"
#include <stdint.h>
#include <stdbool.h>

#define MAX_PLAYERS 2
#define PLAYER_1    0
#define PLAYER_2    1

#define MAX_ENEMIES          10
#define MIN_DIST_FROM_PLAYER  6
#define ENEMY_SPEED           1

#define GAME_TICKS_PER_SECOND 60
#define INVINCIBILITY_TICKS   (GAME_TICKS_PER_SECOND * 3)

struct s_game_state;

typedef struct s_entity {
    char        name[32];
    t_tuple     pos;
    t_tuple     board_pos;
    direction_t dir;
    direction_t sprite_dir;
    uint8_t     animation_phase;
    bool        is_moving;
    bool        active;
    uint8_t     movement_stack[4];
    uint8_t     stack_count;
    uint8_t     speed;
    t_tuple     size;
    uint8_t     lives;
    uint8_t     bomb_max;
    uint8_t     bomb_available;
    uint8_t     powerups;
    uint32_t    invincibility_timer;
    void (*on_snap)(struct s_game_state *game, struct s_entity *entity);
} entity_t;

typedef entity_t player_t;
typedef entity_t enemy_t;

#endif /* MODELS_ENTITY_H */
