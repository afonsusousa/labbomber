// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

#include "mouse.h"

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab4/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}


int (mouse_test_packet)(uint32_t cnt) {
  int ipc_status;
  message msg;
  uint8_t mouse_irq_set;

  if (mouse_subscribe_int(&mouse_irq_set) != 0) return 1;
  if (mouse_enable_data_reporting() != 0) {
    mouse_unsubscribe_int();
    return 1;
  }

  uint8_t packet_bytes[3] = {0};
  uint8_t index = 0;
  uint32_t printed_packets = 0;

  while (printed_packets < cnt) {
    if (driver_receive(ANY, &msg, &ipc_status) != 0) continue;

    if (!is_ipc_notify(ipc_status)) continue;
    if (_ENDPOINT_P(msg.m_source) != HARDWARE) continue;

    if (msg.m_notify.interrupts & BIT(mouse_irq_set)) {
      mouse_ih();
      if (mouse_has_error()) continue;

      uint8_t byte = mouse_get_byte();

      if (index == 0 && !(byte & BIT(3))) continue;

      packet_bytes[index++] = byte;

      if (index == 3) {
        struct packet pp;
        mouse_parse_packet(packet_bytes, &pp);
        mouse_print_packet(&pp);
        printed_packets++;
        index = 0;
      }
    }
  }

  if (mouse_disable_data_reporting() != 0) {
    mouse_unsubscribe_int();
    return 1;
  }

  if (mouse_unsubscribe_int() != 0) return 1;
  return 0; 
}

int (mouse_test_async)(uint8_t idle_time) {
    /* To be completed */
    printf("%s(%u): under construction\n", __func__, idle_time);
    return 1;
}
