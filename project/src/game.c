#include "game.h"
#include <string.h>

void init_hardware_state(hardware_t *hw_state) {
    if (!hw_state) return;

    memset(hw_state, 0, sizeof(hardware_t));

    hw_timer_init(&hw_state->timer);
    hw_rtc_init(&hw_state->time_info);
    hw_keyboard_init(&hw_state->keyboard);
    hw_mouse_init(&hw_state->mouse);
    
    hw_state->is_running = true;
    
    hw_state->mouse.x = 400;
    hw_state->mouse.y = 300; 
    
    //placeholders
    hw_state->video.screen_width = 800;
    hw_state->video.screen_height = 600;
    hw_state->video.bytes_per_pixel = 2;
}
