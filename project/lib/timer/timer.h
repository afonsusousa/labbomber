#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "i8254.h"

typedef struct {
    int         hook_id;
    uint8_t     irq_bit;
    uint32_t    mask;
    uint32_t    ticks;
} hw_timer_t;

void        hw_timer_init(hw_timer_t *timer);
int         hw_timer_subscribe_int(hw_timer_t *timer);
int         hw_timer_unsubscribe_int(hw_timer_t *timer);
void        hw_timer_int_handler(hw_timer_t *timer);
int         timer_set_frequency(uint8_t timer, uint32_t freq);
uint32_t    hw_timer_get_no_interrups(const hw_timer_t *timer);
void        hw_timer_reset_ticks(hw_timer_t *timer);
bool        hw_timer_elapsed(const hw_timer_t *timer, uint32_t start_tick, uint32_t ticks_to_wait);

