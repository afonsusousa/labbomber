#include "widget.h"
#include "draw.h"
#include "state.h"

#include <string.h>
#include <stdlib.h>

#define MIN(a, b) ((uint32_t)(a) < (uint32_t)(b) ? (uint32_t)(a) : (uint32_t)(b))
#define MAX(a, b) ((uint32_t)(a) > (uint32_t)(b) ? (uint32_t)(a) : (uint32_t)(b))

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

static bool has_selection(struct s_widget *self) {
    return self->data.text_input.selection_start != -1 && 
           self->data.text_input.selection_start != (int32_t)self->data.text_input.cursor_pos;
}

static void reset_blink(struct s_widget *self) {
    self->data.text_input.cursor_visible = true;
    self->data.text_input.blink_timer = 0;
}

static uint32_t get_pos_from_mouse(struct s_widget *self, t_state *gui) {
    int32_t rel_x = gui->input.mouse_x - (get_abs_x(self) + 4);
    if (rel_x < 0) return 0;
    return MIN((rel_x + 5) / 11, strlen(self->data.text_input.buffer));
}

static void delete_selection(struct s_widget *self, uint32_t len) {
    if (!has_selection(self)) return;
    
    uint32_t min_s = MIN(self->data.text_input.selection_start, self->data.text_input.cursor_pos);
    uint32_t max_s = MAX(self->data.text_input.selection_start, self->data.text_input.cursor_pos);
    
    memmove(&self->data.text_input.buffer[min_s],
            &self->data.text_input.buffer[max_s],
            len - max_s + 1);
            
    self->data.text_input.cursor_pos = min_s;
    self->data.text_input.selection_start = -1;
}

static uint32_t word_start_offset(const char *buffer, uint32_t cursor_pos) {
    if (cursor_pos == 0) return 0;
    uint32_t pos = cursor_pos;

    while (pos > 0 && buffer[pos - 1] == ' ')
        pos--;
    while (pos > 0 && buffer[pos - 1] != ' ')
        pos--;
        
    return pos;
}

static uint32_t word_end_offset(const char *buffer, uint32_t cursor_pos) {
    uint32_t len = (uint32_t)strlen(buffer);
    if (cursor_pos >= len) return len;
    uint32_t pos = cursor_pos;

    while (pos < len && buffer[pos] != ' ')
        pos++;
    while (pos < len && buffer[pos] == ' ')
        pos++;
        
    return pos;
}

static void on_text_input_key_press(struct s_widget *self, uint8_t scancode, void *state) {
    if (!WIDGET_IS_FOCUSED(self)) return;
    
    t_state *gui = (t_state*)state;
    bool is_shift = gui->input.shift_down;
    bool is_ctrl  = gui->input.ctrl_down;

    if (scancode == 0x1C) { // Enter
        gui_set_focus(gui, NULL);
        return;
    }

    uint32_t *cursor = &self->data.text_input.cursor_pos;
    int32_t  *anchor = &self->data.text_input.selection_start;
    char     *buf    = self->data.text_input.buffer;
    uint32_t len     = strlen(buf);

    if (scancode == 0x4B) { // Left arrow
        if (is_shift && !has_selection(self)) *anchor = *cursor;

        if (is_ctrl)
            *cursor = word_start_offset(buf, *cursor);
        else if (has_selection(self) && !is_shift)
            *cursor = MIN(*anchor, *cursor);
        else if (*cursor > 0)
            (*cursor)--;
        
        if (!is_shift) *anchor = -1;
        reset_blink(self);
        
    } else if (scancode == 0x4D) { // Right arrow
        if (is_shift && !has_selection(self)) *anchor = *cursor;

        if (is_ctrl)
            *cursor = word_end_offset(buf, *cursor);
        else if (has_selection(self) && !is_shift)
            *cursor = MAX(*anchor, *cursor);
        else if (*cursor < len)
            (*cursor)++;
        
        if (!is_shift) *anchor = -1;
        reset_blink(self);
        
    } else if (scancode == 0x0E) { // Backspace
        if (has_selection(self)) {
            delete_selection(self, len);
        } else if (*cursor > 0) {
            uint32_t target = is_ctrl ? word_start_offset(buf, *cursor) : (*cursor) - 1;
            
            memmove(&buf[target], &buf[*cursor], len - *cursor + 1);
            *cursor = target;
        }
        
        *anchor = -1;
        reset_blink(self);
        
    } else { // Typing a character
        char c = get_char_from_scancode(scancode);
        if (c != 0) {
            if (has_selection(self)) {
                delete_selection(self, len);
                len = strlen(buf); // Refresh length since we just deleted text
            } else {
                *anchor = -1;
            }
            if (len < self->data.text_input.max_length) {
                memmove(&buf[*cursor + 1], &buf[*cursor], len - *cursor + 1);
                buf[*cursor] = c;
                (*cursor)++;
                reset_blink(self);
            }
        }
    }
}

