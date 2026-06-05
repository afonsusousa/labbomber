/**
 * @file widget.h
 * @brief Widget system for the GUI, including types, structures, and lifecycle functions.
 */

#ifndef LCOM_PROJECT_WIDGET_H
#define LCOM_PROJECT_WIDGET_H

#include <stdint.h>
#include <stdbool.h>
#include "vbe.h"

// Classic Windows 95 16-bit RGB 5:6:5 Color Palette
#define W95_GRAY        0xC618  /**< #C0C0C0 */
#define W95_WHITE       0xFFFF  /**< #FFFFFF */
#define W95_DARK_GRAY   0x8410  /**< #808080 */
#define W95_LIGHT_GRAY  0xDEFB  /**< #DCDCDC */
#define W95_BLACK       0x0000  /**< #000000 */
#define W95_BLUE        0x0010  /**< #000080 (Classic Title Bar) */

// Simplified UI theme palette
#define UI_BG_COLOR         W95_DARK_GRAY
#define UI_PANEL_COLOR      W95_GRAY
#define UI_PANEL_HOVER      W95_GRAY
#define UI_PANEL_PRESSED    W95_GRAY
#define UI_PANEL_FLASH      W95_LIGHT_GRAY
#define UI_ACCENT_COLOR     W95_BLUE
#define UI_TEXT_COLOR       W95_BLACK
#define UI_TITLE_BAR_COLOR  W95_BLUE
#define UI_BORDER_LIGHT     W95_WHITE
#define UI_BORDER_DARK      W95_BLACK
#define UI_CURSOR_COLOR     W95_BLACK

// UI BUTTON COLORS
#define BTN_RADIUS      12
#define BTN_FILL        0x6a5abf
#define BTN_FILL_HOVER  0x533bd1
#define BTN_FILL_PRESS  0x513db8

// DIALOG CONSTANTS
#define DIALOG_TITLE_HEIGHT    20
#define DIALOG_TITLE_Y_OFFSET  6
#define DIALOG_TITLE_X_OFFSET  4

/**
 * @brief Alignment options for widgets.
 */
typedef enum {
    ALIGN_START,
    ALIGN_CENTER,
    ALIGN_END
} e_alignment;

/**
 * @brief Supported widget types.
 */
typedef enum {
    DIALOG,     /**< Standard window with title bar */
    BUTTON,     /**< Clickable button */
    TEXT,       /**< Static text display */
    TEXT_INPUT, /**< Editable text field */
    CANVAS,     /**< Generic drawing area */
    GAME,       /**< Main game viewport */
    OVERLAY     /**< Container for modal dialogs */
} e_widget_type;

/**
 * @brief Widget status flags.
 */
typedef enum {
    WIDGET_FLAG_ACTIVE  = 1u << 0,    /**< Widget is active and should be updated */
    WIDGET_FLAG_NO_LAYOUT = 1u << 5,  /**< Widget should be ignored by layout managers */
    WIDGET_FLAG_NO_FOCUS = 1u << 6,   /**< Widget cannot receive input focus */
} e_widget_flag;

/** @name Widget State Macros
 *  Helpers to check and modify widget state.
 *  @{
 */
#define WIDGET_HAS_FLAG(widget, flag) ((widget)->flags & (flag))
#define WIDGET_SET_FLAG(widget, flag) ((widget)->flags |= (flag))
#define WIDGET_UNSET_FLAG(widget, flag) ((widget)->flags &= ~(flag))
#define WIDGET_ASSIGN_FLAG(widget, flag, value) \
    ((widget)->flags = ((widget)->flags & ~(flag)) | ((value) ? (flag) : 0))

#define WIDGET_IS_ACTIVE(widget) WIDGET_HAS_FLAG((widget), WIDGET_FLAG_ACTIVE)
#define WIDGET_SET_ACTIVE(widget, value) WIDGET_ASSIGN_FLAG((widget), WIDGET_FLAG_ACTIVE, (value))

#define WIDGET_IS_FOCUSED(gui, widget) ((gui)->input.focused == (widget))
#define WIDGET_IS_HOVERED(gui, widget) ((gui)->input.hovered == (widget))
#define WIDGET_IS_CLICKED(gui, widget) ((gui)->input.clicked_widget == (widget))

#define WIDGET_SET_FOCUSED(gui, widget) ((gui)->input.focused = (widget))
#define WIDGET_SET_HOVERED(gui, widget) ((gui)->input.hovered = (widget))
#define WIDGET_SET_CLICKED(gui, widget) ((gui)->input.clicked_widget = (widget))

/** @brief Checks if a widget is eligible for focus. */
#define WIDGET_CAN_RECEIVE_FOCUS(widget) \
    ((widget) != NULL && !(WIDGET_HAS_FLAG((widget), WIDGET_FLAG_NO_FOCUS)) && ((widget)->type == BUTTON || (widget)->type == TEXT_INPUT))
/** @} */

/**
 * @brief Base structure for all GUI elements.
 */
