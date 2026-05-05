#ifndef LCOM_PROJECT_STATE_H
#define LCOM_PROJECT_STATE_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_VIEWS 10

struct s_widget;
typedef struct s_state {
    
    uint32_t width;
    uint32_t height;

    bool    is_running;
    struct {
        struct s_widget *focused;
        struct s_widget *previous_focus; // Added to track previous focus
        struct s_widget *hovered;
        struct s_widget *clicked_widget; // Widget that was pressed down on
        int32_t  mouse_x;
        int32_t  mouse_y;
        bool     ctrl_down;
        bool     shift_down;
    } input;

    struct {
        struct s_widget *dragged_widget;
        int32_t  dragt_dx;
        int32_t  dragt_dy;
    } drag;

    struct {
        struct s_widget *view_stack[MAX_VIEWS];
        bool      is_overlay[MAX_VIEWS];
        int32_t   view_count;
    } views;


} t_state;

void      gui_init(t_state *gui, uint32_t screen_width, uint32_t screen_height);
void      gui_destroy(t_state *gui);
void      gui_set_focus(t_state *gui, struct s_widget *widget);
void      gui_set_active(t_state *gui, struct s_widget *widget, bool active);
void      widget_layout(struct s_widget *container, uint32_t spacing, uint32_t padding, bool is_vertical);

// Stack API
void      gui_push_view(t_state *gui, struct s_widget *view);
void      gui_push_overlay(t_state *gui, struct s_widget *overlay);
void      gui_pop_view(t_state *gui);
struct s_widget* gui_get_top_view(t_state *gui);
struct s_widget* gui_pop_until_widget_found(t_state *gui, const char *widget_name);

#endif
