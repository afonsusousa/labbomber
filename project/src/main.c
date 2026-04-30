#include <minix/syslib.h>
#include <minix/drivers.h>
#include <minix/endpoint.h>
#include <minix/sysutil.h>
#include <stdio.h>

#include "timer/timer.h"
#include "../lib/keyboard/keyboard.h"
#include "../lib/mouse/mouse.h"
#include "../lib/keyboard/i8042.h"
#include "utils/utils.h"
#include "game.h"

int main(int argc, char *argv[]) {
    hardware_t hw_state;
    init_hardware_state(&hw_state);

    if (timer_set_frequency(0, 144 /*fps*/) != 0) return 1;
    if (timer_subscribe_int(&hw_state.timer) != 0) return 1;
    if (keyboard_subscribe_int(&hw_state.keyboard) != 0) return 1;
    if (mouse_subscribe_int(&hw_state.mouse) != 0) return 1;
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
                timer_int_handler(&hw_state.timer); // Updates the internal clock
                //update state
                //draw things
            }
            
            //keyboard
            if (msg.m_notify.interrupts & hw_state.keyboard.mask) {
                keyboard_ih(&hw_state.keyboard);
                if (hw_state.keyboard.keys_pressed[0x01]) {
                    hw_state.is_running = false;
                }
            }
            
            //mouse
            if (msg.m_notify.interrupts & hw_state.mouse.mask) {
                if (mouse_ih(&hw_state.mouse)) {
                    // do something
                }
            }
        }
    }

    timer_unsubscribe_int(&hw_state.timer);
    keyboard_unsubscribe_int(&hw_state.keyboard);
    mouse_write_cmd(MOUSE_DISABLE_DATA);
    mouse_unsubscribe_int(&hw_state.mouse);
    
    // vg_exit();

    return 0;
}
