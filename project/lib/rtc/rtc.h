#ifndef LIB_RTC_RTC_H
#define LIB_RTC_RTC_H
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
} hw_rtc_t;

void    hw_rtc_init(hw_rtc_t *rtc);
int     hw_rtc_subscribe_int(hw_rtc_t *rtc);
int     hw_rtc_unsubscribe_int(hw_rtc_t *rtc);
int     hw_rtc_get_time(hw_rtc_t *info);

#endif /* LIB_RTC_RTC_H */
