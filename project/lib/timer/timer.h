#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <lcom/lcf.h>
#include "i8254.h"

typedef struct {
    int         hook_id;
    uint8_t     irq_bit;
    uint32_t    mask;
    uint32_t    ticks;
} timer_state_t;

void        timer_init(timer_state_t *timer);
int         timer_subscribe_int(timer_state_t *timer);
int         timer_unsubscribe_int(timer_state_t *timer);
void        timer_int_handler(timer_state_t *timer);
int         timer_set_frequency(uint8_t timer, uint32_t freq);
uint32_t    timer_get_no_interrups(const timer_state_t *timer);
void        timer_reset_ticks(timer_state_t *timer);
bool        timer_elapsed(const timer_state_t *timer, uint32_t start_tick, uint32_t ticks_to_wait);

