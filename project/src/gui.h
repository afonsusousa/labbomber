#ifndef LCOM_PROJECT_GUI_H
#define LCOM_PROJECT_GUI_H

#include <stdint.h>
#include <stdbool.h>
#include "widget.h"

// State for the main game view
// everything about the game will live here
typedef struct {
    uint16_t *pixel_buffer;
    uint32_t width;
    uint32_t height;
} t_game_state;

typedef struct s_gui {
    // Input state
    t_widget *focused;
    t_widget *hovered;
    t_widget *clicked_widget; // Widget that was pressed down on
    int32_t  mouse_x;
    int32_t  mouse_y;

    // Drag state
    t_widget *dragged_widget;
    int32_t  drag_offset_x;
    int32_t  drag_offset_y;

    // Views
    t_widget *start_menu;
    t_widget *single_name_menu;
    t_widget *multi_name_menu;
    t_widget *game_view;

    t_widget *current_view;
} t_gui;

void      gui_init(t_gui *gui, uint32_t screen_width, uint32_t screen_height);
void      gui_destroy(t_gui *gui);
void      gui_set_focus(t_gui *gui, t_widget *widget);
void      gui_set_view(t_gui *gui, t_widget *view);

t_widget* widget_create_start_menu(t_gui *gui, uint32_t screen_width, uint32_t screen_height);
t_widget* widget_create_name_menu(t_gui *gui, uint32_t screen_width, uint32_t screen_height, bool is_multiplayer);
t_widget* widget_create_game_view(t_gui *gui, uint32_t screen_width, uint32_t screen_height);

#endif /* LCOM_PROJECT_GUI_H */
