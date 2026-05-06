#ifndef LCOM_PROJECT_HARDWARE_H
#define LCOM_PROJECT_HARDWARE_H

#include <stdint.h>
#include <stdbool.h>

#include "../lib/timer/timer.h"
#include "../lib/rtc/rtc.h"
#include "../lib/keyboard/keyboard.h"
#include "../lib/mouse/mouse.h"
#include "../lib/vbe/vbe.h"

typedef struct {
    hw_timer_t     timer;
    hw_rtc_t       time_info;
    hw_mouse_t     mouse;
    hw_keyboard_t  keyboard;
    hw_video_t     video;

    bool           is_running;
} hardware_t;

void init_hardware_state(hardware_t *hw_state);

#endif /* LCOM_PROJECT_HARDWARE_H */
