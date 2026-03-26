#include <lcom/lcf.h>

#include <stdint.h>

int(util_get_LSB)(uint16_t val, uint8_t *lsb) {
  *lsb = val & 0xFF; // extrair 8 bits menos significativos
  return 0;
}

int(util_get_MSB)(uint16_t val, uint8_t *msb) {
  *msb = (val >> 8) & 0xFF; // extrair 8 bits mais significativos
  return 0;
}

// Reads a byte from a hardware port
int (util_sys_inb)(int port, uint8_t *value) {

  uint32_t temp; // variável temporária (sys_inb devolve 32 bits)

  // read from the specified port
  if (sys_inb(port, &temp) != OK) return 1; // ler da porta

  // convert the 32-bit value to 8 bits and store it
  *value = (uint8_t) temp; // guardar apenas 8 bits

  return 0;
}
