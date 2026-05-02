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

void on_btn_new_game_click(t_widget *self) {
    printf("New Game Button Clicked!\n");
}

void on_btn_start_click(t_widget *self) {
    printf("Start Button Clicked!\n");
}

void on_text_input_click(t_widget *self) {
    printf("Text Input Clicked!\n");
}

t_widget* widget_create_main_menu(uint32_t screen_width, uint32_t screen_height) {
    t_widget *root = widget_create(CANVAS, 0, 0, screen_width, screen_height);
    if (!root) return NULL;
    
    // New game button - centered at top
    t_widget *btn_new_game = widget_create(BUTTON, (screen_width - 200) / 2, 40, 200, 50);
    btn_new_game->data.button.label = "New Game";
    btn_new_game->on_click = on_btn_new_game_click;
    
    // Dialog for player prompt - centered
    t_widget *dlg_prompt = widget_create(DIALOG, (screen_width - 400) / 2, 150, 400, 300);
    dlg_prompt->data.dialog.title = "New Game Prompts For Players";
    
    // Player 1 input - inside dialog
    t_widget *input_p1 = widget_create(TEXT_INPUT, 50, 50, 300, 40);
    input_p1->data.text_input.buffer = (char*)malloc(256);
    memset(input_p1->data.text_input.buffer, 0, 256);
    input_p1->data.text_input.max_length = 255;
    input_p1->on_click = on_text_input_click;
    
    // Player 2 input - inside dialog, below Player 1
    t_widget *input_p2 = widget_create(TEXT_INPUT, 50, 110, 300, 40);
    input_p2->data.text_input.buffer = (char*)malloc(256);
    memset(input_p2->data.text_input.buffer, 0, 256);
    input_p2->data.text_input.max_length = 255;
    input_p2->on_click = on_text_input_click;
    
    // Start box - inside dialog at bottom
    t_widget *btn_start = widget_create(BUTTON, 125, 230, 150, 40);
    btn_start->data.button.label = "Start Box";
    btn_start->on_click = on_btn_start_click;
    
    // Add inputs and start box to dialog
    widget_add_child(dlg_prompt, input_p1);
    widget_add_child(dlg_prompt, input_p2);
    widget_add_child(dlg_prompt, btn_start);
    
    // Add new game button and dialog to root
    widget_add_child(root, btn_new_game);
    widget_add_child(root, dlg_prompt);
    
    return root;
}

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab4/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(proj_main_loop)(int argc, char* argv[]) {
    hardware_t hw_state;
    init_hardware_state(&hw_state);

    if (timer_set_frequency(0, 144 /*fps*/) != 0) return 1;
    if (hw_timer_subscribe_int(&hw_state.timer) != 0) return 1;
    if (hw_keyboard_subscribe_int(&hw_state.keyboard) != 0) return 1;
    if (hw_mouse_subscribe_int(&hw_state.mouse) != 0) return 1;
    if (mouse_write_cmd(MOUSE_ENABLE_DATA) != 0) return 1;
    
    t_widget *main_menu = widget_create_main_menu(hw_state.video.screen_width, hw_state.video.screen_height);
    t_widget *hovered_widget = NULL;

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
                hw_timer_int_handler(&hw_state.timer); // Updates the internal clock
                //update state
                //draw things
            }
            
            //keyboard
            if (msg.m_notify.interrupts & hw_state.keyboard.mask) {
                hw_keyboard_ih(&hw_state.keyboard);
                if (hw_state.keyboard.keys_pressed[0x01]) {
                    hw_state.is_running = false;
                    printf("at least we know esc was pressed\n");
                }
            }
            
            //mouse
            if (msg.m_notify.interrupts & hw_state.mouse.mask) {
                if (hw_mouse_ih(&hw_state.mouse)) {
                    // Update mouse position
                    uint32_t mx = hw_state.mouse.x;
                    uint32_t my = hw_state.mouse.y;
                    
                    // Reset previous hover
                    if (hovered_widget != NULL) {
                        hovered_widget->hovered = false;
                    }
                    
                    // Get new hovered widget
                    hovered_widget = widget_get_at(main_menu, mx, my);
                    if (hovered_widget != NULL) {
                        hovered_widget->hovered = true;
                        if (hovered_widget->on_hover != NULL) {
                            hovered_widget->on_hover(hovered_widget);
                        }
                    }
                    
                    // Handle clicks
                    if (hw_state.mouse.left_click) {
                        if (hovered_widget != NULL) {
                            hovered_widget->is_clicked = true;
                            if (hovered_widget->on_click != NULL) {
                                hovered_widget->on_click(hovered_widget);
                            }
                        }
                    } else {
                        // could do this properly via mouse release event
                        if (hovered_widget != NULL) {
                            hovered_widget->is_clicked = false;
                        }
                    }
                }
            }
            hw_vbe_clear_screen(&hw_state.video, 0x0);
            
            if (main_menu != NULL) {
                widget_draw(main_menu, &hw_state.video);
            }

            draw_mouse(&hw_state.mouse, &hw_state.video);
            hw_vbe_flip_buffer(&hw_state.video);
        }
    }

    if (main_menu != NULL) {
        widget_destroy(main_menu);
    }

    hw_timer_unsubscribe_int(&hw_state.timer);
    hw_keyboard_unsubscribe_int(&hw_state.keyboard);
    mouse_write_cmd(MOUSE_DISABLE_DATA);
    hw_mouse_unsubscribe_int(&hw_state.mouse);
    
    vg_exit();

    return 0;
}
