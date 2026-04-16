#ifndef I8042_H
#define I8042_H

#define KBC_CMD_REG 0x64
#define KBC_DATA_REG 0x60

#define KBC_WRITE_MOUSE 0xD4

#define MOUSE_ENABLE_DATA 0xF4
#define MOUSE_DISABLE_DATA 0xF5

#define MOUSE_ACK 0xFA
#define MOUSE_NACK 0xFE
#define MOUSE_ERROR 0xFC

#define IRQ_MOUSE 12

#endif