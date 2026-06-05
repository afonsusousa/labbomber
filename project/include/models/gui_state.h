#ifndef LCOM_PROJECT_GUI_STATE_H
#define LCOM_PROJECT_GUI_STATE_H

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
        t_widget *clicked_widget;
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

#endif /* LCOM_PROJECT_GUI_STATE_H */
