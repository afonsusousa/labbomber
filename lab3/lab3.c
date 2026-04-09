#include <lcom/lcf.h>

#include <lcom/lab3.h>
#include "kbc.h"

#include <stdbool.h>
#include <stdint.h>

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
  /* To be completed by the students */
  printf("%s is not yet implemented!\n", __func__);

  return 1;
}

int(kbd_test_timed_scan)(uint8_t n) {
  /* To be completed by the students */
  printf("%s is not yet implemented!\n", __func__);

  return 1;
}
