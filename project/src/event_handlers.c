#include "hardware.h"
#include "gui.h"
#include "widget.h"
#include "draw.h"

void draw_debug_overlay(hw_video_t *video, const t_gui *gui);

void handle_timer(hardware_t *hw_state, t_gui *gui) {
    hw_timer_int_handler(&hw_state->timer);
    hw_vbe_clear_screen(&hw_state->video, 0x0);

    if (gui->views.view_count > 0) {
        int start_idx = gui->views.view_count - 1;
        while (start_idx > 0 && gui->views.is_overlay[start_idx]) {
            start_idx--;
        }
        for (int i = start_idx; i < gui->views.view_count; i++) {
            widget_tick(gui->views.view_stack[i], gui);
            widget_draw(gui->views.view_stack[i], &hw_state->video);
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
void handle_keyboard(hardware_t *hw_state, t_gui *gui, bool *esc_was_pressed) {
    hw_keyboard_ih(&hw_state->keyboard);

    gui->input.shift_down = hw_state->keyboard.keys_pressed[0x2A] || hw_state->keyboard.keys_pressed[0x36];
    gui->input.ctrl_down  = hw_state->keyboard.keys_pressed[0x1D];

    uint8_t sc = hw_state->keyboard.scancode;

    if (sc == 0xE0) {
        return; 
    }
    bool is_make = (sc & 0x80) == 0;
    bool is_valid_press = hw_state->keyboard.is_two_bytes ||
                          hw_state->keyboard.keys_pressed[sc & 0x7F] ||
                          (!is_make);
    if (is_valid_press) {
        // TAB / MAKE ONLY
        if (is_make && sc == 0x0F) {
            if (gui->input.focused != NULL) {
                if (gui->input.focused->focus_cue == 0) {
                    gui->input.focused->focus_cue = 1;
                    return; // Consume the key immediately
                }

                t_widget *next_focus = NULL;
                if (gui->input.shift_down) {
                    next_focus = widget_get_prev_focusable_sibling(gui->input.focused);
                } else {
                    next_focus = widget_get_next_focusable_sibling(gui->input.focused);
                }

                if (next_focus != NULL) {
                    gui->input.focused->focus_cue = (gui->input.focused->focus_cue == 2) ? 2 : 0;
                    gui_set_focus(gui, next_focus);
                    next_focus->focus_cue = 1; 
                }
            }
            else {
                t_widget *first = widget_first_focusable(gui_get_top_view(gui));
                if (first != NULL) {
                    gui_set_focus(gui, first);
                    first->focus_cue = 1;
                }
            }
            return;
        }
        //Make or Break
        if (gui->input.focused != NULL && gui->input.focused->on_key_press != NULL) {
            gui->input.focused->on_key_press(gui->input.focused, sc, gui);
        }
    }

    bool esc_is_pressed = hw_state->keyboard.keys_pressed[0x01];
    if (esc_is_pressed && !(*esc_was_pressed)) {
        t_widget *top_view = gui_get_top_view(gui);
        if (gui->input.focused != NULL && gui->input.focused->on_quit != NULL) {
            gui->input.focused->on_quit(gui->input.focused, gui);
        } else if (top_view != NULL && top_view->on_quit != NULL) {
            top_view->on_quit(top_view, gui);
        }
    }
    *esc_was_pressed = esc_is_pressed;
}

void handle_mouse(hardware_t *hw_state, t_gui *gui) {
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
                (*dragged)->on_drag(*dragged, gui);
            else if ((*dragged)->on_press)
                (*dragged)->on_press(*dragged, gui);
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
                if ((*clicked)->on_press) (*clicked)->on_press(*clicked, gui);
            }
        } else {
            if (target == *clicked) {
                WIDGET_SET_CLICKED(*clicked, true);
            } else {
                WIDGET_SET_CLICKED(*clicked, false);
            }
            if ((*clicked)->on_drag) (*clicked)->on_drag(*clicked, gui);
            else if ((*clicked)->on_press) (*clicked)->on_press(*clicked, gui);
        }
    } else {
        if (*clicked) {
            t_widget *was_clicked = *clicked;
            *clicked = NULL;
            WIDGET_SET_CLICKED(was_clicked, false);
            
            if (was_clicked == target) {
                if (was_clicked->on_click) {
                    was_clicked->on_click(was_clicked, gui);
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
