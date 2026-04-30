#include <minix/syslib.h>
#include <minix/drivers.h>
#include <stdint.h>
#include "timer.h"
#include "../utils/utils.h"

void hw_timer_init(hw_timer_t *timer) {
    if (timer == NULL) return;

    timer->hook_id = TIMER0_IRQ;
    timer->irq_bit = TIMER0_IRQ;
    timer->mask = BIT(timer->irq_bit);
    timer->ticks = 0;
}

int timer_set_frequency(uint8_t timer, uint32_t freq) {
    if (timer > 2 || freq > TIMER_FREQ || freq < 19) 
        return 1;

    uint32_t div = TIMER_FREQ / freq;

    uint8_t old_status = 0;
    
    // Equivalent of timer_get_conf
    uint8_t rbCmd = TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer);
    if (sys_outb(TIMER_CTRL, rbCmd) != 0) return 1;
    if (util_sys_inb(TIMER_0 + timer, &old_status) != 0) return 1;

    uint8_t ctrl_word = (timer << 6) | TIMER_LSB_MSB | (old_status & 0x0F);

    if (sys_outb(TIMER_CTRL, ctrl_word) != 0) return 1;

    uint8_t lsb_val = lsb((uint16_t)div);
    uint8_t msb_val = msb((uint16_t)div);

    if (sys_outb(TIMER_0 + timer, lsb_val) != 0) return 1;
    if (sys_outb(TIMER_0 + timer, msb_val) != 0) return 1;


    return 0;
}

int hw_timer_subscribe_int(hw_timer_t *timer) {
    if (timer == NULL) return 1;

    if (timer->hook_id == 0) timer->hook_id = TIMER0_IRQ;
    timer->irq_bit = timer->hook_id;
    timer->mask = BIT(timer->irq_bit);

    return sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &timer->hook_id);
}

int hw_timer_unsubscribe_int(hw_timer_t *timer) {
    if (timer == NULL) return 1;

    return sys_irqrmpolicy(&timer->hook_id);
}

void hw_timer_int_handler(hw_timer_t *timer) {
    if (timer == NULL) return;

    timer->ticks++;
}

uint32_t hw_timer_get_no_interrups(const hw_timer_t *timer) {
    if (timer == NULL) return 0;

    return timer->ticks;
}

void hw_timer_reset_ticks(hw_timer_t *timer) {
    if (timer == NULL) return;

    timer->ticks = 0;
}

bool hw_timer_elapsed(const hw_timer_t *timer, uint32_t start_tick, uint32_t ticks_to_wait) {
    if (timer == NULL) return false;

    return (timer->ticks - start_tick) >= ticks_to_wait;
}

