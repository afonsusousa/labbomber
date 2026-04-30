#include "keyboard.h"
#include "../utils/utils.h"
#include <minix/syslib.h>
#include <minix/drivers.h>

void hw_keyboard_init(keyboard_t *kbd) {
    if (kbd == NULL) return;

    kbd->hook_id = KBC_IRQ;
    kbd->irq_bit = KBC_IRQ;
    kbd->mask = BIT(kbd->irq_bit);
}

int hw_keyboard_subscribe_int(keyboard_t *kbd) {
    if (kbd == NULL) return 1;

    if (kbd->hook_id == 0) kbd->hook_id = KBC_IRQ;
    kbd->irq_bit = kbd->hook_id;
    kbd->mask = BIT(kbd->irq_bit);

    return sys_irqsetpolicy(KBC_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &kbd->hook_id);
}

int hw_keyboard_unsubscribe_int(keyboard_t *kbd) {
    if (kbd == NULL) return 1;

    return sys_irqrmpolicy(&kbd->hook_id);
}

void hw_keyboard_ih(keyboard_t *kbd) {
    uint8_t status = 0;
    uint8_t scancode = 0;

    if (util_sys_inb(KBC_STATUS_REG, &status) != 0) return;
    if (!(status & ST_OBF_BIT)) return; // buffer full
    if (util_sys_inb(KBC_OUTBUF_REG, &scancode) != 0) return;
    if ((status & ST_PARITY_BIT) || (status & ST_TIMEOUT_BIT) || (status & ST_AUX_BIT)) return;

    if (scancode == 0xE0) {
        kbd->is_two_bytes = true;
    } else {
        kbd->scancode = scancode;

        bool is_make = (scancode & 0x80) == 0;
        
        uint8_t index = scancode & 0x7F;

        if (kbd->is_two_bytes) index += 128;

        kbd->keys_pressed[index] = is_make;
        kbd->is_two_bytes = false;
    }
}
