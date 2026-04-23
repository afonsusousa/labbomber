// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>
#include <lcom/lab4.h>
#include <lcom/timer.h>
#include "i8254.h"

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "mouse.h"

static uint32_t timer_interrupts = 0;

void (timer_int_handler)() {
  timer_interrupts++;
}

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
  bool mouse_subscribed = false;
  bool data_reporting_enabled = false;
  int ret = 0;

  if (mouse_cmd_enable_data_reporting() != 0) return 1;
  data_reporting_enabled = true;

  if (mouse_subscribe_int(&mouse_irq_set) != 0) {
    ret = 1;
    goto cleanup;
  }
  mouse_subscribed = true;


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

      struct packet pp;
      if (mouse_sync_bytes(mouse_get_byte(), packet_bytes, &index, &pp)) {
        mouse_print_packet(&pp);
        printed_packets++;
      }
    }
  }

cleanup:
  if (mouse_subscribed && mouse_unsubscribe_int() != 0) ret = 1;
  if (data_reporting_enabled && mouse_disable_data_reporting() != 0) ret = 1;

  return ret;
}

int (mouse_test_async)(uint8_t idle_time) {
  int ipc_status;
  message msg;
  uint8_t mouse_irq_set;
  uint8_t timer_irq_set = 0;
  int timer_hook_id = 0;

  bool mouse_subscribed = false;
  bool timer_subscribed = false;
  bool data_reporting_enabled = false;
  int ret = 0;

  uint32_t hz = sys_hz();
  if (hz == 0) return 1;

  if (mouse_cmd_enable_data_reporting() != 0) {
    ret = 1;
    goto cleanup;
  }
  data_reporting_enabled = true;

  if (mouse_subscribe_int(&mouse_irq_set) != 0) {
    ret = 1;
    goto cleanup;
  }
  mouse_subscribed = true;

  timer_irq_set = (uint8_t) timer_hook_id;
  if (sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &timer_hook_id) != OK) {
    ret = 1;
    goto cleanup;
  }
  timer_subscribed = true;

  uint8_t packet_bytes[3] = {0};
  uint8_t index = 0;
  uint32_t idle_ticks = 0;
  uint32_t max_idle_ticks = (uint32_t) idle_time * hz;

  while (idle_ticks < max_idle_ticks) {
    if (driver_receive(ANY, &msg, &ipc_status) != 0) continue;
    if (!is_ipc_notify(ipc_status)) continue;
    if (_ENDPOINT_P(msg.m_source) != HARDWARE) continue;

    if (msg.m_notify.interrupts & BIT(mouse_irq_set)) {
      mouse_ih();
      if (mouse_has_error()) continue;

      struct packet pp;
      if (mouse_sync_bytes(mouse_get_byte(), packet_bytes, &index, &pp)) {
        mouse_print_packet(&pp);
        idle_ticks = 0;
      }
    }

    if (msg.m_notify.interrupts & BIT(timer_irq_set)) {
      timer_int_handler();
      idle_ticks++;
    }
  }

cleanup:
  if (timer_subscribed && sys_irqrmpolicy(&timer_hook_id) != OK) ret = 1;
  if (mouse_subscribed && mouse_unsubscribe_int() != 0) ret = 1;
  if (data_reporting_enabled && mouse_disable_data_reporting() != 0) ret = 1;

  return ret;
}
