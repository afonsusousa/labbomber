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
} keyboard_t;

void    keyboard_init(keyboard_t *kbd);
int     keyboard_subscribe_int(keyboard_t *kbd);
int     keyboard_unsubscribe_int(keyboard_t *kbd);
void    keyboard_ih(keyboard_t *kbd);
