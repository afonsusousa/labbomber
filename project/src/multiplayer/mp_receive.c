#include "multiplayer/multiplayer.h"
#include "mp_protocol.h"
#include "core/application.h"
#include "gui/gui.h"
#include "gui/widget.h"
#include "game/game.h"
#include "serial_port.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// packet handler

static void mp_process_packet(t_ctx *ctx) {
    if (ctx == NULL) return;

    if (ctx->is_multiplayer)
        ctx->game.multiplayer_last_contact_ticks = ctx->game.logical_ticks;

    switch (ctx->multiplayer_rx_type) {

        case MP_PACKET_HELLO:
            ctx->multiplayer_remote_nonce =
                (uint16_t)ctx->multiplayer_rx_data[0] |
                ((uint16_t)ctx->multiplayer_rx_data[1] << 8);
            ctx->multiplayer_remote_tiebreaker = ctx->multiplayer_rx_data[2];
            app_multiplayer_assign_roles(ctx);
            break;

        case MP_PACKET_PING:
            break;

        case MP_PACKET_KEY: {
            uint8_t player_id = ctx->multiplayer_rx_data[0];
            uint8_t scancode  = ctx->multiplayer_rx_data[1];

            FILE *f = fopen("/tmp/game_debug.log", "a");
            if (f) {
                fprintf(f, "[MP] rx key player=%u scancode=0x%02X\n", player_id, scancode);
                fclose(f);
            }

            if (ctx->multiplayer_role_assigned && player_id != ctx->multiplayer_local_player)
                game_state_handle_player_key(&ctx->game, player_id, scancode);
            break;
        }

        case MP_PACKET_PLAYER_STATE: {
            uint8_t player_id = ctx->multiplayer_rx_data[0];
            uint8_t lives_active = ctx->multiplayer_rx_data[1];
            uint8_t powerups = ctx->multiplayer_rx_data[2];

            uint8_t lives = lives_active & 0x7F;
            bool active = (lives_active & 0x80) != 0;

            if (ctx->multiplayer_role_assigned &&
                player_id != ctx->multiplayer_local_player &&
                player_id < MAX_PLAYERS) {
                player_t *player = &ctx->game.players[player_id];
                player->lives = lives;
                player->active = active;
                player->powerups = powerups;
            }
            break;
        }

        case MP_PACKET_NAME_PART: {
            uint8_t offset = ctx->multiplayer_rx_data[0];
            char c1 = (char)ctx->multiplayer_rx_data[1];
            char c2 = (char)ctx->multiplayer_rx_data[2];

            if (offset < 31) {
                ctx->multiplayer_remote_name[offset] = c1;

                if (offset + 1 < 31) ctx->multiplayer_remote_name[offset + 1] = c2;
            }

            if (c1 == '\0' || c2 == '\0') {
                ctx->multiplayer_name_received = true;

                mp_log(ctx, "remote name received");
                app_multiplayer_try_start_game(ctx);
            }
            break;
        }

        case MP_PACKET_CANCEL:
            mp_log(ctx, "peer cancelled or quit");

            if (ctx->multiplayer_game_started) {
                ctx->multiplayer_game_started = false;
                ctx->is_multiplayer = false;

                gui_pop_until_widget_found(&ctx->gui, "start_menu_view");
                break;
            }

            ctx->multiplayer_remote_start_ready = false;
            ctx->multiplayer_name_received = false;
            break;

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
            mp_log(ctx, "remote start ready");

            if (!was_remote_ready && ctx->multiplayer_local_start_ready) {
                app_multiplayer_send_start_ready(ctx);
                mp_log(ctx, "start ready ack sent");
            }

            app_multiplayer_try_start_game(ctx);
            break;
        }

        case MP_PACKET_START_GAME: {
            uint32_t seed =
                (uint32_t)ctx->multiplayer_rx_data[0] |
                ((uint32_t)ctx->multiplayer_rx_data[1] << 8) |
                ((uint32_t)ctx->multiplayer_rx_data[2] << 16);

            ctx->multiplayer_remote_start_ready = true;

            if (!ctx->multiplayer_start_game_sent) {
                ctx->multiplayer_start_game_sent = true;
                app_multiplayer_send_start_game(ctx, seed);
            }

            mp_log(ctx, "start game received");
            ctx->multiplayer_match_seed = seed & 0x00FFFFFF;
            ctx->game.enemy_seed = ctx->multiplayer_match_seed;
            ctx->multiplayer_start_game_pending = true;
            break;
        }

        default:
            break;
    }
}

// byte-level state machine

static void mp_receive_byte(t_ctx *ctx, uint8_t byte) {
    if (ctx == NULL) return;

    if (ctx->is_multiplayer) ctx->game.multiplayer_last_contact_ticks = ctx->game.logical_ticks;

    if (ctx->multiplayer_rx_state == 0 && byte == 0xAA) {
        mp_log(ctx, "legacy hello byte seen");
        return;
    }

    switch (ctx->multiplayer_rx_state) {
        case 0:
            if (byte == MP_PACKET_START) {
                ctx->multiplayer_rx_state = 1;
                ctx->multiplayer_rx_pos   = 0;
            }
            break;

        case 1:
            ctx->multiplayer_rx_type  = byte;
            ctx->multiplayer_rx_state = 2;
            break;

        case 2:
            ctx->multiplayer_rx_data[ctx->multiplayer_rx_pos++] = byte;

            if (ctx->multiplayer_rx_pos >= MP_PACKET_PAYLOAD_SIZE) {
                mp_process_packet(ctx);

                ctx->multiplayer_rx_state = 0;
                ctx->multiplayer_rx_pos   = 0;
            }
            break;

        default:
            ctx->multiplayer_rx_state = 0;
            ctx->multiplayer_rx_pos = 0;

            break;
    }
}

// public poll function

void app_multiplayer_poll_serial(t_ctx *ctx) {
    if (ctx == NULL) return;

    for (int i = 0; i < 32 && serial_has_byte(); i++) {
        uint8_t received;
        if (serial_read_byte(&received) == 0 && ctx->is_multiplayer) mp_receive_byte(ctx, received);
    }
}
