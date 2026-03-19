#include <lcom/lcf.h>

#include <stdint.h>

int(util_get_LSB)(uint16_t val, uint8_t *lsb) {
  *lsb = val & 0xFF;
  return 0;
}

int(util_get_MSB)(uint16_t val, uint8_t *msb) {
  *msb = (val >> 8) & 0xFF;
  return 0;
}

// Reads a byte from a hardware port
int (util_sys_inb)(int port, uint8_t *value) {

  uint32_t temp; // temporary variable to store the 32-bit value read from the port

  // read from the specified port
  if (sys_inb(port, &temp) != OK) return 1;

  // convert the 32-bit value to 8 bits and store it
  *value = (uint8_t) temp;

  return 0;
}
