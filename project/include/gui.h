#ifndef LCOM_PROJECT_GUI_H
#define LCOM_PROJECT_GUI_H

#include <stdint.h>
#include <stdbool.h>
#include "widget.h"

#define MAX_VIEWS 10

typedef struct s_state {
    
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
        int32_t  dragt_dx;
        int32_t  dragt_dy;
    } drag;

    struct {
        t_widget *view_stack[MAX_VIEWS];
        bool      is_overlay[MAX_VIEWS];
        int32_t   view_count;
    } views;


} t_state;

void      gui_init(t_state *gui, uint32_t screen_width, uint32_t screen_height);
void      gui_destroy(t_state *gui);
void      gui_set_focus(t_state *gui, t_widget *widget);
void      gui_set_active(t_state *gui, t_widget *widget, bool active);
void      widget_layout(t_widget *container, uint32_t spacing, uint32_t padding, bool is_vertical);

// Stack API
void      gui_push_view(t_state *gui, t_widget *view);
void      gui_push_overlay(t_state *gui, t_widget *overlay);
void      gui_pop_view(t_state *gui);
t_widget* gui_get_top_view(t_state *gui);
t_widget* gui_pop_until_widget_found(t_state *gui, const char *widget_name);

// Menus/Launchers
void init_game(t_state *gui);
void gui_show_start_menu(t_state *gui);
void gui_show_name_menu(t_state *gui, bool is_multiplayer);
void gui_show_pause_menu(t_state *gui);
void gui_show_scoreboard(t_state *gui);
void gui_show_info_dialog(t_state *gui, const char *title, const char *message);
void gui_show_confirm_dialog(
    t_state *gui,
    const char *title,
    const char *message,
    void (*on_yes)(t_widget *, void *),
    void (*on_no)(t_widget *, void *)
);


#endif
