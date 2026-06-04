#include "multiplayer.h"
#include "gui.h"
#include "widget.h"
#include "game.h"
#include "../lib/serialPort/serial_port.h"
#include "../lib/keyboard/i8042.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define MP_PACKET_START        0xA5
#define MP_PACKET_HELLO        0x01
#define MP_PACKET_KEY          0x02
#define MP_PACKET_PLAYER_STATE 0x03
#define MP_PACKET_PAUSE        0x04
#define MP_PACKET_READY        0x05
#define MP_PACKET_START_GAME   0x06
#define MP_PACKET_NAME_PART    0x07
#define MP_PACKET_PAYLOAD_SIZE 3

static int app_multiplayer_send_packet(uint8_t type, uint8_t data0, uint8_t data1, uint8_t data2) {
    if (serial_send_byte(MP_PACKET_START) != 0) return 1;
    if (serial_send_byte(type) != 0) return 1;
    if (serial_send_byte(data0) != 0) return 1;
    if (serial_send_byte(data1) != 0) return 1;
    return serial_send_byte(data2);
}

static void app_multiplayer_log(t_ctx *ctx, const char *message) {
    FILE *log_file = fopen("/tmp/game_debug.log", "a");
    if (log_file == NULL) return;

    fprintf(log_file,
            "[MP] %s local=%u remote=%u ready=%d role=%d nonce=%u remote_nonce=%u\n",
            message,
            ctx != NULL ? ctx->multiplayer_local_player : 255,
            ctx != NULL ? ctx->multiplayer_remote_player : 255,
            ctx != NULL && ctx->multiplayer_partner_ready,
            ctx != NULL && ctx->multiplayer_role_assigned,
            ctx != NULL ? ctx->multiplayer_local_nonce : 0,
            ctx != NULL ? ctx->multiplayer_remote_nonce : 0);
    fclose(log_file);
}

static void app_multiplayer_assign_roles(t_ctx *ctx) {
    if (ctx == NULL) return;

    if (ctx->multiplayer_local_nonce > ctx->multiplayer_remote_nonce ||
        (ctx->multiplayer_local_nonce == ctx->multiplayer_remote_nonce &&
         ctx->multiplayer_local_tiebreaker >= ctx->multiplayer_remote_tiebreaker)) {
        ctx->multiplayer_local_player = PLAYER_1;
        ctx->multiplayer_remote_player = PLAYER_2;
    } else {
        ctx->multiplayer_local_player = PLAYER_2;
        ctx->multiplayer_remote_player = PLAYER_1;
    }

    ctx->game.current_player = ctx->multiplayer_local_player;
    ctx->multiplayer_role_assigned = true;
    ctx->multiplayer_partner_ready = true;
    app_multiplayer_log(ctx, "roles assigned (RTC selection fallback ready)");

    t_widget *wait_conn = widget_find_by_name(&ctx->gui, "wait_conn_overlay");
    if (wait_conn != NULL) {
        gui_pop_view(&ctx->gui);
        gui_show_name_menu(ctx, true);
    }
}

static uint32_t app_make_match_seed(t_ctx *ctx) {
    uint32_t a = ctx->multiplayer_local_nonce;
    uint32_t b = ctx->multiplayer_remote_nonce;

    uint32_t low = a < b ? a : b;
    uint32_t high = a < b ? b : a;

    uint32_t seed = low * 1103515245u + high * 12345u + 0xB00B5u;

    seed &= 0x00FFFFFF;

    if (seed == 0) seed = 1;

    return seed;
}

static void app_multiplayer_queue_start_game(t_ctx *ctx, uint32_t seed) {
    if (ctx == NULL || ctx->multiplayer_game_started) return;

    seed &= 0x00FFFFFF;

    ctx->multiplayer_match_seed = seed;
    ctx->game.enemy_seed = seed;
    ctx->multiplayer_start_game_pending = true;

    app_multiplayer_log(ctx, "start game queued");
}

void app_multiplayer_start_pending_game(t_ctx *ctx) {
    if (ctx == NULL) return;
    if (!ctx->multiplayer_start_game_pending || ctx->multiplayer_game_started) return;
    if (widget_find_by_name(&ctx->gui, "game_view") != NULL) return;

    ctx->multiplayer_start_game_pending = false;
    ctx->multiplayer_game_started = true;

    t_widget *top = gui_get_top_view(&ctx->gui);

    if (top != NULL && top->name != NULL && strcmp(top->name, "info_overlay") == 0) {
        gui_pop_view(&ctx->gui);
    }

    gui_show_game_view(ctx);
}

void app_multiplayer_try_start_game(t_ctx *ctx) {
    if (ctx == NULL || !ctx->is_multiplayer) return;
    if (ctx->multiplayer_game_started || ctx->multiplayer_start_game_pending) return;
    if (!ctx->multiplayer_name_received || !ctx->multiplayer_remote_start_ready || !ctx->multiplayer_local_start_ready) return;

    uint32_t seed = app_make_match_seed(ctx);

    ctx->multiplayer_start_game_sent = true;

    app_multiplayer_send_start_game(ctx, seed);
    app_multiplayer_queue_start_game(ctx, seed);
}

