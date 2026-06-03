#include <stdio.h>
#include <lcom/lcf.h>
#include <string.h>
#include "../lib/timer/timer.h"
#include "../lib/keyboard/keyboard.h"
#include "../lib/mouse/mouse.h"
#include "../lib/keyboard/i8042.h"
#include "../lib/utils/utils.h"
#include "hardware.h"
#include "event_handlers.h"
#include "application.h"
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
    hw_vbe_init(&hw_state->video, 0x11A);
    
    hw_state->is_running = true;
    
    hw_state->mouse.max_x = hw_state->video.screen_width;
    hw_state->mouse.max_y = hw_state->video.screen_height;
    hw_state->mouse.x = (hw_state->video.screen_width / 3) * 2;
    hw_state->mouse.y = (hw_state->video.screen_height / 3) * 2;
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
    memset(&hw_state, 0, sizeof(hw_state));
    init_hardware_state(&hw_state);

    t_ctx app;
    memset(&app, 0, sizeof(app));
    app_update_real_time(&app);
    gui_init(&app, hw_state.video.screen_width, hw_state.video.screen_height);

    if (timer_set_frequency(0, 60) != 0) return 1;
    if (hw_timer_subscribe_int(&hw_state.timer) != 0) return 1;
    if (hw_keyboard_subscribe_int(&hw_state.keyboard) != 0) return 1;
    if (hw_mouse_subscribe_int(&hw_state.mouse) != 0) return 1;
    if (mouse_write_cmd(MOUSE_ENABLE_DATA) != 0) return 1;
    init_sprite_cache();

    int ipc_status;
    message msg;
    bool esc_was_pressed = false;

    while (app.gui.is_running) {
        if (driver_receive(ANY, &msg, &ipc_status) != 0) {
            printf("driver_receive failed\n");
            continue;
        }

        if (is_ipc_notify(ipc_status) && _ENDPOINT_P(msg.m_source) == HARDWARE) {
            if (msg.m_notify.interrupts & hw_state.timer.mask)
                handle_timer(&hw_state, &app);
            if (msg.m_notify.interrupts & hw_state.keyboard.mask) 
                handle_keyboard(&hw_state, &app, &esc_was_pressed);
            if (msg.m_notify.interrupts & hw_state.mouse.mask)
                handle_mouse(&hw_state, &app);
        }
    }

    gui_destroy(&app.gui);
    game_state_destroy(&app.game);

    hw_timer_unsubscribe_int(&hw_state.timer);
    hw_keyboard_unsubscribe_int(&hw_state.keyboard);
    mouse_write_cmd(MOUSE_DISABLE_DATA);
    hw_mouse_unsubscribe_int(&hw_state.mouse);
    
    vg_exit();

    return 0;
}
