#include <lcom/lcf.h>
#include "serial_port.h"

int get_lcr(uint8_t *lcr) {
    return util_sys_inb(COM1_ADDR + SERP_LCR, lcr);
}

int set_bit_rate(uint16_t bit_rate) {
    uint8_t lcr = 0x00;

    if (get_lcr(&lcr) != 0) return 1;

    if (sys_outb(COM1_ADDR + SERP_LCR, lcr | LCR_DLAB) != 0) {
        printf("bit rate error\n");
        return 1;
    }

    uint8_t msb;
    uint8_t lsb;
    util_get_MSB(FIXED_FREQUENCY / bit_rate, &msb);
    util_get_LSB(FIXED_FREQUENCY / bit_rate, &lsb);

    if (sys_outb(COM1_ADDR + SERP_DLM, msb) != 0) return 1;
    if (sys_outb(COM1_ADDR + SERP_DLL, lsb) != 0) return 1;

    return sys_outb(COM1_ADDR + SERP_LCR, lcr & ~LCR_DLAB);
}

// criar line control register -> UART tem de ter os msm parametros
int setup_lcr(int length, int stop) {
    uint8_t lcr = 0x00;

    if (get_lcr(&lcr) != 0) return 1;

    switch (length) {
    case 5:
        break;
    case 6:
        lcr |= LCR_WORD_LENGTH_6;
        break;
    case 7:
        lcr |= LCR_WORD_LENGTH_7;
        break;
    case 8:
        lcr |= LCR_WORD_LENGTH_8;
        break;
    default:
        return 1;
    }

    switch (stop) {
    case 1:
        break;
    case 2:
        lcr |= LCR_STOP_2;
        break;
    default:
        return 1;
    }

    /* Parity even for now. */
    lcr |= LCR_PARITY_EVEN;

    return sys_outb(COM1_ADDR + SERP_LCR, lcr);
}

int get_ier(uint8_t *p) {
    return util_sys_inb(COM1_ADDR + SERP_IER, p);
}

/* Interrupt enable register: enable receive data interrupt only. */
int ier_enable_receive(void) {
    uint8_t ier = 0x00;

    if (get_ier(&ier) != 0) return 1;

    ier |= IER_ENREC_INT;
    return sys_outb(COM1_ADDR + SERP_IER, ier);
}

/* Subscribe interrupts. */
int serial_hook_id = 6;

int serial_subscribe_int(uint8_t *bit_no) {
    if (bit_no == NULL) return 1;

    *bit_no = BIT(serial_hook_id);
    return sys_irqsetpolicy(COM1_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &serial_hook_id);
}

int set_fcr(uint8_t fcr) {
    if (sys_outb(COM1_ADDR + SERP_FCR, fcr) != 0) return 1;
    return 0;
}

/* Create FIFO control register and enable FIFO. */
int fifo_en(void) {
    uint8_t fcr = 0x00;
    fcr |= FCR_EN_1;
    return set_fcr(fcr);
}

int send_char(uint8_t char_send) {
    if (sys_outb(COM1_ADDR, char_send) != 0) {
        printf("send char error\n");
        return 1;
    }
    return 0;
}

int serp_undo() {
    if (sys_irqrmpolicy(&serial_hook_id) != 0) return 1;
    if (set_fcr(0x00) != 0) return 1;

    uint8_t ier = 0x00;
    if (get_ier(&ier) != 0) return 1;
    if (sys_outb(COM1_ADDR + SERP_IER, ier & ~IER_ENREC_INT) != 0) return 1;
    return 0;
}

