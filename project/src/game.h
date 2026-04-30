#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../lib/timer/timer.h"
#include "../lib/rtc/rtc.h"
#include "../lib/keyboard/keyboard.h"
#include "../lib/mouse/mouse.h"
#include "../lib/vbe/vbe.h"

typedef struct {
    timer_state_t   timer;
    rtc_t           time_info;
    mouse_t         mouse;
    keyboard_t      keyboard;
    video_t         video;

    bool            is_running; //move from here
} hardware_t;

void init_hardware_state(hardware_t *hw_state);
