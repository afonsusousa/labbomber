#ifndef LCOM_PROJECT_MULTIPLAYER_H
#define LCOM_PROJECT_MULTIPLAYER_H

#include "core/application.h"
#include <stdint.h>
#include <stdbool.h>

void app_multiplayer_assign_roles(t_ctx *ctx);
void app_multiplayer_start_pending_game(t_ctx *ctx);
void app_multiplayer_try_start_game(t_ctx *ctx);
void app_multiplayer_poll_serial(t_ctx *ctx);

int app_multiplayer_send_hello(t_ctx *ctx);
int app_multiplayer_send_name(t_ctx *ctx);
int app_multiplayer_send_ping(t_ctx *ctx);
int app_multiplayer_send_cancel(t_ctx *ctx);
int app_multiplayer_send_key(t_ctx *ctx, uint8_t scancode);
int app_multiplayer_send_start_ready(t_ctx *ctx);
int app_multiplayer_send_start_game(t_ctx *ctx, uint32_t seed);
int app_multiplayer_send_player_state(t_ctx *ctx, uint8_t player_id);
int app_multiplayer_send_pause(t_ctx *ctx, bool paused);

#endif
