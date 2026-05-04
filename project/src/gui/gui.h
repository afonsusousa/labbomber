#ifndef LCOM_PROJECT_GUI_H
#define LCOM_PROJECT_GUI_H

#include <stdint.h>
#include <stdbool.h>
#include "../widgets/widget.h"

// State for the main game view
// everything about the game will live here
typedef struct {
    uint16_t *pixel_buffer;
    uint32_t width;
    uint32_t height;
} t_game_state;

#define MAX_VIEWS 10

typedef struct s_gui {
    
    uint32_t width;
    uint32_t height;
    bool    is_running;
    struct {
        t_widget *focused;
        t_widget *previous_focus; // Added to track previous focus
        t_widget *hovered;
        t_widget *clicked_widget; // Widget that was pressed down on
        int32_t  mouse_x;
        int32_t  mouse_y;
        bool     ctrl_down;
        bool     shift_down;
    } input;

    struct {
        t_widget *dragged_widget;
        int32_t  drag_offset_x;
        int32_t  drag_offset_y;
    } drag;

    struct {
        t_widget *view_stack[MAX_VIEWS];
        bool      is_overlay[MAX_VIEWS];
        int32_t   view_count;
    } views;

} t_gui;

void      gui_init(t_gui *gui, uint32_t screen_width, uint32_t screen_height);
void      gui_destroy(t_gui *gui);
void      gui_set_focus(t_gui *gui, t_widget *widget);
void      gui_set_active(t_gui *gui, t_widget *widget, bool active);
void      widget_layout(t_widget *container, uint32_t spacing, uint32_t padding, bool is_vertical);

// Stack API
void      gui_push_view(t_gui *gui, t_widget *view);
void      gui_push_overlay(t_gui *gui, t_widget *overlay);
void      gui_pop_view(t_gui *gui);
t_widget* gui_get_top_view(t_gui *gui);

#endif /* LCOM_PROJECT_GUI_H */
