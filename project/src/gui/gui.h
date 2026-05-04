#ifndef LCOM_PROJECT_GUI_H
#define LCOM_PROJECT_GUI_H

#include <stdint.h>
#include <stdbool.h>
#include "../widget.h"
#include "views.h"

// State for the main game view
// everything about the game will live here
typedef struct {
    uint16_t *pixel_buffer;
    uint32_t width;
    uint32_t height;
} t_game_state;

typedef struct s_gui {
    struct {
        t_widget *focused;
        t_widget *previous_focus; // Added to track previous focus
        t_widget *hovered;
        t_widget *clicked_widget; // Widget that was pressed down on
        int32_t  mouse_x;
        int32_t  mouse_y;
    } input;

    struct {
        t_widget *dragged_widget;
        int32_t  drag_offset_x;
        int32_t  drag_offset_y;
    } drag;

    struct {
        t_widget *start_menu;
        t_widget *single_name_menu;
        t_widget *multi_name_menu;
        t_widget *game_view;
        t_widget *current_view;
    } views;

    struct {
        t_widget *pause_overlay;
        t_widget *pause_dialog;
        t_widget *confirm_overlay;
        t_widget *confirm_dialog;
        t_widget *confirm_text;
        t_widget *confirm_yes;
        t_widget *confirm_no;
    } dialogs;
} t_gui;

extern t_gui g_gui;

void      gui_init(uint32_t screen_width, uint32_t screen_height);
void      gui_destroy(void);
void      gui_set_focus(t_widget *widget);
void      gui_set_active(t_widget *widget, bool active);
void      gui_set_view(t_widget *view);
void      widget_layout(t_widget *container, uint32_t spacing, uint32_t padding, bool is_vertical);

void      on_dialog_press(t_widget *self);
void      on_dialog_drag(t_widget *self);

#endif /* LCOM_PROJECT_GUI_H */
