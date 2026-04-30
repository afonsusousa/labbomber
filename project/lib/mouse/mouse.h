#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../keyboard/i8042.h"

typedef struct {
    int         hook_id;
    uint8_t     irq_bit;
    uint32_t    mask;
    int         x;
    int         y;
    int16_t     delta_x;
    int16_t     delta_y;
    bool        left_click;
    bool        right_click;
    bool        middle_click;
    uint8_t     byte_index;
    uint8_t     packet[3];
} mouse_t;

void    mouse_init(mouse_t *mouse);
int     mouse_subscribe_int(mouse_t *mouse);
int     mouse_unsubscribe_int(mouse_t *mouse);
bool    mouse_ih(mouse_t *mouse);
int     mouse_write_cmd(uint8_t cmd);
