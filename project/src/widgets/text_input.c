#include "widget.h"
#include "../draw.h"
#include "../gui/gui.h"

#include <string.h>
#include <stdlib.h>

static char get_char_from_scancode(uint8_t scancode) {
    if (scancode >= 0x02 && scancode <= 0x0A) return "123456789"[scancode - 0x02];
    if (scancode == 0x0B) return '0';
    if (scancode == 0x1E) return 'A';
    if (scancode == 0x30) return 'B';
    if (scancode == 0x2E) return 'C';
    if (scancode == 0x20) return 'D';
    if (scancode == 0x12) return 'E';
    if (scancode == 0x21) return 'F';
    if (scancode == 0x22) return 'G';
    if (scancode == 0x23) return 'H';
    if (scancode == 0x17) return 'I';
    if (scancode == 0x24) return 'J';
    if (scancode == 0x25) return 'K';
    if (scancode == 0x26) return 'L';
    if (scancode == 0x32) return 'M';
    if (scancode == 0x31) return 'N';
    if (scancode == 0x18) return 'O';
    if (scancode == 0x19) return 'P';
    if (scancode == 0x10) return 'Q';
    if (scancode == 0x13) return 'R';
    if (scancode == 0x1F) return 'S';
    if (scancode == 0x14) return 'T';
    if (scancode == 0x16) return 'U';
    if (scancode == 0x2F) return 'V';
    if (scancode == 0x11) return 'W';
    if (scancode == 0x2D) return 'X';
    if (scancode == 0x15) return 'Y';
    if (scancode == 0x2C) return 'Z';
    if (scancode == 0x39) return ' ';
    if (scancode == 0x34) return '.';
    if (scancode == 0x33) return ',';
    if (scancode == 0x0C) return '-';
    return 0;
}

static void on_text_input_key_press(struct s_widget *self, uint8_t scancode, void *state) {
    (void)state;
    if (!WIDGET_IS_FOCUSED(self)) return;

    if (scancode == 0x1C) { // Enter
        t_gui *gui = (t_gui*)state;
        gui_set_focus(gui, NULL);
        return;
    }

    uint32_t len = strlen(self->data.text_input.buffer);

    if (scancode == 0x4B) { // Left arrow
        if (self->data.text_input.cursor_pos > 0) {
            self->data.text_input.cursor_pos--;
            self->data.text_input.cursor_visible = true; // blink reset
            self->data.text_input.blink_timer = 0;
        }
    } else if (scancode == 0x4D) { // Right arrow
        if (self->data.text_input.cursor_pos < len) {
            self->data.text_input.cursor_pos++;
            self->data.text_input.cursor_visible = true; // blink reset
            self->data.text_input.blink_timer = 0;
        }
    } else if (scancode == 0x0E) { // Backspace
        if (self->data.text_input.cursor_pos > 0) {
            memmove(&self->data.text_input.buffer[self->data.text_input.cursor_pos - 1],
                    &self->data.text_input.buffer[self->data.text_input.cursor_pos],
                    len - self->data.text_input.cursor_pos + 1);
            self->data.text_input.cursor_pos--;
            self->data.text_input.cursor_visible = true; // blink reset
            self->data.text_input.blink_timer = 0;
        }
    } else {
        char c = get_char_from_scancode(scancode);
        if (c != 0 && len < self->data.text_input.max_length) {
            memmove(&self->data.text_input.buffer[self->data.text_input.cursor_pos + 1],
                    &self->data.text_input.buffer[self->data.text_input.cursor_pos],
                    len - self->data.text_input.cursor_pos + 1);
            self->data.text_input.buffer[self->data.text_input.cursor_pos] = c;
            self->data.text_input.cursor_pos++;
            self->data.text_input.cursor_visible = true; // blink reset
            self->data.text_input.blink_timer = 0;
        }
    }
}

static void on_text_input_tick(struct s_widget *self, void *state) {
    (void)state;
    if (WIDGET_IS_FOCUSED(self)) {
        self->data.text_input.blink_timer++;
        // Toggle every half second (approx 72 ticks at 144Hz)
        if (self->data.text_input.blink_timer >= 72) {
            self->data.text_input.cursor_visible = !self->data.text_input.cursor_visible;
            self->data.text_input.blink_timer = 0;
        }
    } else {
        self->data.text_input.cursor_visible = false;
        self->data.text_input.blink_timer = 0;
    }
}

static void on_text_input_quit(struct s_widget *self, void *state) {
    t_gui *gui = (t_gui*)state;
    gui_set_focus(gui, NULL);
}

void draw_text_input(t_widget *self, hw_video_t *video) {
    uint32_t abs_x = widget_get_abs_x(self);
    uint32_t abs_y = widget_get_abs_y(self);
    
    uint32_t color = WIDGET_IS_CLICKED(self) ? W95_WHITE : W95_GRAY;
    hw_vbe_draw_rect(video, abs_x, abs_y, self->width, self->height, color);
    
    // text inputs are always sunken
    draw_win95_border(video, abs_x, abs_y, self->width, self->height, true);

    if (self->data.text_input.buffer != NULL) {
        draw_string(video, self->data.text_input.buffer, abs_x + 4, abs_y + (self->height - 11) / 2, color);
        
        if (self->data.text_input.cursor_visible && WIDGET_IS_FOCUSED(self)) {
            // cursor is placed after the text length
            int text_w = self->data.text_input.cursor_pos * 11;
            hw_vbe_draw_vline(video, abs_x + 4 + text_w, abs_y + (self->height - 11) / 2, 11, W95_BLACK);
            hw_vbe_draw_vline(video, abs_x + 5 + text_w, abs_y + (self->height - 11) / 2, 11, W95_BLACK);
        }
    }
}

t_widget* widget_add_text_input(t_widget *parent, int32_t x, int32_t y, uint32_t w, uint32_t h, const char *default_text, void (*on_click)(t_widget*, void*)) {
    t_widget *input = widget_create(TEXT_INPUT, x, y, w, h);
    input->data.text_input.buffer = (char*)malloc(256);
    memset(input->data.text_input.buffer, 0, 256);
    if (default_text) {
        strncpy(input->data.text_input.buffer, default_text, 16);
    }
    input->data.text_input.max_length = 16;
    input->data.text_input.cursor_pos = strlen(input->data.text_input.buffer);
    input->data.text_input.cursor_visible = false;
    input->data.text_input.blink_timer = 0;

    input->on_click = on_click;
    input->on_tick = on_text_input_tick;
    input->on_key_press = on_text_input_key_press;
    input->on_quit = on_text_input_quit;
    input->on_destroy = (void (*)(struct s_widget*)) free;
    widget_add_child(parent, input);
    return input;
}