static void on_text_input_tick(struct s_widget *self, void *state) {
    (void)state;
    if (WIDGET_IS_FOCUSED(self)) {
        if (++self->data.text_input.blink_timer >= 72) {
            self->data.text_input.cursor_visible = !self->data.text_input.cursor_visible;
            self->data.text_input.blink_timer = 0;
        }
    } else {
        self->data.text_input.cursor_visible = false;
        self->data.text_input.blink_timer = 0;
    }
}

static void on_text_input_quit(struct s_widget *self, void *state) {
    gui_set_focus((t_state*)state, NULL);
    self->data.text_input.selection_start = -1;
}

static void on_text_input_press(struct s_widget *self, void *state) {
    self->data.text_input.cursor_pos = get_pos_from_mouse(self, (t_state*)state);
    self->data.text_input.selection_start = self->data.text_input.cursor_pos;
    reset_blink(self);
}

static void on_text_input_drag(struct s_widget *self, void *state) {
    self->data.text_input.cursor_pos = get_pos_from_mouse(self, (t_state*)state);
    reset_blink(self);
}

void draw_text_input(t_widget *self, hw_video_t *video) {
    uint32_t x = get_abs_x(self);
    uint32_t y = get_abs_y(self);
    uint32_t base_color = WIDGET_IS_CLICKED(self) ? W95_LIGHT_GRAY : W95_GRAY;
    
    hw_vbe_draw_rect(video, x, y, self->width, self->height, base_color);
    draw_win95_border(video, x, y, self->width, self->height, true);

    char *buf = self->data.text_input.buffer;

    if (buf != NULL) {
        uint32_t cursor  = self->data.text_input.cursor_pos;
        int32_t  anchor  = self->data.text_input.selection_start;
        bool     visible = self->data.text_input.cursor_visible;
        
        x += 4;
        y += (self->height - 11) / 2; 
        
        bool has_sel = (WIDGET_IS_FOCUSED(self) || WIDGET_IS_CLICKED(self)) && has_selection(self);
        uint32_t min_s = 0, max_s = 0;
        
        //Selection Highlight Background
        if (has_sel) {
            min_s = MIN(anchor, cursor);
            max_s = MIN(MAX(anchor, cursor), strlen(buf));
            hw_vbe_draw_rect(video, x + (min_s * 11), y, (max_s - min_s) * 11, 11, W95_TEAL);
        }
        
        //characters
        for (uint32_t i = 0; buf[i] != '\0'; i++) {
            bool in_sel = has_sel && (i >= min_s) && (i < max_s);
            char c[2] = { buf[i], '\0' };
            draw_string(video, c, x + (i * 11), y, in_sel ? W95_TEAL : base_color);
        }
        
        //Blinking Cursor
        if (visible && WIDGET_IS_FOCUSED(self)) {
            uint32_t cur_x = x + (cursor * 11);
            
            hw_vbe_draw_vline(video, cur_x, y, 11, W95_BLACK);
            hw_vbe_draw_vline(video, cur_x + 1, y, 11, W95_BLACK);
        }
    }
}

t_widget* widget_add_text_input(t_widget *parent, int32_t x, int32_t y, uint32_t w, uint32_t h, const char *default_text, void (*on_click)(t_widget*, void*), const char *name) {
    t_widget *self = widget_create(TEXT_INPUT, x, y, w, h, name);
    
    self->data.text_input.buffer = (char*)calloc(256, 1);
    if (default_text) {
        strncpy(self->data.text_input.buffer, default_text, 16);
    }
    
    self->data.text_input.max_length = 16;
    self->data.text_input.cursor_pos = strlen(self->data.text_input.buffer);
    self->data.text_input.selection_start = -1;
    self->data.text_input.cursor_visible = false;
    self->data.text_input.blink_timer = 0;

    self->on_click = on_click;
    self->on_press = on_text_input_press;
    self->on_drag = on_text_input_drag;
    self->on_tick = on_text_input_tick;
    self->on_key_press = on_text_input_key_press;
    self->on_quit = on_text_input_quit;
    self->on_destroy = (void (*)(struct s_widget*)) free;
    
    widget_add_child(parent, self);
    return self;
}
