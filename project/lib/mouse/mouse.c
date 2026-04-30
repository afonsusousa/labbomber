#include "mouse.h"
#include "../utils/utils.h"
#include <minix/syslib.h>
#include <minix/drivers.h>

void hw_mouse_init(hw_mouse_t *mouse) {
    if (mouse == NULL) return;

    mouse->hook_id = MOUSE_IRQ;
    mouse->irq_bit = MOUSE_IRQ;
    mouse->mask = BIT(mouse->irq_bit);
    mouse->byte_index = 0;
    mouse->packet[0] = 0;
    mouse->packet[1] = 0;
    mouse->packet[2] = 0;
}

int hw_mouse_subscribe_int(hw_mouse_t *mouse) {
    if (mouse == NULL) return 1;

    if (mouse->hook_id == 0) mouse->hook_id = MOUSE_IRQ;
    mouse->irq_bit = mouse->hook_id;
    mouse->mask = BIT(mouse->irq_bit);

    return sys_irqsetpolicy(MOUSE_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &mouse->hook_id);
}

int hw_mouse_unsubscribe_int(hw_mouse_t *mouse) {
    if (mouse == NULL) return 1;

    return sys_irqrmpolicy(&mouse->hook_id);
}

static int kbc_read_status(uint8_t *status) {
    return util_sys_inb(KBC_CMD_REG, status);
}

// Logic essentially lab4
static int kbc_write_to_mouse(uint8_t cmd) {
    uint8_t status = 0;

    for (int i = 0; i < KBC_MAX_TRIES; i++) {
        if (kbc_read_status(&status) != 0) return 1;
        if (!(status & ST_IBF_BIT)) {
            if (sys_outb(KBC_CMD_REG, KBC_WRITE_MOUSE) != 0) return 1;
            for (int j = 0; j < KBC_MAX_TRIES; j++) {
                if (kbc_read_status(&status) != 0) return 1;
                if (!(status & ST_IBF_BIT)) {
                    if (sys_outb(KBC_OUTBUF_REG, cmd) != 0) return 1;
                    return 0;
                }
                tickdelay(micros_to_ticks(KBC_DELAY_US));
            }
            return 1;
        }
        tickdelay(micros_to_ticks(KBC_DELAY_US));
    }
    return 1;
}

static int mouse_read_response(uint8_t *resp) {
    uint8_t status = 0;
    for (int i = 0; i < KBC_MAX_TRIES * 4; i++) {
        if (kbc_read_status(&status) != 0) return 1;
        if (status & ST_OBF_BIT) {
            uint8_t data = 0;
            if (util_sys_inb(KBC_OUTBUF_REG, &data) != 0) return 1;
            if (status & (ST_PARITY_BIT | ST_TIMEOUT_BIT)) continue;
            if (!(status & ST_AUX_BIT)) continue;
            
            if (data == MOUSE_ACK || data == MOUSE_NACK || data == MOUSE_ERROR) {
                *resp = data;
                return 0;
            }
            continue;
        }
        tickdelay(micros_to_ticks(KBC_DELAY_US));
    }
    return 1;
}

int mouse_write_cmd(uint8_t cmd) {
    uint8_t resp = 0;
    for (int i = 0; i < KBC_MAX_TRIES; i++) {
        if (kbc_write_to_mouse(cmd) != 0) return 1;
        if (mouse_read_response(&resp) != 0) return 1;
        
        if (resp == MOUSE_ACK) return 0;
        if (resp == MOUSE_NACK) continue;
        if (resp == MOUSE_ERROR) return 1;
    }
    return 1;
}

bool hw_mouse_ih(hw_mouse_t *mouse_state) {
    uint8_t status = 0;
    uint8_t byte = 0;

    if (util_sys_inb(KBC_STATUS_REG, &status) != 0) return false;
    if (!(status & ST_OBF_BIT)) return false;
    if (util_sys_inb(KBC_OUTBUF_REG, &byte) != 0) return false;
    if (!(status & ST_AUX_BIT)) return false;
    if (status & (ST_PARITY_BIT | ST_TIMEOUT_BIT)) return false;

    if (mouse_state->byte_index == 0 && !(byte & BIT(3))) return false; // out of sync

    mouse_state->packet[mouse_state->byte_index] = byte;
    mouse_state->byte_index++;

    if (mouse_state->byte_index == 3) {
        mouse_state->byte_index = 0;

        mouse_state->left_click = mouse_state->packet[0] & BIT(0);
        mouse_state->right_click = mouse_state->packet[0] & BIT(1);
        mouse_state->middle_click = mouse_state->packet[0] & BIT(2);

        mouse_state->delta_x = (mouse_state->packet[0] & BIT(4)) ? (int16_t)(0xFF00 | mouse_state->packet[1]) : (int16_t)mouse_state->packet[1];
        mouse_state->delta_y = (mouse_state->packet[0] & BIT(5)) ? (int16_t)(0xFF00 | mouse_state->packet[2]) : (int16_t)mouse_state->packet[2];
        
        mouse_state->x += mouse_state->delta_x;
        mouse_state->y -= mouse_state->delta_y; 
        
        return true;
    }

    return false;
}
