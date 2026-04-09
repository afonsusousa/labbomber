#include <lcom/lcf.h>

#include <lcom/lab3.h>
#include <lcom/timer.h>
#include "kbc.h"

#include <stdbool.h>
#include <stdint.h>

int (timer_subscribe_int)(uint8_t *bit_no);
int (timer_unsubscribe_int)();
void (timer_int_handler)();

extern uint32_t timer_counter;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(kbd_test_scan)() {
  int ipc_status;
  message msg;
  uint8_t irq_set;

  if (kbc_subscribe_int(&irq_set) != 0) return 1;

  uint8_t bytes[2];
  uint8_t size = 0;
  bool two_bytes = false;
  bool done = false;

  while (!done) {
    if (driver_receive(ANY, &msg, &ipc_status) != 0)
      continue;

    if (is_ipc_notify(ipc_status) && _ENDPOINT_P(msg.m_source) == HARDWARE) {
      if (msg.m_notify.interrupts & BIT(irq_set)) {
        kbc_ih();

        if (check_kbc_error()) 
          continue;

        uint8_t data = get_current_scancode();

        if (data == 0xE0) {
          two_bytes = true;
          bytes[0] = data;
          size = 1;
        }

        else {
          if (two_bytes) {
            bytes[1] = data;
            size = 2;
            two_bytes = false;
          } else {
            bytes[0] = data;
            size = 1;
          }

          bool make = !(bytes[size - 1] & BIT(7));
          kbd_print_scancode(make, size, bytes);

          if (size == 1 && bytes[0] == 0x81)
            done = true;
        }
      }
    }
  }

  if (kbc_unsubscribe_int() != 0)
    return 1;

  return 0;
}

int(kbd_test_poll)() {
  uint8_t bytes[2] = {0};
  uint8_t size = 0;
  bool two_bytes = false;
  bool done = false;

  while (!done) {
    uint8_t status = 0;
    uint8_t data = 0;

    if (util_sys_inb(KBC_STATUS_REG, &status) != 0) return 1;

    if (!KBC_OBF_FULL(status)) {
      tickdelay(micros_to_ticks(20000));
      continue;
    }

    if (util_sys_inb(KBC_OUTBUF_REG, &data) != 0) return 1;

    if (ERROR_PARITY(status) || ERROR_TIMEOUT(status) || KBC_AUX_DATA(status)) {
      tickdelay(micros_to_ticks(20000));
      continue;
    }

    if (data == 0xE0) {
      two_bytes = true;
      bytes[0] = data;
      size = 1;
      continue;
    }

    if (two_bytes) {
      bytes[1] = data;
      size = 2;
      two_bytes = false;
    }
    else {
      bytes[0] = data;
      size = 1;
    }

    bool make = !(bytes[size - 1] & BIT(7));
    if (kbd_print_scancode(make, size, bytes) != 0) return 1;

    if (size == 1 && bytes[0] == 0x81) done = true;
  }

  return 0;
}

int(kbd_test_timed_scan)(uint8_t n) {
  int ipc_status;
  message msg;
  uint8_t kbc_irq_set;
  uint8_t timer_irq_set;

  if (kbc_subscribe_int(&kbc_irq_set) != 0) return 1;

  if (timer_subscribe_int(&timer_irq_set) != 0) {
    kbc_unsubscribe_int();
    return 1;
  }

  uint8_t bytes[2];
  uint8_t size = 0;
  bool two_bytes = false;
  bool done = false;
  timer_counter = 0;

  while (!done) {
    if (driver_receive(ANY, &msg, &ipc_status) != 0)
      continue;

    if (is_ipc_notify(ipc_status) && _ENDPOINT_P(msg.m_source) == HARDWARE) {
      if (msg.m_notify.interrupts & BIT(timer_irq_set)) {
        timer_int_handler();

        if (timer_counter >= n * 60) done = true;
      }

      if (msg.m_notify.interrupts & BIT(kbc_irq_set)) {
        kbc_ih();

        if (check_kbc_error())
          continue;

        timer_counter = 0;

        uint8_t data = get_current_scancode();

        if (data == 0xE0) {
          two_bytes = true;
          bytes[0] = data;
          size = 1;
        }
        else {
          if (two_bytes) {
            bytes[1] = data;
            size = 2;
            two_bytes = false;
          }
          else {
            bytes[0] = data;
            size = 1;
          }

          bool make = !(bytes[size - 1] & BIT(7));
          kbd_print_scancode(make, size, bytes);

          if (size == 1 && bytes[0] == 0x81)
            done = true;
        }
      }
    }
  }

  if (timer_unsubscribe_int() != 0) {
    kbc_unsubscribe_int();
    return 1;
  }

  if (kbc_unsubscribe_int() != 0) return 1;

  return 0;
}
