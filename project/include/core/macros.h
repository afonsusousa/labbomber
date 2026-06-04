#ifndef LCOM_PROJECT_MACROS_H
#define LCOM_PROJECT_MACROS_H

// =============================================================================
// Essential Game & Rendering Constants
// =============================================================================

#define GAME_TILE_SIZE 16
#define GAME_SPEED 2
#define GAME_WIDTH 1024
#define GAME_HEIGHT 768
#define PLAYERS 2
#define SPRITE_CACHE_SIZE 256

// XPM rotation constants
#define XPM_ROTATE_0     0
#define XPM_ROTATE_90    1
#define XPM_ROTATE_180   2
#define XPM_ROTATE_270   3

// Player animation constants
#define ANIM_DIRECTIONS 4
#define ANIM_PHASES 4
#define ANIM_IDLE_TICK 30
#define ANIM_MOVE_TICK 15

// UI dimensions
#define UI_BAR_HEIGHT 40
#define BUTTON_HEIGHT 40

// Keyboard scancodes (make codes)
#define KB_ENTER 0x1C
#define KB_ENTER_BRK 0x9C
#define KB_SPACE 0x39
#define KB_BACKSPACE 0x0E
#define KB_LEFT 0x4B
#define KB_RIGHT 0x4D
#define KB_EXT_PREFIX 0xE0

// Colors
// (Moved to include/widget.h)

// =============================================================================
// Callback Helper Macros - Clean extraction of context pointers
// =============================================================================

#define CTX(state) ((t_ctx*)(state))
#define GUI(state) (&CTX(state)->gui)
#define GAME(state) (&CTX(state)->game)

/* Power-up bit-field macros */
#define POWERUP_REACH_MASK  0x03
#define POWERUP_REACH_SHIFT 0
#define POWERUP_COUNT_MASK  0x0C
#define POWERUP_COUNT_SHIFT 2

#define GET_POWERUP_REACH(p) (((p) & POWERUP_REACH_MASK) >> POWERUP_REACH_SHIFT)
#define GET_POWERUP_COUNT(p) (((p) & POWERUP_COUNT_MASK) >> POWERUP_COUNT_SHIFT)

#define SET_POWERUP_REACH(p, v) (p) = ((p) & ~POWERUP_REACH_MASK) | (((v) << POWERUP_REACH_SHIFT) & POWERUP_REACH_MASK)
#define SET_POWERUP_COUNT(p, v) (p) = ((p) & ~POWERUP_COUNT_MASK) | (((v) << POWERUP_COUNT_SHIFT) & POWERUP_COUNT_MASK)

#endif /* LCOM_PROJECT_MACROS_H */
