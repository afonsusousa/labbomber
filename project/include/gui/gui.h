#ifndef LCOM_PROJECT_GUI_H
#define LCOM_PROJECT_GUI_H

#include <stdint.h>
#include <stdbool.h>
#include "gui/widget.h"
#include "rtc.h"

struct s_ctx;

#define MAX_VIEWS 10

typedef struct s_state {

    uint32_t width;
    uint32_t height;

    bool    is_running;
    struct {
        t_widget *focused;
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

    hw_rtc_t *rtc;
    hw_video_t *video;

} t_gui;

void      gui_init(struct s_ctx *ctx, uint32_t screen_width, uint32_t screen_height);
void      gui_destroy(t_gui *gui);
void      gui_set_focus(t_gui *gui, t_widget *widget);
void      gui_set_active(t_gui *gui, t_widget *widget, bool active);
void      gui_begin_drag(t_gui *gui, t_widget *widget, int32_t mouse_x, int32_t mouse_y);
void      gui_end_drag(t_gui *gui);
void      widget_layout(t_widget *container, uint32_t spacing, uint32_t padding, bool is_vertical);

void      gui_handle_tab_navigation(t_gui *gui, bool shift_down);
int       gui_get_curr_time(t_gui *gui, hw_rtc_t *out);

// Stack API
void      gui_push_view(t_gui *gui, t_widget *view);
void      gui_push_overlay(t_gui *gui, t_widget *overlay);
void      gui_pop_view(t_gui *gui);
t_widget* gui_get_top_view(t_gui *gui);
t_widget* gui_pop_until_widget_found(t_gui *gui, const char *widget_name);

t_widget* widget_find_by_name(t_gui *gui, const char *name);
t_widget* gui_create_dialog(struct s_ctx *ctx, const char *title, uint32_t w, uint32_t h, void (*on_close)(t_widget*, void*), const char *name);
// Menus/Launchers
void init_game(struct s_ctx *ctx);
void gui_show_start_menu(struct s_ctx *ctx);
void gui_show_name_menu(struct s_ctx *ctx, bool is_multiplayer);
void gui_show_session_menu(struct s_ctx *ctx, const char *title, const char *message);
void gui_show_info_dialog(struct s_ctx *ctx, const char *title, const char *message);
void gui_show_scoreboard(struct s_ctx *ctx);
void gui_show_confirm_dialog(
    struct s_ctx *ctx,
    const char *title,
    const char *message,
    void (*on_yes)(t_widget *, void *),
    void (*on_no)(t_widget *, void *)
);

bool is_blank_string(const char *s);


#endif
