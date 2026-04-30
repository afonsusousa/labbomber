#include "rtc.h"
#include "../utils/utils.h"
#include <minix/syslib.h>
#include <minix/drivers.h>

#define RTC_ADDR_REG    0x70
#define RTC_DATA_REG    0x71

#define RTC_REG_SECONDS 0x00
#define RTC_REG_MINUTES 0x02
#define RTC_REG_HOURS   0x04
#define RTC_REG_DAY     0x07
#define RTC_REG_MONTH   0x08
#define RTC_REG_YEAR    0x09

#define RTC_REG_A       0x0A
#define RTC_REG_B       0x0B
#define RTC_UIP_MSK     BIT(7)
#define RTC_DM_MSK      BIT(2)

static uint8_t bcd_to_bin(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

void hw_rtc_init(hw_rtc_t *rtc) {
    if (rtc == NULL) return;

    rtc->hook_id = RTC_IRQ;
    rtc->irq_bit = RTC_IRQ;
    rtc->mask = BIT(rtc->irq_bit);
}

static int rtc_read_reg(uint8_t reg, uint8_t *data) {
    if (sys_outb(RTC_ADDR_REG, reg) != 0) return 1;
    
    if (util_sys_inb(RTC_DATA_REG, data) != 0) return 1;
    
    return 0;
}

static int rtc_wait_stable(void) {
    uint8_t reg_a = 0;
    do {
        if (rtc_read_reg(RTC_REG_A, &reg_a) != 0) return 1;
    } while (reg_a & RTC_UIP_MSK);
    return 0;
}

int hw_rtc_subscribe_int(hw_rtc_t *rtc) {
    if (rtc == NULL) return 1;

    if (rtc->hook_id == 0) rtc->hook_id = RTC_IRQ;
    rtc->irq_bit = rtc->hook_id;
    rtc->mask = BIT(rtc->irq_bit);

    return sys_irqsetpolicy(RTC_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &rtc->hook_id);
}

int hw_rtc_unsubscribe_int(hw_rtc_t *rtc) {
    if (rtc == NULL) return 1;

    return sys_irqrmpolicy(&rtc->hook_id);
}

int hw_rtc_get_time(hw_rtc_t *info) {
    if (info == NULL) return 1;

    uint8_t reg_b = 0;
    
    if (rtc_wait_stable() != 0) return 1;

    if (rtc_read_reg(RTC_REG_SECONDS, &info->seconds) != 0) return 1;
    if (rtc_read_reg(RTC_REG_MINUTES, &info->minutes) != 0) return 1;
    if (rtc_read_reg(RTC_REG_HOURS, &info->hours) != 0) return 1;
    if (rtc_read_reg(RTC_REG_DAY, &info->day) != 0) return 1;
    if (rtc_read_reg(RTC_REG_MONTH, &info->month) != 0) return 1;
    if (rtc_read_reg(RTC_REG_YEAR, &info->year) != 0) return 1;

    if (rtc_read_reg(RTC_REG_B, &reg_b) != 0) return 1;

    bool is_bcd = !(reg_b & RTC_DM_MSK);

    if (is_bcd) {
        info->seconds = bcd_to_bin(info->seconds);
        info->minutes = bcd_to_bin(info->minutes);
        info->hours = bcd_to_bin(info->hours);
        info->day = bcd_to_bin(info->day);
        info->month = bcd_to_bin(info->month);
        info->year = bcd_to_bin(info->year);
    }

    return 0;
}
