#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include <lcom/lcf.h>
#include "i8250.h"

int get_lcr(uint8_t *lcr);
int set_bit_rate(uint16_t bit_rate);
int setup_lcr(int length, int stop);
int get_ier(uint8_t *p);
int ier_enable_receive(void);
int serial_subscribe_int(uint8_t *bit_no);
int set_fcr(uint8_t fcr);
int fifo_en(void);
int send_char(uint8_t char_send);
int serp_undo(void);

#endif /* SERIAL_PORT_H */
