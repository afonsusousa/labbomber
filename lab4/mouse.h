#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>
#include <stdbool.h>

int mouse_subscribe_int(uint8_t *bit_no);
int mouse_unsubscribe_int();

void (mouse_ih)();

int mouse_disable_data_reporting();
uint8_t mouse_get_byte();
bool mouse_has_error();
void mouse_build_packet(const uint8_t bytes[3], struct packet *pp);

#endif
