#include "rtc.h"
#include "stdbool.h"
#include <minix/syslib.h>

#define TODO return -1

#define RTC_ADDR_REG 0x70
#define RTC_DATA_REG 0x71
#define RTC_REG_A 0x0A
#define RTC_REG_B 0x0B
#define RTC_REG_DAY 0x07
#define RTC_REG_MONTH 0x08
#define RTC_REG_YEAR 0x09
#define RTC_UIP_MSK (1 << 7)
#define RTC_DM_MSK (1 << 2)
#define RTC_1224_MSK (1)


static int bcd_to_bin(uint8_t bcd) { 
    int left = 0;
    int right = 0;

    left = (bcd >> 4);
    right = (bcd & 0xFF);

    return (left | right);
}

static int rtc_req_read(int req, uint32_t *data)
{
    int status = -1;
    status = sys_outb(RTC_ADDR_REG, req);
    if (status < 0)
        printf("error1");
    status= sys_inb(RTC_DATA_REG, data);
    if (status < 0)
        printf("error2");
    return (status);
}

int rtc_read_date(rtc_date *date) { 

    bool data_mode = false, twenty4 = false; 
    uint32_t data = 0;
    rtc_req_read(RTC_REG_A, &data);

    // stall while update in progress
    while (data & RTC_UIP_MSK) rtc_req_read(RTC_REG_A, &data);

    // Ctrl bits
    rtc_req_read(RTC_REG_B, &data);
    data_mode = data & RTC_DM_MSK; // 0 -> BCD, 1 -> BIN
    twenty4 = data & RTC_1224_MSK; // 0 -> 12, 1 -> 24

    // Day
    while (data & RTC_UIP_MSK) rtc_req_read(RTC_REG_A, &data);
    rtc_req_read(RTC_REG_DAY, &data);
    if (!data_mode) // 
        data = bcd_to_bin(data);
    date->day = data;

    // Month
    while (data & RTC_UIP_MSK) rtc_req_read(RTC_REG_A, &data);
    rtc_req_read(RTC_REG_MONTH, &data);
    if (!data_mode) // 
        data = bcd_to_bin(data);
    date->month = data;

    // Year
    while (data & RTC_UIP_MSK) rtc_req_read(RTC_REG_A, &data);
    rtc_req_read(RTC_REG_YEAR, &data);
    if (!data_mode) // 
        data = bcd_to_bin(data);
    date->year = data;

    return (0);
}
