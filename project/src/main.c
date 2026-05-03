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
#include "gui.h"


int main(int argc, char *argv[]) {
  lcf_set_language("EN-US");
  lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");
  lcf_log_output("/home/lcom/labs/lab4/output.txt");

  if (lcf_start(argc, argv))
    return 1;

  lcf_cleanup();

  return 0;
}

int(proj_main_loop)(int argc, char* argv[]) {
    hardware_t hw_state;
    init_hardware_state(&hw_state);

    t_gui gui;
    gui_init(&gui, hw_state.video.screen_width, hw_state.video.screen_height);

    if (timer_set_frequency(0, 144 /*fps*/) != 0) return 1;
    if (hw_timer_subscribe_int(&hw_state.timer) != 0) return 1;
    if (hw_keyboard_subscribe_int(&hw_state.keyboard) != 0) return 1;
    if (hw_mouse_subscribe_int(&hw_state.mouse) != 0) return 1;
    if (mouse_write_cmd(MOUSE_ENABLE_DATA) != 0) return 1;

    int ipc_status;
    message msg;
    while (hw_state.is_running) {
        
        if (driver_receive(ANY, &msg, &ipc_status) != 0) {
            printf("driver_receive failed\n");
            continue;
        }

        if (is_ipc_notify(ipc_status) && _ENDPOINT_P(msg.m_source) == HARDWARE) {
            
            //timer
            if (msg.m_notify.interrupts & hw_state.timer.mask) {
                hw_timer_int_handler(&hw_state.timer);

                hw_vbe_clear_screen(&hw_state.video, 0x0);
                if (gui.current_menu != NULL)
                    widget_draw(gui.current_menu, &hw_state.video);

                draw_mouse(&hw_state.mouse, &hw_state.video);
                hw_vbe_flip_buffer(&hw_state.video);
            }
            
            //keyboard
            if (msg.m_notify.interrupts & hw_state.keyboard.mask) {
                hw_keyboard_ih(&hw_state.keyboard);
                if (hw_state.keyboard.keys_pressed[0x01]) {
                    // ESC pressed
                    if (gui.current_menu == gui.start_menu) {
                        hw_state.is_running = false;
                    } else if (gui.current_menu == gui.single_name_menu || gui.current_menu == gui.multi_name_menu) {
                        gui.current_menu = gui.start_menu;
                        hw_state.keyboard.keys_pressed[0x01] = false;
                    }
                }
            }
            
            //mouse
            if (msg.m_notify.interrupts & hw_state.mouse.mask) {
                if (hw_mouse_ih(&hw_state.mouse)) {
                    uint32_t mx = hw_state.mouse.x;
                    uint32_t my = hw_state.mouse.y;
                    
                    if (gui.hovered_widget != NULL) {
                        gui.hovered_widget->hovered = false;
                    }
                    
                    gui.hovered_widget = widget_get_at(gui.current_menu, mx, my);
                    if (gui.hovered_widget != NULL) {
                        gui.hovered_widget->hovered = true;
                        if (gui.hovered_widget->on_hover != NULL) {
                            gui.hovered_widget->on_hover(gui.hovered_widget);
                        }
                    }
                    
                    if (hw_state.mouse.left_click) {
                        if (gui.hovered_widget != NULL) {
                            gui.hovered_widget->is_clicked = true;
                            if (gui.hovered_widget->on_click != NULL) {
                                gui.hovered_widget->on_click(gui.hovered_widget);
                            }
                        }
                    } else {
                        if (gui.hovered_widget != NULL) {
                            gui.hovered_widget->is_clicked = false;
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
