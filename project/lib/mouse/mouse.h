#ifndef LIB_MOUSE_MOUSE_H
#define LIB_MOUSE_MOUSE_H
#include <stdint.h>
#include <stdbool.h>
#include "../keyboard/i8042.h"

typedef struct {
    int         hook_id;
    uint8_t     irq_bit;
    uint32_t    mask;

    int32_t     x;
    int32_t     y;

    int32_t     max_x;
    int32_t     max_y;

    int16_t     delta_x;
    int16_t     delta_y;

    bool        left_click;
    bool        right_click;
    bool        middle_click;
    uint8_t     byte_index;
    uint8_t     packet[3];
} hw_mouse_t;

void    hw_mouse_init(hw_mouse_t *mouse);
int     hw_mouse_subscribe_int(hw_mouse_t *mouse);
int     hw_mouse_unsubscribe_int(hw_mouse_t *mouse);
bool    hw_mouse_ih(hw_mouse_t *mouse);
int     mouse_write_cmd(uint8_t cmd);

#endif /* LIB_MOUSE_MOUSE_H */
