#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <lcom/lcf.h>

uint8_t lsb(uint16_t bytes) { return (bytes & 0xFF); }

uint8_t msb(uint16_t bytes) { return ((bytes >> 8) & 0xFF); }

bool is_single_byte(uint16_t keycode) { return (keycode <= 0xFF); }

uint16_t break_from_make(uint16_t keycode)
{
    if (is_single_byte(keycode))
        return (keycode | (1 << 7));
    return (keycode | (1 << 15));
}

int (util_sys_inb)(int port, uint8_t *value){
    uint32_t temp;
    if (sys_inb(port, &temp) != OK) return 1;

    *value = (uint8_t) temp;
    return 0;
}
