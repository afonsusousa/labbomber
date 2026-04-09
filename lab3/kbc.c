#include <lcom/lcf.h>
#include <stdint.h>
#include "kbc.h"

static int      kbc_hook_id = 1;
static uint8_t  current_scancode = 0;
static bool     has_error = false;

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

void (kbc_ih)() {
  uint8_t status;
  has_error = false;

  // Read status register
  if (util_sys_inb(KBC_STATUS_REG, &status) != OK) {
    has_error = true;
    return;
  }

  // Read output buffer
  if (util_sys_inb(KBC_OUTBUF_REG, &current_scancode) != OK) {
    has_error = true;
    return;
  }

  // Check for errors
  if (ERROR_PARITY(status) || ERROR_TIMEOUT(status)) {
    has_error = true;
  }
}

uint8_t get_current_scancode() {
  return current_scancode;
}

bool check_kbc_error() {
  return has_error;
}
