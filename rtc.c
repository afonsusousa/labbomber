#include "rtc.h"
#include <minix/syslib.h>
#include <minix/drivers.h>

#define RTC_ADDR_REG    0x70
#define RTC_DATA_REG    0x71
#define RTC_REG_A       0x0A
#define RTC_REG_B       0x0B
#define RTC_REG_CENTURY 0x32  // Optional century register
#define RTC_REG_DAY     0x07
#define RTC_REG_MONTH   0x08
#define RTC_REG_YEAR    0x09
#define RTC_UIP_MSK     (1 << 7)
#define RTC_DM_MSK      (1 << 2)

static int bcd_to_bin(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

int rtc_read_date(rtc_date *date) {
    if (date == NULL) return -1;

    uint32_t day, month, year, status_a, status_b;
    int is_bcd;

    // 1. Wait until the Update In Progress (UIP) bit clears
    // This ensures we don't read the registers while they are being updated.
    do {
        sys_outb(RTC_ADDR_REG, RTC_REG_A);
        sys_inb(RTC_DATA_REG, &status_a);
    } while (status_a & RTC_UIP_MSK);

    // 2. Read Status Register B to check Data Mode (DM)
    // DM = 0 means BCD mode, DM = 1 means Binary mode.
    sys_outb(RTC_ADDR_REG, RTC_REG_B);
    sys_inb(RTC_DATA_REG, &status_b);
    is_bcd = !(status_b & RTC_DM_MSK);

    // 3. Read raw Day, Month, and Year values
    sys_outb(RTC_ADDR_REG, RTC_REG_DAY);
    sys_inb(RTC_DATA_REG, &day);

    sys_outb(RTC_ADDR_REG, RTC_REG_MONTH);
    sys_inb(RTC_DATA_REG, &month);

    sys_outb(RTC_ADDR_REG, RTC_REG_YEAR);
    sys_inb(RTC_DATA_REG, &year);

    // 4. Convert values from BCD to Binary if necessary
    if (is_bcd) {
        day   = bcd_to_bin((uint8_t)day);
        month = bcd_to_bin((uint8_t)month);
        year  = bcd_to_bin((uint8_t)year);
    }

    // 5. Fill the rtc_date structure
    // Since date->year is uint8_t, we provide the 2-digit year (e.g., 26).
    // Storing 2026 in an 8-bit field causes overflow (resulting in 234).
    date->day   = (uint8_t)day;
    date->month = (uint8_t)month;
    date->year  = (uint8_t)year; 

    // Optional debug line to verify output in your terminal
    // printf("RTC READ: %02d/%02d/%02d\n", date->day, date->month, date->year);

    return 0;
}