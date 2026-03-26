#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

uint8_t lsb(uint16_t bytes) { return (bytes & 0xFF); }

uint8_t msb(uint16_t bytes) { return ((bytes >> 8) & 0xFF); }

bool is_single_byte(uint16_t keycode) { return (keycode <= 0xFF); }

uint16_t break_from_make(uint16_t keycode)
{
    if (is_single_byte(keycode))
        return (keycode | (1 << 7));
    return (keycode | (1 << 15));
}
