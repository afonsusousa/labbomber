#include <lcom/lcf.h>
#include <lcom/timer.h>
#include <stdbool.h>

#include "mouse.h"
#include "i8042.h"

static int hook_id = 2;
static uint8_t current_mouse_byte = 0;
static bool ih_error = true;

#define ST_PARITY_BIT  BIT(7)
#define ST_TIMEOUT_BIT BIT(6)
#define ST_AUX_BIT     BIT(5)
#define ST_IBF_BIT     BIT(1)
#define ST_OBF_BIT     BIT(0)

#define KBC_MAX_TRIES  10
#define KBC_DELAY_US   20000

static int kbc_read_status(uint8_t *status) {
  return util_sys_inb(KBC_CMD_REG, status);
}

static int kbc_write_to_mouse(uint8_t cmd) {
  uint8_t status = 0;

  for (int i = 0; i < KBC_MAX_TRIES; i++) {
    if (kbc_read_status(&status) != OK) return 1;

    if (!(status & ST_IBF_BIT)) {
      if (sys_outb(KBC_CMD_REG, KBC_WRITE_MOUSE) != OK) return 1;
      if (sys_outb(KBC_DATA_REG, cmd) != OK) return 1;
      return 0;
    }

    tickdelay(micros_to_ticks(KBC_DELAY_US));
  }

  return 1;
}

static int mouse_read_response(uint8_t *resp) {
  uint8_t status = 0;

  for (int i = 0; i < KBC_MAX_TRIES; i++) {
    if (kbc_read_status(&status) != OK) return 1;

    if (status & ST_OBF_BIT) {
      if (util_sys_inb(KBC_DATA_REG, resp) != OK) return 1;

      if (status & (ST_PARITY_BIT | ST_TIMEOUT_BIT)) return 1;
      if (!(status & ST_AUX_BIT)) return 1;

      return 0;
    }

    tickdelay(micros_to_ticks(KBC_DELAY_US));
  }

  return 1;
}

int mouse_subscribe_int(uint8_t *bit_no) {
    if (bit_no == NULL) return 1;
    *bit_no = hook_id;
    return sys_irqsetpolicy(IRQ_MOUSE, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id);
}

int mouse_unsubscribe_int() {
    return sys_irqrmpolicy(&hook_id);
}

void (mouse_ih)() {
    uint8_t status = 0;
    ih_error = true;

    if (kbc_read_status(&status) != OK) return;
    if (!(status & ST_OBF_BIT)) return;
    if (util_sys_inb(KBC_DATA_REG, &current_mouse_byte) != OK) return;
    if (!(status & ST_AUX_BIT)) return;
    if (status & (ST_PARITY_BIT | ST_TIMEOUT_BIT)) return;

    ih_error = false;
}

int mouse_enable_data_reporting() {
    uint8_t resp = 0;

    for (int i = 0; i < KBC_MAX_TRIES; i++) {
      if (kbc_write_to_mouse(MOUSE_ENABLE_DATA) != 0) return 1;
      if (mouse_read_response(&resp) != 0) return 1;

      if (resp == MOUSE_ACK) return 0;
      if (resp == MOUSE_ERROR) return 1;
    }

    return 1;
}

int mouse_disable_data_reporting() {
    uint8_t resp = 0;

    for (int i = 0; i < KBC_MAX_TRIES; i++) {
      if (kbc_write_to_mouse(MOUSE_DISABLE_DATA) != 0) return 1;
      if (mouse_read_response(&resp) != 0) return 1;

      if (resp == MOUSE_ACK) return 0;
      if (resp == MOUSE_ERROR) return 1;
    }

    return 1;
}

uint8_t mouse_get_byte() {
  return current_mouse_byte;
}

bool mouse_has_error() {
  return ih_error;
}
