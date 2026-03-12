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

// Reads the status byte of the selected timer
int (timer_get_conf)(uint8_t timer, uint8_t *st) {

  if (st == NULL) return 1; // check if pointer is valid

  // Build the read-back command for the selected timer
  uint8_t readBackCmd = TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer);

  // Send command to the timer control register
  if (sys_outb(TIMER_CTRL, readBackCmd) != OK) return 1;

  // Read the status byte from the timer port
  if (util_sys_inb(TIMER_0 + timer, st) != OK) return 1;

  return 0;
}

// Interprets the timer status byte and prints the requested field
int (timer_display_conf)(uint8_t timer, uint8_t st,
                         enum timer_status_field field) {

  union timer_status_field_val val; // union used to store the decoded field

  if (field == tsf_all) val.byte = st; // show full status byte

  else if (field == tsf_initial) val.in_mode = (st >> 4) & 0x03; // extract initialization mode

  else if (field == tsf_mode) {
      val.count_mode = (st >> 1) & 0x07; // extract counting mode
      if (val.count_mode > 5) val.count_mode &= 0x03; // adjust modes 6 and 7
  }

  else if (field == tsf_base) val.bcd = st & 0x01; // extract BCD/binary mode

  else return 1;

  return timer_print_config(timer, field, val); // print the interpreted configuration
}
