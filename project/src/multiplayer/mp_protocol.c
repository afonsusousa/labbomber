#include "mp_protocol.h"
#include "core/application.h"
#include "serial_port.h"
#include <stdio.h>
#include <stdint.h>

int mp_send_packet(uint8_t type, uint8_t d0, uint8_t d1, uint8_t d2) {
    if (serial_send_byte(MP_PACKET_START) != 0) return 1;
    if (serial_send_byte(type)            != 0) return 1;
    if (serial_send_byte(d0)              != 0) return 1;
    if (serial_send_byte(d1)             != 0) return 1;
    return serial_send_byte(d2);
}

void mp_log(t_ctx *ctx, const char *message) {
    FILE *f = fopen("/tmp/game_debug.log", "a");
    if (f == NULL) return;

    fprintf(f,
        "[MP] %s local=%u remote=%u ready=%d role=%d nonce=%u remote_nonce=%u\n",
        message,
        ctx != NULL ? ctx->multiplayer_local_player       : 255,
        ctx != NULL ? ctx->multiplayer_remote_player      : 255,
        ctx != NULL && ctx->multiplayer_partner_ready,
        ctx != NULL && ctx->multiplayer_role_assigned,
        ctx != NULL ? ctx->multiplayer_local_nonce        : 0,
        ctx != NULL ? ctx->multiplayer_remote_nonce       : 0);

    fclose(f);
}
