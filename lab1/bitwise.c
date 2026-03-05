#include "bitwise.h"

#define TODO return 255

#include <stdio.h>
static void print_bits(uint8_t msk)
{
    for (int i = 7; i >= 0; i--)
        printf("%d", (msk >> i) & 1);
}

uint8_t clear(uint8_t msk, int pos) { return (msk & ~(1 << pos)); }

uint8_t set(uint8_t msk, int pos) { return (msk | (1 << pos)); }

bool is_set(uint8_t msk, int pos) { return (msk & (1 << pos)); }

uint8_t lsb(uint16_t wide_msk) { return (wide_msk & 0xFF); }

uint8_t msb(uint16_t wide_msk) { return ((wide_msk >> 8) & 0xFF); }

#include <stdarg.h>
uint8_t mask(int pos, ...) { 
    
    uint8_t msk = 0;
    va_list args;
    va_start(args, pos);
    msk = set(msk, pos);
    while(1)
    {
        int current_pos = va_arg(args, int);
        if (current_pos == MSK_END)
            break ;
        msk = set(msk, current_pos);
    }
    va_end(args);
    return (msk);
}
