#pragma once

#include "../utils/utils.h" 

#define KBC_IRQ         1
#define MOUSE_IRQ       12

#define KBC_OUTBUF_REG  0x60
#define KBC_INBUF_REG   0x60
#define KBC_STATUS_REG  0x64
#define KBC_CMD_REG     0x64

#define KBC_READ_CMD    0x20
#define KBC_WRITE_CMD   0x60
#define KBC_WRITE_MOUSE 0xD4

// Status register bits
#define ST_PARITY_BIT   BIT(7)
#define ST_TIMEOUT_BIT  BIT(6)
#define ST_AUX_BIT      BIT(5)
#define ST_IBF_BIT      BIT(1)
#define ST_OBF_BIT      BIT(0)

#define KBC_MAX_TRIES   10
#define KBC_DELAY_US    20000

// Mouse specific commands
#define MOUSE_ENABLE_DATA   0xF4
#define MOUSE_DISABLE_DATA  0xF5
#define MOUSE_ACK           0xFA
#define MOUSE_NACK          0xFE
#define MOUSE_ERROR         0xFC
