#ifndef _LCOM_UTILS_H_
#define _LCOM_UTILS_H_

#include <stdint.h>
#include <stdbool.h>

uint8_t lsb(uint16_t bytes);
uint8_t msb(uint16_t bytes);
bool is_single_byte(uint16_t keycode);
uint16_t break_from_make(uint16_t keycode);

#endif /* _LCOM_UTILS_H_ */