static void app_multiplayer_process_packet(t_ctx *ctx) {
    if (ctx == NULL) return;

    switch (ctx->multiplayer_rx_type) {
        case MP_PACKET_HELLO:
            ctx->multiplayer_remote_nonce =
                (uint16_t)ctx->multiplayer_rx_data[0] |
                ((uint16_t)ctx->multiplayer_rx_data[1] << 8);
            ctx->multiplayer_remote_tiebreaker = ctx->multiplayer_rx_data[2];
            app_multiplayer_assign_roles(ctx);
            break;

        case MP_PACKET_KEY: {
            uint8_t player_id = ctx->multiplayer_rx_data[0];
            uint8_t scancode = ctx->multiplayer_rx_data[1];

            FILE *log_file = fopen("/tmp/game_debug.log", "a");
            if (log_file) {
                fprintf(log_file, "[MP] rx key player=%u scancode=0x%02X\n", player_id, scancode);
                fclose(log_file);
            }

            if (ctx->multiplayer_role_assigned && player_id != ctx->multiplayer_local_player) {
                game_state_handle_player_key(&ctx->game, player_id, scancode);
            }

            break;
        }

        case MP_PACKET_PLAYER_STATE: {
            uint8_t player_id = ctx->multiplayer_rx_data[0];
            uint8_t state_value = ctx->multiplayer_rx_data[1];
            uint8_t lives = state_value & 0x7F;
            bool active = (state_value & 0x80) != 0;

            FILE *log_file = fopen("/tmp/game_debug.log", "a");
            if (log_file) {
                fprintf(log_file, "[MP] rx state player=%u lives=%u active=%u\n", player_id, lives, active);
                fclose(log_file);
            }

            if (ctx->multiplayer_role_assigned && player_id != ctx->multiplayer_local_player && player_id < MAX_PLAYERS) {
                player_t *player = &ctx->game.players[player_id];
                player->lives = lives;
                player->active = active;
            }

            break;
        }

        case MP_PACKET_NAME_PART: {
            uint8_t offset = ctx->multiplayer_rx_data[0];
            char c1 = (char)ctx->multiplayer_rx_data[1];
            char c2 = (char)ctx->multiplayer_rx_data[2];

            if (offset < 31) {
                ctx->multiplayer_remote_name[offset] = c1;
                if (offset + 1 < 31) {
                    ctx->multiplayer_remote_name[offset + 1] = c2;
                }
            }
            if (c1 == '\0' || c2 == '\0') {
                ctx->multiplayer_name_received = true;
                app_multiplayer_log(ctx, "remote name received");
                app_multiplayer_try_start_game(ctx);
            }
            break;
        }

        case MP_PACKET_PAUSE: {
            bool paused = ctx->multiplayer_rx_data[0] != 0;

            if (ctx->game.match_state == MATCH_RUNNING || ctx->game.match_state == MATCH_PAUSED) {
                ctx->game.is_frozen = paused;
                ctx->game.match_state = paused ? MATCH_PAUSED : MATCH_RUNNING;
            }

            break;
        }

        case MP_PACKET_READY: {
            bool was_remote_ready = ctx->multiplayer_remote_start_ready;

            ctx->multiplayer_remote_start_ready = true;
            app_multiplayer_log(ctx, "remote start ready");

            if (!was_remote_ready && ctx->multiplayer_local_start_ready) {
                app_multiplayer_send_start_ready(ctx);
                app_multiplayer_log(ctx, "start ready ack sent");
            }

            app_multiplayer_try_start_game(ctx);

            break;
        }

        case MP_PACKET_START_GAME: {
            uint32_t seed = (uint32_t)ctx->multiplayer_rx_data[0] |
                            ((uint32_t)ctx->multiplayer_rx_data[1] << 8) |
                            ((uint32_t)ctx->multiplayer_rx_data[2] << 16);

            ctx->multiplayer_remote_start_ready = true;

            if (!ctx->multiplayer_start_game_sent) {
                ctx->multiplayer_start_game_sent = true;
                app_multiplayer_send_start_game(ctx, seed);
            }

            app_multiplayer_log(ctx, "start game received");
            app_multiplayer_queue_start_game(ctx, seed);
            break;
        }

        default:
            break;
    }
}

static void app_multiplayer_receive_byte(t_ctx *ctx, uint8_t byte) {
    if (ctx == NULL) return;

    if (ctx->multiplayer_rx_state == 0 && byte == 0xAA) {
        app_multiplayer_log(ctx, "legacy hello byte seen");
        return;
    }

    switch (ctx->multiplayer_rx_state) {
        case 0:
            if (byte == MP_PACKET_START) {
                ctx->multiplayer_rx_state = 1;
                ctx->multiplayer_rx_pos = 0;
            }
            break;

        case 1:
            ctx->multiplayer_rx_type = byte;
            ctx->multiplayer_rx_state = 2;
            break;

        case 2:
            ctx->multiplayer_rx_data[ctx->multiplayer_rx_pos++] = byte;

            if (ctx->multiplayer_rx_pos >= MP_PACKET_PAYLOAD_SIZE) {
                app_multiplayer_process_packet(ctx);
                ctx->multiplayer_rx_state = 0;
                ctx->multiplayer_rx_pos = 0;
            }

            break;

        default:
            ctx->multiplayer_rx_state = 0;
            ctx->multiplayer_rx_pos = 0;
            break;
    }
}

