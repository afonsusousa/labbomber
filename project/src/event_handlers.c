#include "hardware.h"
#include "gui.h"
#include "application.h"
#include "widget.h"
#include "draw.h"
#include "../lib/rtc/rtc.h"

void draw_debug_overlay(hw_video_t *video, const t_gui *gui);

void handle_timer(hardware_t *hw_state, t_ctx *ctx) {
    t_gui *gui = &ctx->gui;
    hw_timer_int_handler(&hw_state->timer);
    
    if (hw_state->timer.ticks % 60 == 0) {
        hw_rtc_t hw_time;
        if (hw_rtc_get_time(&hw_time) == 0) {
            ctx->real_time.year    = hw_time.year;
            ctx->real_time.month   = hw_time.month;
            ctx->real_time.day     = hw_time.day;
            ctx->real_time.hours   = hw_time.hours;
            ctx->real_time.minutes = hw_time.minutes;
            ctx->real_time.seconds = hw_time.seconds;
        }
    }
    
    if (!ctx->game.is_paused) {
        ctx->game.logical_ticks++;
        game_state_update(ctx);
    }
    
    hw_vbe_clear_screen(&hw_state->video, 0x0);

    if (gui->views.view_count > 0) {
        int start_idx = gui->views.view_count - 1;
        while (start_idx > 0 && gui->views.is_overlay[start_idx]) {
            start_idx--;
        }
        for (int i = start_idx; i < gui->views.view_count; i++) {
            widget_tick(gui->views.view_stack[i], ctx);
            widget_draw(gui->views.view_stack[i], &hw_state->video, ctx);
        }
    }

    if (gui->input.hovered != NULL && gui->input.hovered->type == TEXT_INPUT) {
        draw_text_cursor(&hw_state->mouse, &hw_state->video);
    } else {
        draw_mouse(&hw_state->mouse, &hw_state->video);
    }

    draw_debug_overlay(&hw_state->video, gui);
    hw_vbe_flip_buffer(&hw_state->video);
}

//will need to check if this is enough for CTRL SHIFT and other 2byte keys
void handle_keyboard(hardware_t *hw_state, t_ctx *ctx, bool *esc_was_pressed) {
    t_gui *gui = &ctx->gui;
    hw_keyboard_ih(&hw_state->keyboard);

    // Update modifier key states
    gui->input.shift_down = hw_state->keyboard.keys_pressed[KEY_SHIFT_LEFT] || hw_state->keyboard.keys_pressed[KEY_SHIFT_RIGHT];
    gui->input.ctrl_down  = hw_state->keyboard.keys_pressed[KEY_CTRL];

    uint8_t sc = hw_state->keyboard.scancode;

    // Ignore extended key prefix
    if (sc == 0xE0) {
        return; 
    }

    bool is_make = IS_MAKE_CODE(sc);

    // Process make codes (key presses) with special logic
    if (is_make) {
        // Handle tab navigation in UI
        if (sc == KEY_TAB) {
            gui_handle_tab_navigation(gui, gui->input.shift_down);
            return;
        }

        // Handle ESC key for quit
        bool esc_is_pressed = hw_state->keyboard.keys_pressed[KEY_ESC];
        if (esc_is_pressed && !(*esc_was_pressed)) {
            t_widget *top_view = gui_get_top_view(gui);
            if (gui->input.focused != NULL && gui->input.focused->on_quit != NULL) {
                gui->input.focused->on_quit(gui->input.focused, ctx);
            } else if (top_view != NULL && top_view->on_quit != NULL) {
                top_view->on_quit(top_view, ctx);
            }
        }
        *esc_was_pressed = esc_is_pressed;
    }

    // Pass all key events (both make and break) to the widget
    if (gui->input.focused != NULL && gui->input.focused->on_key_press != NULL) {
        gui->input.focused->on_key_press(gui->input.focused, sc, ctx);
    }
}

void handle_mouse(hardware_t *hw_state, t_ctx *ctx) {
    t_gui *gui = &ctx->gui;
    if (!hw_mouse_ih(&hw_state->mouse)) return;

    gui->input.mouse_x = hw_state->mouse.x;
    gui->input.mouse_y = hw_state->mouse.y;

    bool is_pressed = hw_state->mouse.left_click;
    t_widget *top_view = gui_get_top_view(gui);
    t_widget *target = widget_get_at(top_view, gui->input.mouse_x, gui->input.mouse_y);

    t_widget **dragged = &gui->drag.dragged_widget;
    t_widget **clicked = &gui->input.clicked_widget;
    t_widget **hovered = &gui->input.hovered;

    if (*dragged) { // something is being dragged
        
        if (is_pressed) {
            if ((*dragged)->on_drag)
                (*dragged)->on_drag(*dragged, ctx);
            else if ((*dragged)->on_press)
                (*dragged)->on_press(*dragged, ctx);
        } else {
            *dragged = NULL;
        }
    } else if (is_pressed) {
        if (!*clicked) {
            *clicked = target;
            if (*clicked) {
                WIDGET_SET_CLICKED(*clicked, true);
                if (WIDGET_CAN_RECEIVE_FOCUS(*clicked)) {
                    if (gui->input.focused != NULL) 
                        gui->input.focused->focus_cue = 0; 
                    gui_set_focus(gui, *clicked);
                }
                if ((*clicked)->on_press) (*clicked)->on_press(*clicked, ctx);
            }
        } else {
            if (target == *clicked) {
                WIDGET_SET_CLICKED(*clicked, true);
            } else {
                WIDGET_SET_CLICKED(*clicked, false);
            }
            if ((*clicked)->on_drag) (*clicked)->on_drag(*clicked, ctx);
            else if ((*clicked)->on_press) (*clicked)->on_press(*clicked, ctx);
        }
    } else {
        if (*clicked) {
            t_widget *was_clicked = *clicked;
            *clicked = NULL;
            WIDGET_SET_CLICKED(was_clicked, false);
            
            if (was_clicked == target) {
                if (was_clicked->on_click) {
                    was_clicked->on_click(was_clicked, ctx);
                }
            }
        }
    } 

    // only update hover if we aren't dragging anything 
    if (!*dragged) {
        if (*hovered != target) {
            if (*hovered) WIDGET_SET_HOVERED(*hovered, false);
            *hovered = target;
            if (*hovered) WIDGET_SET_HOVERED(*hovered, true);
        }
    }
}
