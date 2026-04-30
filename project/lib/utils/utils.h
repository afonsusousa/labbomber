#ifndef _LCOM_UTILS_H_
#define _LCOM_UTILS_H_

#include <stdint.h>
#include <stdbool.h>

#ifndef BIT
#define BIT(n) (1 << (n))
#endif

uint8_t     lsb(uint16_t bytes);
uint8_t     msb(uint16_t bytes);
bool        is_single_byte(uint16_t keycode);
uint16_t    break_from_make(uint16_t keycode);
int         util_sys_inb(int port, uint8_t *value);

#endif /* _LCOM_UTILS_H_ */
