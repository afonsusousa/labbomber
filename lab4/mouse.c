#include <lcom/lcf.h>

#include "mouse.h"
#include "i8042.h"

static int hook_id = 2;

int mouse_subscribe_int(uint8_t *bit_no) {
    *bit_no = hook_id;
    return sys_irqsetpolicy(IRQ_MOUSE, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id);
}

int mouse_unsubscribe_int() {
    return sys_irqrmpolicy(&hook_id);
}

void (mouse_ih)() {
    // por fazer
}

int mouse_enable_data_reporting() {
    // por fazer
    return 0;
}

int mouse_disable_data_reporting() {
    // por fazer
    return 0;
}