#include <lcom/lcf.h>
#include <lcom/lab2.h>

#include <stdbool.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
  lcf_set_language("EN-US"); // definir linguagem

  lcf_trace_calls("/home/lcom/labs/lab2/trace.txt"); // trace de chamadas
  lcf_log_output("/home/lcom/labs/lab2/output.txt"); // log de output

  if (lcf_start(argc, argv)) // inicia framework LCF
    return 1;

  lcf_cleanup(); // limpeza final

  return 0;
}

// Reads the configuration of a timer and displays the requested field
int(timer_test_read_config)(uint8_t timer, enum timer_status_field field) {
  uint8_t st;
  
  // Get the current configuration of the selected timer
  if (timer_get_conf(timer, &st) != 0) // ler config
    return 1;

  // Interpret and display the requested configuration field
  if (timer_display_conf(timer, st, field) != 0) // mostrar campo
    return 1;

  return 0;
}

// Changes the frequency of the selected timer
int(timer_test_time_base)(uint8_t timer, uint32_t freq) {
  // Set the new frequency for the timer
  if (timer_set_frequency(timer, freq) != 0) // alterar frequência
    return 1;

  return 0;
}

extern uint32_t timer_counter;

int(timer_test_int)(uint8_t time) {
  int ipc_status;
  message msg;
  uint8_t irq_set; // bit do timer

  // Subscribe timer interrupts
  if (timer_subscribe_int(&irq_set) != 0) // ativar interrupções
    return 1;

  timer_counter = 0; // reset contador

  // Loop até atingir tempo desejado (time segundos ~ 60 ticks/s)
  while (timer_counter < time * 60) {

    if (driver_receive(ANY, &msg, &ipc_status) != 0) { // receber mensagem
      printf("driver_receive failed\n");
      continue;
    }

    if (is_ipc_notify(ipc_status) && _ENDPOINT_P(msg.m_source) == HARDWARE) { // notificação HW
      
      if (msg.m_notify.interrupts & BIT(irq_set)) { // veio do timer
        
        timer_int_handler(); // tratar interrupção
        
        if (timer_counter % 60 == 0) { // a cada 1 segundo
          timer_print_elapsed_time(); // imprimir tempo
        }
      }
    }
  }

  // Unsubscribe timer interrupts
  if (timer_unsubscribe_int() != 0) // desativar interrupções
    return 1;

  return 0;
}