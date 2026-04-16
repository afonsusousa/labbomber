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

#include "utils.h"

void kbd_process_scancode(uint8_t data, uint8_t *bytes, uint8_t *size, bool *two_bytes, bool *done) {
  if (data == 0xE0) {
    *two_bytes = true;
    bytes[0] = data; // Store the prefix temporarily if needed (though kbd_print_scancode takes the full array)
    *size = 1;
    return; // Wait for the second byte
  }

  uint16_t scancode = data;
  if (*two_bytes) {
    scancode |= (0xE0 << 8);
    *two_bytes = false;
  }

  if (is_single_byte(scancode)) {
    *size = 1;
    bytes[0] = lsb(scancode);
  } else {
    *size = 2;
    bytes[0] = msb(scancode);
    bytes[1] = lsb(scancode);
  }

  bool make = !(lsb(scancode) & BIT(7));
  kbd_print_scancode(make, *size, bytes);

  if (scancode == 0x81)
    *done = true;
}

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

  // Subscribe to keyboard interrupts and save the notification bit.
  if (kbc_subscribe_int(&irq_set) != 0) return 1;

  uint8_t bytes[2];           // buffer to assemble one or two-byte scancodes
  uint8_t size = 0;          // number of bytes in the current scancode
  bool two_bytes = false;    // whether the next byte is the second half
  bool done = false;         // loop termination flag

  while (!done) {
    if (driver_receive(ANY, &msg, &ipc_status) != 0)
      continue; // ignore failed receives and wait for the next message

    if (is_ipc_notify(ipc_status) && _ENDPOINT_P(msg.m_source) == HARDWARE) {
      if (msg.m_notify.interrupts & BIT(irq_set)) {
        // Keyboard interrupt received
        kbc_ih();

        if (check_kbc_error())
          continue; // skip invalid scancode reads

        uint8_t data = get_current_scancode();

        kbd_process_scancode(data, bytes, &size, &two_bytes, &done);
      }
    }
  }

  // Restore keyboard interrupt policy
  if (kbc_unsubscribe_int() != 0)
    return 1;

  return 0;
}

int(kbd_test_poll)() {
  uint8_t bytes[2] = {0};      // buffer for the current scancode
  uint8_t size = 0;            // current scancode size
  bool two_bytes = false;      // waiting for second scancode byte
  bool done = false;           // loop termination flag

  while (!done) {
    uint8_t status = 0;
    uint8_t data = 0;

    // Read the KBC status register to check if the output buffer has data.
    if (util_sys_inb(KBC_STATUS_REG, &status) != 0) return 1;

    if (!KBC_OBF_FULL(status)) {
      // No data yet, wait a short period and poll again.
      tickdelay(micros_to_ticks(20000));
      continue;
    }

    if (util_sys_inb(KBC_OUTBUF_REG, &data) != 0) return 1;

    // Discard invalid data when there are communication errors or mouse bytes.
    if (ERROR_PARITY(status) || ERROR_TIMEOUT(status) || KBC_AUX_DATA(status)) {
      tickdelay(micros_to_ticks(20000));
      continue;
    }

    kbd_process_scancode(data, bytes, &size, &two_bytes, &done);
  }

  return 0;
}

int(kbd_test_timed_scan)(uint8_t n) {
  int ipc_status;
  message msg;
  uint8_t kbc_irq_set;
  uint8_t timer_irq_set;

  // Subscribe to keyboard interrupts.
  if (kbc_subscribe_int(&kbc_irq_set) != 0) return 1;

  // Subscribe to timer interrupts.
  if (timer_subscribe_int(&timer_irq_set) != 0) {
    kbc_unsubscribe_int();
    return 1;
  }

  uint8_t bytes[2];
  uint8_t size = 0;
  bool two_bytes = false;
  bool done = false;
  timer_counter = 0; // reset the elapsed time counter

  while (!done) {
    if (driver_receive(ANY, &msg, &ipc_status) != 0)
      continue; // ignore failed receives

    if (is_ipc_notify(ipc_status) && _ENDPOINT_P(msg.m_source) == HARDWARE) {
      if (msg.m_notify.interrupts & BIT(timer_irq_set)) {
        // Timer interrupt: update the shared timer counter.
        timer_int_handler();

        if (timer_counter >= n * 60)
          done = true; // timeout reached
      }

      if (msg.m_notify.interrupts & BIT(kbc_irq_set)) {
        // Keyboard interrupt: read scancode as in kbd_test_scan.
        kbc_ih();

        if (check_kbc_error())
          continue;

        timer_counter = 0; // reset timer on any keyboard activity

        uint8_t data = get_current_scancode();

        kbd_process_scancode(data, bytes, &size, &two_bytes, &done);
      }
    }
  }

  // Unsubscribe from timer interrupts first to avoid leaving timer notifications active.
  if (timer_unsubscribe_int() != 0) {
    kbc_unsubscribe_int();
    return 1;
  }

  // Unsubscribe from keyboard interrupts.
  if (kbc_unsubscribe_int() != 0) return 1;

  return 0;
}
