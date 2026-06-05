#include "multiplayer/multiplayer.h"
#include "mp_protocol.h"
#include "core/application.h"
#include "core/macros.h"
#include "game/game.h"
#include "serial_port.h"
#include "i8042.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

int app_multiplayer_send_hello(t_ctx *ctx) {
    if (ctx == NULL) return 1;

    int result = mp_send_packet(
        MP_PACKET_HELLO,
        (uint8_t)(ctx->multiplayer_local_nonce & 0xFF),
        (uint8_t)(ctx->multiplayer_local_nonce >> 8),
        ctx->multiplayer_local_tiebreaker
    );

    serial_send_byte(0xAA);
    mp_log(ctx, result == 0 ? "hello sent" : "hello send failed");
    return result;
}

int app_multiplayer_send_start_ready(t_ctx *ctx) {
    if (ctx == NULL || !ctx->is_multiplayer) return 1;

    int result = mp_send_packet(MP_PACKET_READY, 0, 0, 0);
    mp_log(ctx, result == 0 ? "start ready sent" : "start ready send failed");
    return result;
}

int app_multiplayer_send_start_game(t_ctx *ctx, uint32_t seed) {
    if (ctx == NULL || !ctx->is_multiplayer) return 1;

    seed &= 0x00FFFFFF;

    int result = mp_send_packet(
        MP_PACKET_START_GAME,
        (uint8_t)(seed & 0xFF),
        (uint8_t)((seed >> 8)  & 0xFF),
        (uint8_t)((seed >> 16) & 0xFF)
    );

    mp_log(ctx, result == 0 ? "start game sent" : "start game send failed");
    return result;
}

int app_multiplayer_send_name(t_ctx *ctx) {
    if (ctx == NULL || !ctx->is_multiplayer) return 1;

    size_t len = strlen(ctx->multiplayer_local_name);
    for (size_t i = 0; i <= len; i += 2) {
        mp_send_packet(
            MP_PACKET_NAME_PART,
            (uint8_t)i,
            (uint8_t)ctx->multiplayer_local_name[i],
            (uint8_t)(i + 1 <= len ? ctx->multiplayer_local_name[i + 1] : 0)
        );
    }

    ctx->multiplayer_name_sent = true;
    mp_log(ctx, "local name sent");
    return 0;
}

int app_multiplayer_send_cancel(t_ctx *ctx) {
    if (ctx == NULL || !ctx->is_multiplayer) return 1;

    int result = mp_send_packet(MP_PACKET_CANCEL, 0, 0, 0);
    mp_log(ctx, result == 0 ? "cancel sent" : "cancel send failed");
    return result;
}

int app_multiplayer_send_ping(t_ctx *ctx) {
    if (ctx == NULL || !ctx->is_multiplayer) return 1;
    return mp_send_packet(MP_PACKET_PING, 0, 0, 0);
}

int app_multiplayer_send_key(t_ctx *ctx, uint8_t scancode) {
    if (ctx == NULL || !ctx->is_multiplayer || !ctx->multiplayer_role_assigned) return 1;

    uint8_t key_index = MAKE_FROM_BREAK(scancode);
    if (key_index != KEY_W && key_index != KEY_A && key_index != KEY_D && key_index != KEY_S && key_index != KEY_E) {
        return 0;
    }

    int result = mp_send_packet(MP_PACKET_KEY, ctx->multiplayer_local_player, scancode, 0);

    FILE *f = fopen("/tmp/game_debug.log", "a");
    if (f) {
        fprintf(f, "[MP] tx key player=%u scancode=0x%02X result=%d\n", ctx->multiplayer_local_player, scancode, result);
        fclose(f);
    }

    return result;
}

int app_multiplayer_send_player_state(t_ctx *ctx, uint8_t player_id) {
    if (ctx == NULL || !ctx->is_multiplayer || !ctx->multiplayer_role_assigned) return 1;
    if (player_id >= MAX_PLAYERS) return 1;

    player_t *player = &ctx->game.players[player_id];

    uint8_t state_value = player->lives & 0x7F;
    if (player->active) state_value |= 0x80;

    int result = mp_send_packet(MP_PACKET_PLAYER_STATE, player_id, state_value, player->powerups);

    FILE *f = fopen("/tmp/game_debug.log", "a");
    if (f) {
        fprintf(f, "[MP] tx state player=%u lives=%u active=%u result=%d\n", player_id, player->lives, player->active, result);
        fclose(f);
    }

    return result;
}

int app_multiplayer_send_pause(t_ctx *ctx, bool paused) {
    if (ctx == NULL || !ctx->is_multiplayer || !ctx->multiplayer_role_assigned) return 1;

    int result = mp_send_packet(MP_PACKET_PAUSE, paused ? 1 : 0, 0, 0);

    FILE *f = fopen("/tmp/game_debug.log", "a");
    if (f) {
        fprintf(f, "[MP] tx pause paused=%u result=%d\n", paused, result);
        fclose(f);
    }

    return result;
}
