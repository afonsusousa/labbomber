#ifndef LIB_KEYBOARD_I8042_H
#define LIB_KEYBOARD_I8042_H
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

// Make and Break codes
#define KBD_BREAK_CODE_BIT  0x80
#define IS_BREAK_CODE(sc)   ((sc) & KBD_BREAK_CODE_BIT)
#define IS_MAKE_CODE(sc)    (((sc) & KBD_BREAK_CODE_BIT) == 0)
#define BREAK_FROM_MAKE(sc) ((sc) | KBD_BREAK_CODE_BIT)
#define MAKE_FROM_BREAK(sc) ((sc) & 0x7F)

// Key scancodes
#define KEY_ESC             0x01
#define KEY_TAB             0x0F
#define KEY_Q               0x10
#define KEY_W               0x11
#define KEY_E               0x12
#define KEY_A               0x1E
#define KEY_S               0x1F
#define KEY_D               0x20
#define KEY_SPACE           0x39
#define KEY_P               0x19
#define KEY_F3              0x3D
#define KEY_ENTER           0x1C
#define KEY_CTRL            0x1D
#define KEY_SHIFT_LEFT      0x2A
#define KEY_SHIFT_RIGHT     0x36

#define KBC_MAX_TRIES   10
#define KBC_DELAY_US    20000

// Mouse specific commands
#define MOUSE_ENABLE_DATA   0xF4
#define MOUSE_DISABLE_DATA  0xF5
#define MOUSE_ACK           0xFA
#define MOUSE_NACK          0xFE
#define MOUSE_ERROR         0xFC

#endif /* LIB_KEYBOARD_I8042_H */
