#ifndef LCOM_PROJECT_EVENT_HANDLERS_H
#define LCOM_PROJECT_EVENT_HANDLERS_H

#include "hardware.h"
#include "gui.h"

void handle_timer(hardware_t *hw_state, t_gui *gui);
void handle_keyboard(hardware_t *hw_state, t_gui *gui, bool *esc_was_pressed);
void handle_mouse(hardware_t *hw_state, t_gui *gui);

#endif /* LCOM_PROJECT_EVENT_HANDLERS_H */
