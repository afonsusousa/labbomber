#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"

int (timer_set_frequency)(uint8_t timer, uint32_t freq) {
  /* To be implemented by the students */
  printf("%s is not yet implemented!\n", __func__);

  return 1;
}

int (timer_subscribe_int)(uint8_t *bit_no) {
    /* To be implemented by the students */
  printf("%s is not yet implemented!\n", __func__);

  return 1;
}

int (timer_unsubscribe_int)() {
  /* To be implemented by the students */
  printf("%s is not yet implemented!\n", __func__);

  return 1;
}

void (timer_int_handler)() {
  /* To be implemented by the students */
  printf("%s is not yet implemented!\n", __func__);
}

int (timer_get_conf)(uint8_t timer, uint8_t *st) {

  if (st == NULL) return 1;

  uint8_t readBackCmd = TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer);

  if (sys_outb(TIMER_CTRL, readBackCmd) != OK) return 1;

  if (util_sys_inb(TIMER_0 + timer, st) != OK) return 1;

  return 0;
}

int (timer_display_conf)(uint8_t timer, uint8_t st,
                         enum timer_status_field field) {

  union timer_status_field_val val;

  if (field == tsf_all) val.byte = st;

  else if (field == tsf_initial) val.in_mode = (st >> 4) & 0x03;

  else if (field == tsf_mode) {
      val.count_mode = (st >> 1) & 0x07;
      if (val.count_mode > 5)
          val.count_mode &= 0x03;
  }

  else if (field == tsf_base) val.bcd = st & 0x01;

  else return 1;

  return timer_print_config(timer, field, val);
}
