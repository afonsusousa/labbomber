#include <lcom/lcf.h>
#include <lcom/timer.h>
#include <stdbool.h>

#include "mouse.h"
#include "i8042.h"

static int hook_id = 2;
static const int hook_id_default = 2;
static bool mouse_is_subscribed = false;
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

      // After 0xD4, wait again until IBF is empty before sending the mouse byte.
      for (int j = 0; j < KBC_MAX_TRIES; j++) {
        if (kbc_read_status(&status) != OK) return 1;
        if (!(status & ST_IBF_BIT)) {
          if (sys_outb(KBC_DATA_REG, cmd) != OK) return 1;
          return 0;
        }
        tickdelay(micros_to_ticks(KBC_DELAY_US));
      }

      return 1;
    }

    tickdelay(micros_to_ticks(KBC_DELAY_US));
  }

  return 1;
}

static int mouse_read_response(uint8_t *resp) {
  uint8_t status = 0;

  for (int i = 0; i < KBC_MAX_TRIES * 4; i++) {
    if (kbc_read_status(&status) != OK) return 1;

    if (status & ST_OBF_BIT) {
      uint8_t data = 0;
      if (util_sys_inb(KBC_DATA_REG, &data) != OK) return 1;

      // Ignore invalid bytes and keep waiting for a proper mouse response byte.
      if (status & (ST_PARITY_BIT | ST_TIMEOUT_BIT)) continue;
      if (!(status & ST_AUX_BIT)) continue;

      if (data == MOUSE_ACK || data == MOUSE_NACK || data == MOUSE_ERROR) {
        *resp = data;
        return 0;
      }

      continue;
    }

    tickdelay(micros_to_ticks(KBC_DELAY_US));
  }

  return 1;
}

int mouse_subscribe_int(uint8_t *bit_no) {
    if (bit_no == NULL) return 1;
    hook_id = hook_id_default;
    *bit_no = hook_id;
    if (sys_irqsetpolicy(IRQ_MOUSE, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id) != OK) return 1;
    mouse_is_subscribed = true;
    return 0;
}

int mouse_unsubscribe_int() {
    if (!mouse_is_subscribed) return 0;
    if (sys_irqrmpolicy(&hook_id) != OK) return 1;
    mouse_is_subscribed = false;
    hook_id = hook_id_default;
    return 0;
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

int mouse_disable_data_reporting() {
    uint8_t resp = 0;

    for (int i = 0; i < KBC_MAX_TRIES; i++) {
      if (kbc_write_to_mouse(MOUSE_DISABLE_DATA) != 0) return 1;
      if (mouse_read_response(&resp) != 0) return 1;

      if (resp == MOUSE_ACK) return 0;
      if (resp == MOUSE_NACK) continue;
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

void mouse_build_packet(const uint8_t bytes[3], struct packet *pp) {
  if (pp == NULL) return;

  pp->bytes[0] = bytes[0];
  pp->bytes[1] = bytes[1];
  pp->bytes[2] = bytes[2];

  pp->lb = bytes[0] & BIT(0);
  pp->rb = bytes[0] & BIT(1);
  pp->mb = bytes[0] & BIT(2);

  pp->x_ov = bytes[0] & BIT(6);
  pp->y_ov = bytes[0] & BIT(7);

  pp->delta_x = (bytes[0] & BIT(4)) ? (int16_t)(0xFF00 | bytes[1]) : (int16_t)bytes[1];
  pp->delta_y = (bytes[0] & BIT(5)) ? (int16_t)(0xFF00 | bytes[2]) : (int16_t)bytes[2];
}
