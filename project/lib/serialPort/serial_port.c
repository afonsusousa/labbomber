#include <lcom/lcf.h>
#include "serial_port.h"
#include "../utils/utils.h"

int serial_init(void) {
    if (setup_lcr(8, 1) != 0) return 1;
    if (set_bit_rate(9600) != 0) return 1;
    if (sys_outb(COM1_ADDR + SERP_IER, 0x00) != 0) return 1;
    if (fifo_en() != 0) return 1;
    if (setup_mcr() != 0) return 1;

    uint8_t lcr = 0, lsr = 0, msr = 0, ier = 0;
    get_lcr(&lcr);
    get_ier(&ier);
    util_sys_inb(COM1_ADDR + SERP_LSR, &lsr);
    util_sys_inb(COM1_ADDR + SERP_MSR, &msr);

    FILE *log_file = fopen("/tmp/game_debug.log", "a");
    if (log_file) {
        fprintf(log_file, "[SERIAL] init complete LCR=0x%02X IER=0x%02X LSR=0x%02X MSR=0x%02X\n",
                lcr, ier, lsr, msr);
        fclose(log_file);
    }

    return 0;
}

int setup_mcr(void) {
    return sys_outb(COM1_ADDR + SERP_MCR, MCR_DTR | MCR_RTS | MCR_OUT2);
}

int serial_send_byte(uint8_t b) {
    FILE *log_file = fopen("/tmp/game_debug.log", "a");
    uint8_t status = 0;
    for (int i = 0; i < 10000; i++) {
        if (util_sys_inb(COM1_ADDR + SERP_LSR, &status) != 0) {
            if (log_file) {
                fprintf(log_file, "[SERIAL] send_byte util_sys_inb failed\n");
                fclose(log_file);
            }
            return 1;
        }
        if (status & LSR_THR_EMPTY) {
            int result = send_char(b);
            if (log_file) {
                fprintf(log_file, "[SERIAL] send_byte(0x%02X) sent, result=%d\n", b, result);
                fclose(log_file);
            }
            return result;
        }
    }
    if (log_file) {
        fprintf(log_file, "[SERIAL] send_byte(0x%02X) timeout - THR never empty\n", b);
        fclose(log_file);
    }
    return 1;
}


bool serial_has_byte(void) {
    uint8_t status = 0;
    if (util_sys_inb(COM1_ADDR + SERP_LSR, &status) != 0) return false;
    bool has_data = (status & LSR_DATA_READY) != 0;
    
    static int poll_count = 0;
    if (++poll_count % 1000 == 0) {
        FILE *log_file = fopen("/tmp/game_debug.log", "a");
        if (log_file) {
            fprintf(log_file, "[SERIAL] serial_has_byte check: status=0x%02X, has_data=%d\n", status, has_data);
            fclose(log_file);
        }
    }
    
    return has_data;
}


int serial_read_byte(uint8_t *b) {
    if (b == NULL) return 1;

    uint8_t status = 0;
    if (util_sys_inb(COM1_ADDR + SERP_LSR, &status) != 0) return 1;
    if (!(status & LSR_DATA_READY)) return 1;
    
    int result = util_sys_inb(COM1_ADDR + SERP_DATA, b);
    FILE *log_file = fopen("/tmp/game_debug.log", "a");
    if (log_file) {
        fprintf(log_file, "[SERIAL] serial_read_byte: got 0x%02X, result=%d\n", *b, result);
        fclose(log_file);
    }
    return result;
}

void serial_flush_rx(void) {
    uint8_t status = 0;
    uint8_t discarded = 0;
    int count = 0;

    while (util_sys_inb(COM1_ADDR + SERP_LSR, &status) == 0 && (status & LSR_DATA_READY)) {
        if (util_sys_inb(COM1_ADDR + SERP_DATA, &discarded) != 0) break;
        count++;
    }

    FILE *log_file = fopen("/tmp/game_debug.log", "a");
    if (log_file) {
        fprintf(log_file, "[SERIAL] flushed %d stale RX byte(s)\n", count);
        fclose(log_file);
    }
}


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

    uint8_t msb_val;
    uint8_t lsb_val;
    msb_val = msb(FIXED_FREQUENCY / bit_rate);
    lsb_val = lsb(FIXED_FREQUENCY / bit_rate);

    if (sys_outb(COM1_ADDR + SERP_DLM, msb_val) != 0) return 1;
    if (sys_outb(COM1_ADDR + SERP_DLL, lsb_val) != 0) return 1;

    return sys_outb(COM1_ADDR + SERP_LCR, lcr & ~LCR_DLAB);
}

// criar line control register -> UART tem de ter os msm parametros
int setup_lcr(int length, int stop) {
    uint8_t lcr = 0x00;

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

    lcr |= LCR_PARITY_NONE;

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
    if (sys_outb(COM1_ADDR + SERP_DATA, char_send) != 0) {
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

