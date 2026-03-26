#include <lcom/lcf.h>
#include <stdint.h>
#include "kbc.h"

int kbc_hook_id = 1;

// TODO : este bit_no = hook_id nao faz sentido

int kbc_subscribe_int(uint8_t *bit_no) {
  if (bit_no == NULL) return 1; // validar ponteiro

  *bit_no = kbc_hook_id; // guardar bit

  if (sys_irqsetpolicy(KBC_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &kbc_hook_id) != OK) return 1; // EXCLUSIVE STEALS THE KEYBOARD FOR US

  return 0;
}

int kbc_unsubscribe_int() {
  if (sys_irqrmpolicy(&kbc_hook_id) != OK) return 1; // remover interrupções
  return 0;
}