typedef struct s_widget {
    e_widget_type   type;   /**< Type of the widget */

    int32_t         x, y;           /**< Position relative to parent */
    int32_t         abs_x, abs_y;   /**< Absolute screen position (computed) */
    uint32_t        height, width;  /**< Dimensions in pixels */

    e_alignment     h_align;        /**< Horizontal alignment */
    e_alignment     v_align;        /**< Vertical alignment */

    char            *name;          /**< Unique identifier */

    struct s_widget *parent;        /**< Pointer to parent widget */
    struct s_widget *children;      /**< Linked list of children */
    struct s_widget *next;          /**< Next sibling */
    struct s_widget *prev;          /**< Previous sibling */

    uint32_t        flags;          /**< Status flags */
    union {
        struct {
            char        *label;
            int32_t     action_delay_timer;
            void        (*on_click_action)(struct s_widget*, void*);
        } button; /**< Data for BUTTON type */

        struct {
            char        *text;
            char text_buf[64];
        } text_display; /**< Data for TEXT type */

        struct {
            char        *buffer;
            uint32_t    max_length;
            uint32_t    cursor_pos;
            uint32_t    len;
            int32_t     selection_start; // -1 if no selection
            bool        cursor_visible;
            uint32_t    blink_timer;
            uint32_t    flash_timer;
            bool        was_focused;
        } text_input; /**< Data for TEXT_INPUT type */

        struct {
            char            *title;
            struct s_widget *close_button;
        } dialog; /**< Data for DIALOG type */
    } data; /**< Type-specific data */

    /** @name Callbacks */
    /** @{ */
    void (*draw)(struct s_widget *self, hw_video_t *video, void *state);     /**< Logic to render the widget */
    void (*on_click)(struct s_widget *self, void *state);                   /**< Called when clicked and released */
    void (*on_press)(struct s_widget *self, void *state);                   /**< Called when mouse button is pressed */
    void (*on_drag)(struct s_widget *self, void *state);                    /**< Called during a drag operation */
    void (*on_hover)(struct s_widget *self, void *state);                   /**< Called when mouse is over widget */
    void (*on_key_press)(struct s_widget *self, uint8_t scancode, void *state); /**< Called on keyboard input */
    void (*on_tick)(struct s_widget *self, void *state);                    /**< Called every frame/timer tick */
    void (*on_quit)(struct s_widget *self, void *state);                    /**< Called on ESC or close event */

    void (*on_destroy)(struct s_widget *self);                              /**< Lifecycle hook for cleanup */
    /** @} */

} t_widget;

/**
 * @brief Creates a new widget.
 * @return Pointer to the new widget.
 */
t_widget*   widget_create(e_widget_type type, int32_t x, int32_t y, uint32_t width, uint32_t height, const char *name);

/** @brief Adds a child widget to a parent. */
void        widget_add_child(t_widget *parent, t_widget *child);

/** @brief Destroys a widget and all its children. */
void        widget_destroy(t_widget *widget);

/** @brief Sets the relative position of a widget. */
t_widget*   widget_set_position(t_widget *widget, int32_t x, int32_t y);

/** @brief Finds the front-most widget at screen coordinates (x, y). */
t_widget*   widget_get_at(t_widget *root, int32_t x, int32_t y);

/** @brief Searches for a child widget by its name. */
t_widget*   widget_get_child_by_name(t_widget *root, const char *name);

/** @brief Recursively updates absolute coordinates based on parent positions. */
void        widget_update_abs_coords(t_widget *widget);

/** @name Rendering Utilities
 *  @{
 */
/** @brief Draws a beveled Windows 95 style border. */
void        draw_win95_border(hw_video_t *video, int32_t x, int32_t y, uint16_t w, uint16_t h, bool sunken);
/** @brief Draws a focus indicator outline. */
void        draw_focus_outline(hw_video_t *video, int32_t x, int32_t y, uint16_t w, uint16_t h, uint32_t color);
/** @} */

/** @brief Dispatches the draw call to a widget and its children. */
void        widget_draw(t_widget *widget, hw_video_t *video, void *state);

/** @brief Dispatches the tick call to a widget and its children. */
void        widget_tick(t_widget *widget, void *state);

/** @name Focus Navigation
 *  @{
 */
t_widget*   widget_first_focusable(t_widget *root);
t_widget*   widget_get_next_focusable_sibling(t_widget *widget);
t_widget*   widget_get_prev_focusable_sibling(t_widget *widget);
/** @} */

/** @name Internal Draw Functions
 *  @{
 */
void        draw_canvas(struct s_widget *self, hw_video_t *video, void *state);
void        draw_button(struct s_widget *self, hw_video_t *video, void *state);
void        draw_text(struct s_widget *self, hw_video_t *video, void *state);
void        draw_text_input(struct s_widget *self, hw_video_t *video, void *state);
void        draw_dialog(struct s_widget *self, hw_video_t *video, void *state);
void        draw_game_board(struct s_widget *self, hw_video_t *video, void *state);
/** @} */

/** @name Builder Helpers
 *  Convenience functions to create and add widgets in one step.
 *  @{
 */
t_widget* widget_add_button(t_widget *parent, int32_t x, int32_t y, uint32_t w, uint32_t h, const char *label, void (*on_click)(t_widget*, void*), const char *name);
t_widget* widget_add_text_input(t_widget *parent, int32_t x, int32_t y, uint32_t w, uint32_t h, const char *default_text, void (*on_click)(t_widget*, void*), const char *name);
t_widget* widget_add_text(t_widget *parent, int32_t x, int32_t y, uint32_t w, uint32_t h, const char *text, const char *name);
/** @} */

/** @name Overlay / Dialog Helpers
 *  @{
 */
t_widget* widget_create_overlay(uint32_t screen_w, uint32_t screen_h, void (*on_quit)(t_widget*, void*), const char *name);
t_widget* widget_add_dialog(t_widget *parent, const char *title, uint32_t w, uint32_t h, uint32_t screen_w, uint32_t screen_h, void (*on_close)(t_widget*, void*), const char *name);
/** @} */


typedef void (*widget_draw_func)(t_widget*, hw_video_t*, void*);
extern widget_draw_func default_draw_funcs[];

#endif /* LCOM_PROJECT_WIDGET_H */
