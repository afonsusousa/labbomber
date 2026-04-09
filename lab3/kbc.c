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


// Lê um byte do Output Buffer com verificação de status
int kbc_read_outbuf(uint8_t *byte) {
  uint8_t status;
  uint32_t tries = 0;

  while (tries < KBC_MAX_TRIES) {
    if (util_sys_inb(KBC_STATUS_REG, &status) != OK) return 1;

    if (ERROR_PARITY(status) || ERROR_TIMEOUT(status)) return 1;

    if (KBC_OBF_FULL(status)) {
      if (KBC_AUX_DATA(status)) return 1; // são dados do rato, ignorar
      return util_sys_inb(KBC_OUTBUF_REG, byte);
    }

    tickdelay(micros_to_ticks(KBC_DELAY_US));
    tries++;
  }
  return 1; // timeout
}

// Escreve um comando no KBC (espera IBF estar livre)
int kbc_write_cmd(uint8_t cmd) {
  uint8_t status;
  uint32_t tries = 0;

  while (tries < KBC_MAX_TRIES) {
    if (util_sys_inb(KBC_STATUS_REG, &status) != OK) return 1;

    if (!KBC_IBF_FULL(status)) {
      return sys_outb(KBC_CMD_REG, cmd);
    }

    tickdelay(micros_to_ticks(KBC_DELAY_US));
    tries++;
  }
  return 1; // timeout
}

// Escreve um argumento no Input Buffer (espera IBF estar livre)
int kbc_write_arg(uint8_t arg) {
  uint8_t status;
  uint32_t tries = 0;

  while (tries < KBC_MAX_TRIES) {
    if (util_sys_inb(KBC_STATUS_REG, &status) != OK) return 1;

    if (!KBC_IBF_FULL(status)) {
      return sys_outb(KBC_INBUF_REG, arg);
    }

    tickdelay(micros_to_ticks(KBC_DELAY_US));
    tries++;
  }
  return 1;
}



