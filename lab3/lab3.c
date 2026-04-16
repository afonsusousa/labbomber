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

static int kbd_restore() {
  uint8_t cmd_byte;
  uint8_t status;

  // 1. Garantir que o Input Buffer está vazio antes de enviar o comando
  if (util_sys_inb(KBC_STATUS_REG, &status) != 0) return 1;
  if (status & BIT(1)) tickdelay(micros_to_ticks(20000));

  // 2. Enviar comando 0x20 para ler o Command Byte atual
  if (sys_outb(KBC_CMD_REG, 0x20) != 0) return 1;

  // 3. Ler o byte resultante do Output Buffer
  if (util_sys_inb(KBC_OUTBUF_REG, &cmd_byte) != 0) return 1;

  // 4. Modificar o byte: Ativar bit 0 (interrupções) e limpar bit 4 (interface teclado)
  cmd_byte |= BIT(0);
  cmd_byte &= ~BIT(4);

  // 5. Enviar comando 0x60 para avisar que vamos escrever o Command Byte
  if (sys_outb(KBC_CMD_REG, 0x60) != 0) return 1;

  // 6. Escrever o byte modificado para a porta de dados (0x60)
  if (sys_outb(0x60, cmd_byte) != 0) return 1;

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

        if (data == 0xE0) {
          // First byte of a two-byte scancode
          two_bytes = true;
          bytes[0] = data;
          size = 1;
        }
        else {
          if (two_bytes) {
            // Complete the two-byte scancode
            bytes[1] = data;
            size = 2;
            two_bytes = false;
          } else {
            // Single-byte scancode
            bytes[0] = data;
            size = 1;
          }

          bool make = !(bytes[size - 1] & BIT(7));
          kbd_print_scancode(make, size, bytes);

          // Exit when the break code for ESC (0x81) is received
          if (size == 1 && bytes[0] == 0x81)
            done = true;
        }
      }
    }
  }

  // Restore keyboard interrupt policy
  if (kbc_unsubscribe_int() != 0)
    return 1;

  if (kbd_restore() != 0) return 1;

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

    if (data == 0xE0) {
      // Start of a two-byte scancode sequence.
      two_bytes = true;
      bytes[0] = data;
      size = 1;
      continue;
    }

    if (two_bytes) {
      // Complete the multi-byte scancode from the second byte.
      bytes[1] = data;
      size = 2;
      two_bytes = false;
    }
    else {
      // Single-byte scancode received.
      bytes[0] = data;
      size = 1;
    }

    bool make = !(bytes[size - 1] & BIT(7));
    if (kbd_print_scancode(make, size, bytes) != 0) return 1;

    // Exit when the ESC break code is received.
    if (size == 1 && bytes[0] == 0x81) done = true;
  }

  // Restore keyboard to working state (re-enable interrupts + interface)
  if (kbd_restore() != 0) return 1;


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

          // Exit when ESC break code is received.
          if (size == 1 && bytes[0] == 0x81)
            done = true;
        }
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
