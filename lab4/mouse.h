#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

int mouse_subscribe_int(uint8_t *bit_no);
int mouse_unsubscribe_int();

void (mouse_ih)();

int mouse_enable_data_reporting();
int mouse_disable_data_reporting();

#endif