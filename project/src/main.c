#include <stdio.h>
#include <lcom/lcf.h>

#include "../lib/timer/timer.h"
#include "../lib/keyboard/keyboard.h"
#include "../lib/mouse/mouse.h"
#include "../lib/keyboard/i8042.h"
#include "../lib/utils/utils.h"
#include "game.h"
#include "draw.h"
#include "widgets/widget.h"
#include "gui/gui.h"

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

    t_gui gui; // g_gui is now a local variable
    gui_init(&gui, hw_state.video.screen_width, hw_state.video.screen_height);

    if (timer_set_frequency(0, 144 /*fps*/) != 0) return 1;
    if (hw_timer_subscribe_int(&hw_state.timer) != 0) return 1;
    if (hw_keyboard_subscribe_int(&hw_state.keyboard) != 0) return 1;
    if (hw_mouse_subscribe_int(&hw_state.mouse) != 0) return 1;
    if (mouse_write_cmd(MOUSE_ENABLE_DATA) != 0) return 1;

    int ipc_status;
    message msg;
    bool esc_was_pressed = false;

    while (hw_state.is_running) {
        if (driver_receive(ANY, &msg, &ipc_status) != 0) {
            printf("driver_receive failed\n");
            continue;
        }

        if (is_ipc_notify(ipc_status) && _ENDPOINT_P(msg.m_source) == HARDWARE) {
			//TIMER (TICKS)
            if (msg.m_notify.interrupts & hw_state.timer.mask) {
                hw_timer_int_handler(&hw_state.timer);
                hw_vbe_clear_screen(&hw_state.video, 0x0);

                if (gui.views.view_count > 0) {
                    int start_idx = gui.views.view_count - 1;
                    while (start_idx > 0 && gui.views.is_overlay[start_idx]) {
                        start_idx--;
                    }
                    for (int i = start_idx; i < gui.views.view_count; i++) {
                        widget_tick(gui.views.view_stack[i], &gui);
                        widget_draw(gui.views.view_stack[i], &hw_state.video);
                    }
                }

                if (gui.input.hovered != NULL && gui.input.hovered->type == TEXT_INPUT) {
                    draw_text_cursor(&hw_state.mouse, &hw_state.video);
                } else {
                    draw_mouse(&hw_state.mouse, &hw_state.video);
                }

                hw_vbe_flip_buffer(&hw_state.video);
            }

			//KEYBOARD
            if (msg.m_notify.interrupts & hw_state.keyboard.mask) {
                hw_keyboard_ih(&hw_state.keyboard);
                if (!hw_state.keyboard.is_two_bytes && (hw_state.keyboard.scancode & 0x80) == 0) {
                    if (gui.input.focused != NULL && gui.input.focused->on_key_press != NULL) {
                        gui.input.focused->on_key_press(gui.input.focused, hw_state.keyboard.scancode, &gui);
                    }
                }
                
                bool esc_is_pressed = hw_state.keyboard.keys_pressed[0x01];
                if (esc_is_pressed && !esc_was_pressed) {
                    t_widget *top_view = gui_get_top_view(&gui);
                    if (gui.input.focused != NULL && gui.input.focused->on_quit != NULL) {
                        gui.input.focused->on_quit(gui.input.focused, &gui);
                    } else if (top_view != NULL && top_view->on_quit != NULL) {
                        top_view->on_quit(top_view, &gui);
                    } else if (gui.views.view_count <= 1) {
                        hw_state.is_running = false;
                    }
                }
                esc_was_pressed = esc_is_pressed;
            }

			//MOUSE
            if (msg.m_notify.interrupts & hw_state.mouse.mask) {
                if (hw_mouse_ih(&hw_state.mouse)) {
                    gui.input.mouse_x = hw_state.mouse.x;
                    gui.input.mouse_y = hw_state.mouse.y;
                    t_widget *top_view = gui_get_top_view(&gui);

					if (gui.drag.dragged_widget != NULL) {
                        if (hw_state.mouse.left_click) {
                            if (gui.drag.dragged_widget->on_drag) {
                                gui.drag.dragged_widget->on_drag(gui.drag.dragged_widget, &gui);
                            }
                        } else {
							gui.drag.dragged_widget = NULL;
						}
                    } else if (hw_state.mouse.left_click) {
                        if (gui.input.clicked_widget == NULL) {
                            gui.input.clicked_widget = widget_get_at(top_view, gui.input.mouse_x, gui.input.mouse_y);
                            if (gui.input.clicked_widget != NULL) {
                                WIDGET_SET_CLICKED(gui.input.clicked_widget, true);
                                if (gui.input.clicked_widget->on_press) {
                                    gui.input.clicked_widget->on_press(gui.input.clicked_widget, &gui);
                                }
                            }
                        } else if (gui.input.clicked_widget->on_drag) {
                                gui.input.clicked_widget->on_drag(gui.input.clicked_widget, &gui);
                        }
                    } else {
                        if (gui.input.clicked_widget != NULL) {
                            t_widget* clicked = gui.input.clicked_widget;
                            t_widget* current_hover = widget_get_at(top_view, gui.input.mouse_x, gui.input.mouse_y);

                            if (clicked == current_hover) {
                                if (WIDGET_CAN_RECEIVE_FOCUS(clicked))
                                    gui_set_focus(&gui, clicked);
                                if (clicked->on_click)
                                    clicked->on_click(clicked, &gui);
                            }
                            WIDGET_SET_CLICKED(clicked, false);
                            if (gui.input.clicked_widget == clicked) {
                                gui.input.clicked_widget = NULL;
                            }
                        }
                    }

                    if (gui.drag.dragged_widget == NULL) {
                        t_widget* new_hover = widget_get_at(top_view, gui.input.mouse_x, gui.input.mouse_y);
                        if (gui.input.hovered != new_hover) {
                            if (gui.input.hovered != NULL) WIDGET_SET_HOVERED(gui.input.hovered, false);
                            gui.input.hovered = new_hover;
                            if (gui.input.hovered != NULL) WIDGET_SET_HOVERED(gui.input.hovered, true);
                        }
                    }
                }
            }
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
