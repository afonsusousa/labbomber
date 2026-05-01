#include <stdio.h>
#include <lcom/lcf.h>

#include "../lib/timer/timer.h"
#include "../lib/keyboard/keyboard.h"
#include "../lib/mouse/mouse.h"
#include "../lib/keyboard/i8042.h"
#include "../lib/utils/utils.h"
#include "game.h"

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
    int speed = 2;
    int x = hw_state.video.screen_width / 2;
    int y = hw_state.video.screen_height / 2;

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
                if (hw_state.keyboard.keys_pressed[0x11]) {
                    y -= speed; 
                }
                if (hw_state.keyboard.keys_pressed[0x1E]) {
                    x -= speed;
                }
                if (hw_state.keyboard.keys_pressed[0x1F]) {
                    y += speed;
                }
                if (hw_state.keyboard.keys_pressed[0x20]) {
                    x += speed;
                }
            }
            
            //mouse
            if (msg.m_notify.interrupts & hw_state.mouse.mask) {
                if (hw_mouse_ih(&hw_state.mouse)) {
                    // do something
                }
            }
            hw_vbe_draw_pixel(&hw_state.video, x, y, 0xFFFF00);
            hw_vbe_draw_pixel(&hw_state.video, x + 1, y, 0xFFFF00);
            hw_vbe_draw_pixel(&hw_state.video, x, y, 0xFFFF00);
            hw_vbe_draw_pixel(&hw_state.video, x, y + 1, 0xFFFF00);
            hw_vbe_flip_buffer(&hw_state.video);
        }
    }

    hw_timer_unsubscribe_int(&hw_state.timer);
    hw_keyboard_unsubscribe_int(&hw_state.keyboard);
    mouse_write_cmd(MOUSE_DISABLE_DATA);
    hw_mouse_unsubscribe_int(&hw_state.mouse);
    
    vg_exit();

    return 0;
}
