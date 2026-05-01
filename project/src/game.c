#include "game.h"
#include <string.h>

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
