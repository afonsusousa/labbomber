#include "rtc.h"
#include <stdbool.h>
#include <minix/syslib.h>

#define RTC_ADDR_REG 0x70
#define RTC_DATA_REG 0x71
#define RTC_REG_A 0x0A
#define RTC_REG_B 0x0B
#define RTC_REG_DAY 0x07
#define RTC_REG_MONTH 0x08
#define RTC_REG_YEAR 0x09
#define RTC_UIP_MSK (1 << 7)
#define RTC_DM_MSK (1 << 2)

static int bcd_to_bin(uint8_t bcd) {
    int tens = (bcd >> 4) & 0x0F;
    int units = bcd & 0x0F;
    return tens * 10 + units;
}

static int rtc_req_read(uint8_t req, uint8_t *data) {
    uint32_t raw = 0;

    if (data == NULL)
        return 1;

    if (sys_outb(RTC_ADDR_REG, req) != 0)
        return 1;

    if (sys_inb(RTC_DATA_REG, &raw) != 0)
        return 1;

    *data = (uint8_t)(raw & 0xFF);
    return 0;
}

static int rtc_wait_for_stable_update(void) {
    uint8_t reg_a = 0;

    do {
        if (rtc_req_read(RTC_REG_A, &reg_a) != 0)
            return 1;
    } while (reg_a & RTC_UIP_MSK);

    return 0;
}

int rtc_read_date(rtc_date *date) {
    uint8_t reg_b = 0;
    uint8_t day = 0;
    uint8_t month = 0;
    uint8_t year = 0;
    bool data_mode_binary = false;

    if (date == NULL)
        return 1;

    if (rtc_wait_for_stable_update() != 0)
        return 1;

    if (rtc_req_read(RTC_REG_B, &reg_b) != 0)
        return 1;

    data_mode_binary = (reg_b & RTC_DM_MSK) != 0;

    if (rtc_wait_for_stable_update() != 0)
        return 1;
    if (rtc_req_read(RTC_REG_DAY, &day) != 0)
        return 1;

    if (rtc_wait_for_stable_update() != 0)
        return 1;
    if (rtc_req_read(RTC_REG_MONTH, &month) != 0)
        return 1;

    if (rtc_wait_for_stable_update() != 0)
        return 1;
    if (rtc_req_read(RTC_REG_YEAR, &year) != 0)
        return 1;

    if (!data_mode_binary) {
        day = (uint8_t)bcd_to_bin(day);
        month = (uint8_t)bcd_to_bin(month);
        year = (uint8_t)bcd_to_bin(year);
    }

    date->day = day;
    date->month = month;
    date->year = year;

    return 0;
}
