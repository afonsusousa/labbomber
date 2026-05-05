#include <stdio.h>
#include <lcom/lcf.h>

#include "../lib/timer/timer.h"
#include "../lib/keyboard/keyboard.h"
#include "../lib/mouse/mouse.h"
#include "../lib/keyboard/i8042.h"
#include "../lib/utils/utils.h"
#include "hardware.h"
#include "game.h"
#include "draw.h"
#include "widget.h"
#include "gui.h"

void init_hardware_state(hardware_t *hw_state) {
    if (!hw_state) return;

    memset(hw_state, 0, sizeof(hardware_t));

    hw_timer_init(&hw_state->timer);
    hw_rtc_init(&hw_state->time_info);
    hw_keyboard_init(&hw_state->keyboard);
    hw_mouse_init(&hw_state->mouse);
    hw_vbe_init(&hw_state->video, 0x110);
    
    hw_state->is_running = true;
    
    hw_state->mouse.max_x = hw_state->video.screen_width;
    hw_state->mouse.max_y = hw_state->video.screen_height;
    hw_state->mouse.x = (hw_state->video.screen_width / 3) * 2;
    hw_state->mouse.y = (hw_state->video.screen_height / 3) * 2;
}

void draw_debug_overlay(hw_video_t *video, const t_gui *gui);

static void handle_timer(hardware_t *hw_state, t_gui *gui) {
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
static void handle_keyboard(hardware_t *hw_state, t_gui *gui, bool *esc_was_pressed) {
    hw_keyboard_ih(&hw_state->keyboard);
    
    gui->input.shift_down = hw_state->keyboard.keys_pressed[0x2A] || hw_state->keyboard.keys_pressed[0x36];
    gui->input.ctrl_down  = hw_state->keyboard.keys_pressed[0x1D];

    uint8_t sc = hw_state->keyboard.scancode;

    if (sc == 0xE0) {
        return; 
    }
    bool is_make = (sc & 0x80) == 0;
    // Mask out the break bit (0x80) so we can check if the key is valid 
    // even when it's being released.
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

static void handle_mouse(hardware_t *hw_state, t_gui *gui) {
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

int main(int argc, char *argv[]) {
  lcf_set_language("EN-US");
  lcf_trace_calls("/home/lcom/labs/project/trace.txt");
  lcf_log_output("/home/lcom/labs/project/output.txt");

  if (lcf_start(argc, argv))
    return 1;

  lcf_cleanup();

  return 0;
}

int(proj_main_loop)(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    hardware_t hw_state;
    init_hardware_state(&hw_state);

    t_gui gui;
    gui_init(&gui, hw_state.video.screen_width, hw_state.video.screen_height);

    if (timer_set_frequency(0, 144) != 0) return 1;
    if (hw_timer_subscribe_int(&hw_state.timer) != 0) return 1;
    if (hw_keyboard_subscribe_int(&hw_state.keyboard) != 0) return 1;
    if (hw_mouse_subscribe_int(&hw_state.mouse) != 0) return 1;
    if (mouse_write_cmd(MOUSE_ENABLE_DATA) != 0) return 1;
    init_sprite_cache();

    int ipc_status;
    message msg;
    bool esc_was_pressed = false;

    while (gui.is_running) {
        if (driver_receive(ANY, &msg, &ipc_status) != 0) {
            printf("driver_receive failed\n");
            continue;
        }

        if (is_ipc_notify(ipc_status) && _ENDPOINT_P(msg.m_source) == HARDWARE) {
            if (msg.m_notify.interrupts & hw_state.timer.mask)
                handle_timer(&hw_state, &gui);
            if (msg.m_notify.interrupts & hw_state.keyboard.mask) 
                handle_keyboard(&hw_state, &gui, &esc_was_pressed);
            if (msg.m_notify.interrupts & hw_state.mouse.mask)
                handle_mouse(&hw_state, &gui);
        }
    }

    gui_destroy(&gui);

    hw_timer_unsubscribe_int(&hw_state.timer);
    hw_keyboard_unsubscribe_int(&hw_state.keyboard);
    mouse_write_cmd(MOUSE_DISABLE_DATA);
    hw_mouse_unsubscribe_int(&hw_state.mouse);
    
    vg_exit();

    return 0;
}
