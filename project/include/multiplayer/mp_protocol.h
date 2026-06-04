#ifndef MP_PROTOCOL_H
#define MP_PROTOCOL_H

#include "core/application.h"
#include <stdint.h>

/* Packet framing */
#define MP_PACKET_START        0xA5
#define MP_PACKET_PAYLOAD_SIZE 3

/* Packet types */
#define MP_PACKET_HELLO        0x01
#define MP_PACKET_KEY          0x02
#define MP_PACKET_PLAYER_STATE 0x03
#define MP_PACKET_PAUSE        0x04
#define MP_PACKET_READY        0x05
#define MP_PACKET_START_GAME   0x06
#define MP_PACKET_NAME_PART    0x07
#define MP_PACKET_CANCEL       0x08
#define MP_PACKET_PING         0x09

int  mp_send_packet(uint8_t type, uint8_t d0, uint8_t d1, uint8_t d2);
void mp_log(t_ctx *ctx, const char *message);

#endif /* MP_PROTOCOL_H */
