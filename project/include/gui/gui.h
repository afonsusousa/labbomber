/**
 * @file gui.h
 * @brief High-level GUI management functions and view stack API.
 */

#ifndef LCOM_PROJECT_GUI_H
#define LCOM_PROJECT_GUI_H

#include <stdint.h>
#include <stdbool.h>
#include "models/gui_state.h"
#include "gui/widget.h"
#include "rtc.h"

struct s_ctx;

/**
 * @brief Initializes the GUI system.
 * @param ctx Pointer to the application context.
 * @param screen_width Width of the screen in pixels.
 * @param screen_height Height of the screen in pixels.
 */
void gui_init(struct s_ctx *ctx, uint32_t screen_width, uint32_t screen_height);

/**
 * @brief Destroys the GUI system and frees associated resources.
 * @param gui Pointer to the GUI manager.
 */
void gui_destroy(t_gui *gui);

/**
 * @brief Sets the input focus to a specific widget.
 * @param gui Pointer to the GUI manager.
 * @param widget Pointer to the widget to focus.
 */
void gui_set_focus(t_gui *gui, t_widget *widget);

/**
 * @brief Sets a widget as active or inactive.
 * @param gui Pointer to the GUI manager.
 * @param widget Pointer to the target widget.
 * @param active True to activate, false to deactivate.
 */
void gui_set_active(t_gui *gui, t_widget *widget, bool active);

/**
 * @brief Starts a drag operation for a widget.
 * @param gui Pointer to the GUI manager.
 * @param widget Pointer to the widget to drag.
 * @param mouse_x Current X coordinate of the mouse.
 * @param mouse_y Current Y coordinate of the mouse.
 */
void gui_begin_drag(t_gui *gui, t_widget *widget, int32_t mouse_x, int32_t mouse_y);

/**
 * @brief Ends the current drag operation.
 * @param gui Pointer to the GUI manager.
 */
void gui_end_drag(t_gui *gui);

/**
 * @brief Automatically layouts children of a container widget.
 * @param container Pointer to the container widget.
 * @param spacing Space between widgets.
 * @param padding Padding inside the container.
 * @param is_vertical True for vertical layout, false for horizontal.
 */
void widget_layout(t_widget *container, uint32_t spacing, uint32_t padding, bool is_vertical);

/**
 * @brief Handles focus navigation using the TAB key.
 * @param gui Pointer to the GUI manager.
 * @param shift_down True if Shift is held (backward navigation).
 */
void gui_handle_tab_navigation(t_gui *gui, bool shift_down);

/**
 * @brief Retrieves the current system time via RTC.
 * @param gui Pointer to the GUI manager (for RTC reference).
 * @param out Pointer to store the retrieved time.
 * @return 0 on success, non-zero otherwise.
 */
int gui_get_curr_time(t_gui *gui, hw_rtc_t *out);

/** @name Stack API
 *  Functions for managing the view and overlay stack.
 *  @{
 */

/**
 * @brief Pushes a new view onto the stack.
 * @param gui Pointer to the GUI manager.
 * @param view Pointer to the view widget to push.
 */
void gui_push_view(t_gui *gui, t_widget *view);

/**
 * @brief Pushes a new overlay (e.g., modal) onto the stack.
 * @param gui Pointer to the GUI manager.
 * @param overlay Pointer to the overlay widget to push.
 */
void gui_push_overlay(t_gui *gui, t_widget *overlay);

/**
 * @brief Pops the top-most view/overlay from the stack.
 * @param gui Pointer to the GUI manager.
 */
void gui_pop_view(t_gui *gui);

/**
 * @brief Gets the top-most widget on the stack.
 * @param gui Pointer to the GUI manager.
 * @return Pointer to the top widget, or NULL if empty.
 */
t_widget* gui_get_top_view(t_gui *gui);

/**
 * @brief Pops widgets until a specific named widget is found.
 * @param gui Pointer to the GUI manager.
 * @param widget_name Name of the widget to find.
 * @return Pointer to the found widget, or NULL.
 */
t_widget* gui_pop_until_widget_found(t_gui *gui, const char *widget_name);

/** @} */

/**
 * @brief Finds a widget by its name in the current stack.
 * @param gui Pointer to the GUI manager.
 * @param name Name of the widget.
 * @return Pointer to the widget, or NULL.
 */
t_widget* widget_find_by_name(t_gui *gui, const char *name);

/**
 * @brief Creates and pushes a standard dialog onto the stack.
 * @param ctx Pointer to the application context.
 * @param title Title of the dialog.
 * @param w Width of the dialog.
 * @param h Height of the dialog.
 * @param on_close Callback function when the dialog is closed.
 * @param name Unique name for the dialog widget.
 * @return Pointer to the created dialog widget.
 */
t_widget* gui_create_dialog(struct s_ctx *ctx, const char *title, uint32_t w, uint32_t h, void (*on_close)(t_widget*, void*), const char *name);

/** @name Menus/Launchers
 *  Functions to show specific pre-defined menus.
 *  @{
 */

/** @brief Initializes the game logic. */
void init_game(struct s_ctx *ctx);

/** @brief Shows the main start menu. */
void gui_show_start_menu(struct s_ctx *ctx);

/** @brief Shows the name entry menu. */
void gui_show_name_menu(struct s_ctx *ctx, bool is_multiplayer);

/** @brief Shows the session selection menu. */
void gui_show_session_menu(struct s_ctx *ctx, const char *title, const char *message);

/** @brief Shows an information dialog. */
void gui_show_info_dialog(struct s_ctx *ctx, const char *title, const char *message);

/** @brief Shows the high-score scoreboard. */
void gui_show_scoreboard(struct s_ctx *ctx);

/** @brief Shows the main game view. */
void gui_show_game_view(struct s_ctx *ctx);

/** @brief Resets the game view state. */
void gui_reset_game_view(struct s_ctx *ctx);

/**
 * @brief Shows a confirmation dialog with Yes/No options.
 * @param ctx Application context.
 * @param title Dialog title.
 * @param message Message to display.
 * @param on_yes Callback for "Yes".
 * @param on_no Callback for "No".
 */
void gui_show_confirm_dialog(
    struct s_ctx *ctx,
    const char *title,
    const char *message,
    void (*on_yes)(t_widget *, void *),
    void (*on_no)(t_widget *, void *)
);

/** @} */

/**
 * @brief Checks if a string is NULL or contains only whitespace.
 * @param s String to check.
 * @return True if blank, false otherwise.
 */
bool is_blank_string(const char *s);

#endif
