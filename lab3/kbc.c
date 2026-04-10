#include <lcom/lcf.h>

#include <stdbool.h>
#include <stdint.h>

#include "kbc.h"

static int      kbc_hook_id = 1;
static uint8_t  current_scancode = 0;
static bool     has_error = false;

int kbc_subscribe_int(uint8_t *bit_no) {
  if (bit_no == NULL) return 1; // validate pointer

  *bit_no = kbc_hook_id; // store the hook ID bit for notifications

  // Subscribe to keyboard interrupts using exclusive mode so the process
  // receives IRQ1 notifications directly.
  if (sys_irqsetpolicy(KBC_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &kbc_hook_id) != OK) return 1;

  return 0;
}

int kbc_unsubscribe_int() {
  // Remove the keyboard interrupt subscription policy.
  if (sys_irqrmpolicy(&kbc_hook_id) != OK) return 1;
  return 0;
}

void (kbc_ih)() {
  uint8_t status = 0;

  has_error = true; // assume an error until a clean read occurs

  if (util_sys_inb(KBC_STATUS_REG, &status) != OK) {
    return; // cannot read status register
  }

  if (!KBC_OBF_FULL(status)) return; // no data available yet

  if (util_sys_inb(KBC_OUTBUF_REG, &current_scancode) != OK) {
    return; // cannot read output buffer
  }

  // If there is a parity or timeout error, or the byte is from the mouse,
  // keep has_error true and discard the scancode.
  if (ERROR_PARITY(status) || ERROR_TIMEOUT(status) || KBC_AUX_DATA(status)) return;

  has_error = false; // successful scancode read
}

uint8_t get_current_scancode() {
  return current_scancode;
}

bool check_kbc_error() {
  return has_error;
}