int app_multiplayer_send_hello(t_ctx *ctx) {
    if (ctx == NULL) return 1;

    int result = app_multiplayer_send_packet(
        MP_PACKET_HELLO,
        (uint8_t)(ctx->multiplayer_local_nonce & 0xFF),
        (uint8_t)(ctx->multiplayer_local_nonce >> 8),
        ctx->multiplayer_local_tiebreaker
    );

    serial_send_byte(0xAA);
    app_multiplayer_log(ctx, result == 0 ? "hello sent" : "hello send failed");

    return result;
}

int app_multiplayer_send_start_ready(t_ctx *ctx) {
    if (ctx == NULL || !ctx->is_multiplayer) return 1;

    int result = app_multiplayer_send_packet(MP_PACKET_READY, 0, 0, 0);
    app_multiplayer_log(ctx, result == 0 ? "start ready sent" : "start ready send failed");

    return result;
}

int app_multiplayer_send_start_game(t_ctx *ctx, uint32_t seed) {
    if (ctx == NULL || !ctx->is_multiplayer) return 1;

    seed &= 0x00FFFFFF;

    int result = app_multiplayer_send_packet(
        MP_PACKET_START_GAME,
        (uint8_t)(seed & 0xFF),
        (uint8_t)((seed >> 8) & 0xFF),
        (uint8_t)((seed >> 16) & 0xFF)
    );

    app_multiplayer_log(ctx, result == 0 ? "start game sent" : "start game send failed");

    return result;
}

int app_multiplayer_send_name(t_ctx *ctx) {
    if (ctx == NULL || !ctx->is_multiplayer) return 1;

    size_t len = strlen(ctx->multiplayer_local_name);
    for (size_t i = 0; i <= len; i += 2) {
        app_multiplayer_send_packet(
            MP_PACKET_NAME_PART,
            (uint8_t)i,
            (uint8_t)ctx->multiplayer_local_name[i],
            (uint8_t)(i + 1 <= len ? ctx->multiplayer_local_name[i + 1] : 0)
        );
    }
    ctx->multiplayer_name_sent = true;
    app_multiplayer_log(ctx, "local name sent");
    return 0;
}

int app_multiplayer_send_key(t_ctx *ctx, uint8_t scancode) {
    if (ctx == NULL || !ctx->is_multiplayer || !ctx->multiplayer_role_assigned) return 1;

    uint8_t key_index = MAKE_FROM_BREAK(scancode);

    if (key_index != KEY_W && key_index != KEY_A && key_index != KEY_D &&
        key_index != KEY_S && key_index != KEY_E) {
        return 0;
    }

    int result = app_multiplayer_send_packet(MP_PACKET_KEY, ctx->multiplayer_local_player, scancode, 0);

    FILE *log_file = fopen("/tmp/game_debug.log", "a");
    if (log_file) {
        fprintf(log_file,
                "[MP] tx key player=%u scancode=0x%02X result=%d\n",
                ctx->multiplayer_local_player,
                scancode,
                result);
        fclose(log_file);
    }

    return result;
}

void app_multiplayer_poll_serial(t_ctx *ctx) {
    if (ctx == NULL) return;

    for (int i = 0; i < 32 && serial_has_byte(); i++) {
        uint8_t received;

        if (serial_read_byte(&received) == 0) {
            if (ctx->is_multiplayer) {
                app_multiplayer_receive_byte(ctx, received);
            }
        }
    }
}

int app_multiplayer_send_player_state(t_ctx *ctx, uint8_t player_id) {
    if (ctx == NULL || !ctx->is_multiplayer || !ctx->multiplayer_role_assigned) return 1;
    if (player_id >= MAX_PLAYERS) return 1;

    player_t *player = &ctx->game.players[player_id];

    uint8_t state_value = player->lives & 0x7F;
    if (player->active) state_value |= 0x80;

    int result = app_multiplayer_send_packet(MP_PACKET_PLAYER_STATE, player_id, state_value, 0);

    FILE *log_file = fopen("/tmp/game_debug.log", "a");
    if (log_file) {
        fprintf(log_file,
                "[MP] tx state player=%u lives=%u active=%u result=%d\n",
                player_id,
                player->lives,
                player->active,
                result);
        fclose(log_file);
    }

    return result;
}

int app_multiplayer_send_pause(t_ctx *ctx, bool paused) {
    if (ctx == NULL || !ctx->is_multiplayer || !ctx->multiplayer_role_assigned) return 1;

    int result = app_multiplayer_send_packet(MP_PACKET_PAUSE, paused ? 1 : 0, 0, 0);

    FILE *log_file = fopen("/tmp/game_debug.log", "a");
    if (log_file) {
        fprintf(log_file, "[MP] tx pause paused=%u result=%d\n", paused, result);
        fclose(log_file);
    }

    return result;
}
