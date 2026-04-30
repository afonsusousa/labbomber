#pragma once

#include <stdint.h>
#include <stdbool.h>

#define RTC_IRQ 8

typedef struct {
    int         hook_id;
    uint8_t     irq_bit;
    uint32_t    mask;
    uint8_t     year;
    uint8_t     month;
    uint8_t     day;
    uint8_t     hours;
    uint8_t     minutes;
    uint8_t     seconds;
} rtc_t;

void rtc_init(rtc_t *rtc);
int rtc_subscribe_int(rtc_t *rtc);
int rtc_unsubscribe_int(rtc_t *rtc);
int rtc_get_time(rtc_t *info);
