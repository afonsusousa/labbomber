#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "i8042.h"

typedef struct {
    int         hook_id;
    uint8_t     irq_bit;
    uint32_t    mask;
    uint8_t     scancode;
    bool        is_two_bytes;
    bool        keys_pressed[256];
} hw_keyboard_t;

void    hw_keyboard_init(hw_keyboard_t *kbd);
int     hw_keyboard_subscribe_int(hw_keyboard_t *kbd);
int     hw_keyboard_unsubscribe_int(hw_keyboard_t *kbd);
void    hw_keyboard_ih(hw_keyboard_t *kbd);
