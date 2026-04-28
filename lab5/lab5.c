#include <lcom/lcf.h>
#include <lcom/lab5.h>

#include <stdint.h>
#include <stdio.h>

#include "video.h"

int main(int argc, char *argv[]) {
    // sets the language of LCF messages (can be either EN-US or PT-PT)
    lcf_set_language("EN-US");

    // enables to log function invocations that are being "wrapped" by LCF
    // [comment this out if you don't want/need it]
    lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

    // enables to save the output of printf function calls on a file
    // [comment this out if you don't want/need it]
    lcf_log_output("/home/lcom/labs/lab5/output.txt");

    // handles control over to LCF
    // [LCF handles command line arguments and invokes the right function]
    if (lcf_start(argc, argv))
        return 1;

    // LCF clean up tasks
    // [must be the last statement before return]
    lcf_cleanup();

    return 0;
}

int wait_esc() {
    int ipc_status, r;
    message msg;
    uint32_t scancode = 0;
    int hook_id = 1;
    uint8_t irq_set = BIT(1);

    if (sys_irqsetpolicy(1, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id) != 0) return 1;

    while (scancode != 0x81) {  // ESC break code
        if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) continue;

        if (is_ipc_notify(ipc_status) && _ENDPOINT_P(msg.m_source) == HARDWARE) {
            if (msg.m_notify.interrupts & irq_set) {
                sys_inb(0x60, &scancode);
            }
        }
    }

    if (sys_irqrmpolicy(&hook_id) != 0) return 1;
    return 0;
}

int(video_test_init)(uint16_t mode, uint8_t delay) {
    if (vg_init_mode(mode) != 0) {
        printf("Error initializing video mode\n");
        return 1;
    }

    tickdelay(sys_hz() * delay);

    if (vg_exit() != 0) {
        printf("Error exiting video mode\n");
        return 1;
    }

    return 0;
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height, uint32_t color) {
    
    if (init_video_mem(mode) != 0) return 1;
    if (vg_init_mode(mode) != 0) return 1;

    draw_rectangle(x, y, width, height, color);

    wait_esc();
    vg_exit();

    return 0;
}

int(video_test_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y) {
    if (init_framebuffer(VBE_MODE_105) != 0) return 1;
    if (vg_init_mode(VBE_MODE_105) != 0) return 1;

    xpm_image_t img;
    uint8_t *map = xpm_load(xpm, XPM_INDEXED, &img);
    if (map == NULL) {
        vg_exit();
        return 1;
    }

    draw_xpm(map, img, x, y);

    wait_esc();
    vg_exit();

    return 0;
}
