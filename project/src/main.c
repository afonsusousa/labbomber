#include <stdio.h>
#include <lcom/lcf.h>

#include "../lib/timer/timer.h"
#include "../lib/keyboard/keyboard.h"
#include "../lib/mouse/mouse.h"
#include "../lib/keyboard/i8042.h"
#include "../lib/utils/utils.h"
#include "game.h"
#include "draw.h"
#include "widget.h"
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

    gui_init(hw_state.video.screen_width, hw_state.video.screen_height);

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

                if (g_gui.views.view_count > 0) {
                    // Find the highest opaque view so we don't draw
                    // the entire stack if we don't need to.
                    int start_idx = g_gui.views.view_count - 1;
                    while (start_idx > 0 && g_gui.views.is_overlay[start_idx]) {
                        start_idx--;
                    }

                    // Only draw the opaque view and any transparent overlays on top of it
                    for (int i = start_idx; i < g_gui.views.view_count; i++) {
                        widget_tick(g_gui.views.view_stack[i]);
                        widget_draw(g_gui.views.view_stack[i], &hw_state.video);
                    }
                }

                // Draw the cursor based on hover state
                if (g_gui.input.hovered != NULL && g_gui.input.hovered->type == TEXT_INPUT) {
                    draw_text_cursor(&hw_state.mouse, &hw_state.video);
                } else {
                    draw_mouse(&hw_state.mouse, &hw_state.video);
                }

                hw_vbe_flip_buffer(&hw_state.video);
            }

			//KEYBOARD
            if (msg.m_notify.interrupts & hw_state.keyboard.mask) {
                hw_keyboard_ih(&hw_state.keyboard);
                bool esc_is_pressed = hw_state.keyboard.keys_pressed[0x01];
                if (esc_is_pressed && !esc_was_pressed) { // ESC just pressed
                    t_widget *top_view = gui_get_top_view();
                    if (g_gui.input.focused != NULL && g_gui.input.focused->on_quit != NULL) {
                        g_gui.input.focused->on_quit(g_gui.input.focused, &g_gui);
                    } else if (top_view != NULL && top_view->on_quit != NULL) {
                        top_view->on_quit(top_view, &g_gui);
                    } else if (g_gui.views.view_count <= 1) { // Only the base view is left
                        hw_state.is_running = false;
                    }
                }
                esc_was_pressed = esc_is_pressed;
            }

			//MOUSE
            if (msg.m_notify.interrupts & hw_state.mouse.mask) {
                if (hw_mouse_ih(&hw_state.mouse)) {
                    g_gui.input.mouse_x = hw_state.mouse.x;
                    g_gui.input.mouse_y = hw_state.mouse.y;
                    t_widget *top_view = gui_get_top_view();

					// Dragged widgets capture the mouse
                    if (g_gui.drag.dragged_widget != NULL) {
                        if (hw_state.mouse.left_click) {
                            if (g_gui.drag.dragged_widget->on_drag) {
                                g_gui.drag.dragged_widget->on_drag(g_gui.drag.dragged_widget, &g_gui);
                            }
                        } else {
							g_gui.drag.dragged_widget = NULL;
						}
                	//if nothing is being dragged, then interpret the actual click
                    } else if (hw_state.mouse.left_click) {
                        if (g_gui.input.clicked_widget == NULL) {
                            // --- Mouse Down ---
                            g_gui.input.clicked_widget = widget_get_at(top_view, g_gui.input.mouse_x, g_gui.input.mouse_y);
                            if (g_gui.input.clicked_widget != NULL) {
                                WIDGET_SET_CLICKED(g_gui.input.clicked_widget, true);
                                if (g_gui.input.clicked_widget->on_press) {
                                    g_gui.input.clicked_widget->on_press(g_gui.input.clicked_widget, &g_gui);
                                }
                            }
                        } else if (g_gui.input.clicked_widget->on_drag) {
                                g_gui.input.clicked_widget->on_drag(g_gui.input.clicked_widget, &g_gui);
                        }
					// no left click means left click release ;)
                    } else {
                        if (g_gui.input.clicked_widget != NULL) {
                            t_widget* clicked = g_gui.input.clicked_widget;
                            t_widget* current_hover = widget_get_at(top_view, g_gui.input.mouse_x, g_gui.input.mouse_y);

                            if (clicked == current_hover) {
                                if (WIDGET_CAN_RECEIVE_FOCUS(clicked))
                                    gui_set_focus(clicked);
                                if (clicked->on_click)
                                    clicked->on_click(clicked, &g_gui);
                            }

                            // Always un-click the visual state of the widget itself
                            WIDGET_SET_CLICKED(clicked, false);

                            // If the global state still tracks this widget as clicked, clear it
                            if (g_gui.input.clicked_widget == clicked) {
                                g_gui.input.clicked_widget = NULL;
                            }
                        }
                    }

					//hover can only happen if nothing is being dragged
                    if (g_gui.drag.dragged_widget == NULL) {
                        t_widget* new_hover = widget_get_at(top_view, g_gui.input.mouse_x, g_gui.input.mouse_y);
                        if (g_gui.input.hovered != new_hover) {
                            if (g_gui.input.hovered != NULL) WIDGET_SET_HOVERED(g_gui.input.hovered, false);
                            g_gui.input.hovered = new_hover;
                            if (g_gui.input.hovered != NULL) WIDGET_SET_HOVERED(g_gui.input.hovered, true);
                        }
                    }
                }
            }
        }
    }

    gui_destroy();

    hw_timer_unsubscribe_int(&hw_state.timer);
    hw_keyboard_unsubscribe_int(&hw_state.keyboard);
    mouse_write_cmd(MOUSE_DISABLE_DATA);
    hw_mouse_unsubscribe_int(&hw_state.mouse);
    
    vg_exit();

    return 0;
}
