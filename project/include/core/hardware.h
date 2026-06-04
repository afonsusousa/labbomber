#ifndef LCOM_PROJECT_HARDWARE_H
#define LCOM_PROJECT_HARDWARE_H

#include <stdint.h>
#include <stdbool.h>

#include "timer.h"
#include "rtc.h"
#include "keyboard.h"
#include "mouse.h"
#include "vbe.h"

typedef struct {
    hw_timer_t     timer;
    hw_rtc_t       time_info;
    hw_mouse_t     mouse;
    hw_keyboard_t  keyboard;
    hw_video_t     video;
    uint8_t        serial_mask;

    bool           is_running;
} hardware_t;

void init_hardware_state(hardware_t *hw_state);

#endif /* LCOM_PROJECT_HARDWARE_H */
