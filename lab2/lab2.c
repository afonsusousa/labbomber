#include <lcom/lcf.h>
#include <lcom/lab2.h>

#include <stdbool.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
  lcf_set_language("EN-US");

  lcf_trace_calls("/home/lcom/labs/lab2/trace.txt");
  lcf_log_output("/home/lcom/labs/lab2/output.txt");

  if (lcf_start(argc, argv))
    return 1;

  lcf_cleanup();

  return 0;
}

// Reads the configuration of a timer and displays the requested field
int(timer_test_read_config)(uint8_t timer, enum timer_status_field field) {

  uint8_t st; // stores the timer status byte

  // Get the current configuration of the selected timer
  if (timer_get_conf(timer, &st) != 0) return 1;

  // Interpret and display the requested configuration field
  if (timer_display_conf(timer, st, field) != 0) return 1;

  return 0; // success
}

// Changes the frequency of the selected timer
int(timer_test_time_base)(uint8_t timer, uint32_t freq) {

  // Set the new frequency for the timer
  if (timer_set_frequency(timer, freq) != 0) return 1;

  return 0; // success
}

// Subscribes to timer interrupts and counts them for a given number of seconds
int(timer_test_int)(uint8_t time) {

  int ipc_status;
  message msg;
  int r;

  uint8_t irq_set; // bit mask for timer interrupts

  // Subscribe timer interrupts
  if (timer_subscribe_int(&irq_set) != 0) return 1;

  int counter = 0; // interrupt counter

  // Loop until the required number of timer ticks (time * 60)
  while (counter < time * 60) {

    // Receive messages from the system
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d\n", r);
      continue;
    }

    // Check if the message is a notification
    if (is_ipc_notify(ipc_status)) {
      if (_ENDPOINT_P(msg.m_source) == HARDWARE) {

<<<<<<< Updated upstream
        // Check if the interrupt came from the timer
        if (msg.m_notify.interrupts & BIT(irq_set)) {
          timer_int_handler(); // handle timer interrupt
          counter++;           // increment interrupt count
        }

=======
        case HARDWARE:
          if (msg.m_notify.interrupts & irq_set) {
            timer_int_handler();
            counter++;
          }
          break;

        default:
          break;
>>>>>>> Stashed changes
      }
    }
  }

  // Unsubscribe timer interrupts
  if (timer_unsubscribe_int() != 0) return 1;

  return 0; // success
}
