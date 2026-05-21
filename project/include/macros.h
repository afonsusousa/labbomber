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
#define COL_TEAL       0x0410
#define COL_GRAY       0xC618
#define COL_LIGHT_GRAY 0xDEFB
#define COL_DARK_GRAY  0x8410
#define COL_WHITE      0xFFFF
#define COL_BLACK      0x0000

// =============================================================================
// Callback Helper Macros - Clean extraction of context pointers
// =============================================================================

#define CTX(state) ((t_ctx*)(state))
#define GUI(state) (&CTX(state)->gui)
#define GAME(state) (&CTX(state)->game)

#endif /* LCOM_PROJECT_MACROS_H */
