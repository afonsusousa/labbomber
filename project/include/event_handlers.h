#ifndef LCOM_PROJECT_EVENT_HANDLERS_H
#define LCOM_PROJECT_EVENT_HANDLERS_H

#include "hardware.h"
#include "application.h"

void handle_timer(hardware_t *hw_state, t_ctx *ctx);
void handle_keyboard(hardware_t *hw_state, t_ctx *ctx, bool *esc_was_pressed);
void handle_mouse(hardware_t *hw_state, t_ctx *ctx);

#endif /* LCOM_PROJECT_EVENT_HANDLERS_H */
