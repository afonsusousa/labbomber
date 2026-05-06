#include "widget.h"
#include "gui.h"
#include "application.h"
#include "draw.h"
#include <stdio.h>
#include <string.h>

/*
--------------------------------------------------------------------------------
    SIMPLE TEXT DISPLAY
--------------------------------------------------------------------------------
*/

void draw_text(t_widget *self, hw_video_t *video, void *state) {
    (void)state;
    hw_vbe_draw_rect(video, get_abs_x(self), get_abs_y(self), self->width, self->height, W95_LIGHT_GRAY);
    if (self->data.text_display.text != NULL) {
        draw_string(video, self->data.text_display.text, get_abs_x(self) + 4, get_abs_y(self) + 4, W95_LIGHT_GRAY);
    }
}

t_widget* widget_add_text(t_widget *parent, int32_t x, int32_t y, uint32_t w, uint32_t h, const char *text, const char *name) {
    t_widget *txt = widget_create(TEXT, x, y, w, h, name);
    txt->data.text_display.text = (char*)text;
    widget_add_child(parent, txt);
    return txt;
}

/*
--------------------------------------------------------------------------------
    BUTTONS
--------------------------------------------------------------------------------
*/

void btn_on_tick(t_widget *self, void *state) {
    if (self->data.button.action_delay_timer > 0) {
        self->data.button.action_delay_timer--;
        if (self->data.button.action_delay_timer == 0) {
            if (self->data.button.on_click_action != NULL) {
                self->data.button.on_click_action(self, state);
            }
        }
    }
}

void btn_internal_on_click(t_widget *self, void *state) {
    self->data.button.action_delay_timer = 7;
}

void btn_on_key_press(t_widget *self, uint8_t scancode, void *state) {
    if (scancode == 0x1C) {
        WIDGET_SET_CLICKED(self, true);
    } 
    else if (scancode == 0x9C) {
        if (WIDGET_IS_CLICKED(self)) {
            WIDGET_SET_CLICKED(self, false);
            btn_internal_on_click(self, state);
        }
    }
}

void draw_button(t_widget *self, hw_video_t *video, void *state) {
    (void)state;
    uint32_t abs_x = get_abs_x(self);
    uint32_t abs_y = get_abs_y(self);
    
    hw_vbe_draw_rect(video, abs_x, abs_y, self->width, self->height, W95_GRAY);
    draw_win95_border(video, 
        abs_x, abs_y,
        self->width, self->height, 
        WIDGET_IS_CLICKED(self) && WIDGET_IS_FOCUSED(self)
    );

    if (self->data.button.label != NULL) {
        int text_w = strlen(self->data.button.label) * 11;
        int text_x = abs_x + (self->width - text_w) / 2;
        int text_y = abs_y + (self->height - 11) / 2;

        if (WIDGET_IS_CLICKED(self)) { text_x += 1; text_y += 1;}
        draw_string(video, self->data.button.label, text_x, text_y, W95_GRAY);
        if (WIDGET_IS_FOCUSED(self) && self->focus_cue) {
            int focus_x = text_x - 4;
            int focus_y = text_y - 4;
            int focus_w = text_w + 4;
            int focus_h = 11 + 4;
            hw_vbe_draw_hline(video, focus_x, focus_y, focus_w, W95_BLACK);
            hw_vbe_draw_hline(video, focus_x, focus_y + focus_h - 1, focus_w, W95_BLACK);
            hw_vbe_draw_vline(video, focus_x, focus_y, focus_h, W95_BLACK);
            hw_vbe_draw_vline(video, focus_x + focus_w - 1, focus_y, focus_h, W95_BLACK);
        }
    }
}

t_widget* widget_add_button(t_widget *parent, int32_t x, int32_t y, uint32_t w, uint32_t h, const char *label, void (*on_click)(t_widget*, void*), const char *name) {
    t_widget *btn = widget_create(BUTTON, x, y, w, h, name);
    
    btn->data.button.label = (char*)label;
    btn->data.button.action_delay_timer = 0;
    btn->data.button.on_click_action = on_click;
    
    btn->on_click = btn_internal_on_click;
    btn->on_key_press = btn_on_key_press; 
    btn->on_tick = btn_on_tick;
    
    widget_add_child(parent, btn);
    return btn;
}

/*
--------------------------------------------------------------------------------
    SIMPLE CANVAS
--------------------------------------------------------------------------------
*/

void draw_canvas(t_widget *self, hw_video_t *video, void *state) {
    (void)state;
    hw_vbe_draw_rect(video, get_abs_x(self), get_abs_y(self), self->width, self->height, W95_TEAL);
}

/* 
--------------------------------------------------------------------------------  
    DIALOGS
--------------------------------------------------------------------------------
*/

static void on_dialog_press(t_widget *self, void *state) {
    t_ctx *ctx = (t_ctx*)state;
    t_gui *gui = &ctx->gui;
    int32_t abs_y = get_abs_y(self);
    if (gui->input.mouse_y >= abs_y && gui->input.mouse_y < abs_y + 24) {
        gui->drag.dragged_widget = self;
        gui->drag.dragt_dx = gui->input.mouse_x - get_abs_x(self);
        gui->drag.dragt_dy = gui->input.mouse_y - abs_y;
    }
}

static void on_dialog_drag(t_widget *self, void *state) {
    t_ctx *ctx = (t_ctx*)state;
    t_gui *gui = &ctx->gui;
    int32_t new_x = gui->input.mouse_x - gui->drag.dragt_dx;
    int32_t new_y = gui->input.mouse_y - gui->drag.dragt_dy;
    widget_set_position(self, new_x, new_y);
}

static void build_dialog_close_button_name(const t_widget *parent, char *buffer, size_t buffer_size) {
    const char *parent_name = (parent != NULL && parent->name != NULL) ? parent->name : "dialog";
    snprintf(buffer, buffer_size, "%s_close_button", parent_name);
}

void draw_dialog(t_widget *self, hw_video_t *video, void *state) {
    (void)state;
    uint32_t abs_x = get_abs_x(self);
    uint32_t abs_y = get_abs_y(self);
    
    hw_vbe_draw_rect(video, abs_x, abs_y, self->width, self->height, W95_GRAY);
    
    // dialog windows are always raised
    draw_win95_border(video, abs_x, abs_y, self->width, self->height, false);

    if (self->data.dialog.title != NULL) {
        hw_vbe_draw_rect(video, abs_x + 4, abs_y + 4, self->width - 8, 20, 0x000080); // Title bar background
        draw_string(video, self->data.dialog.title, abs_x + 8, abs_y + 8, 0x000080);
    }
}

t_widget* widget_add_dialog(t_widget *parent, const char *title, uint32_t w, uint32_t h, uint32_t screen_w, uint32_t screen_h, void (*on_close)(t_widget*, void*), const char *name) {
    t_widget *dialog = widget_create(DIALOG, 0, 0, w, h, name);
    dialog->data.dialog.title = (char*)title;

    // Default dialog dragging behavior
    dialog->on_press = on_dialog_press;
    dialog->on_drag = on_dialog_drag;

    uint32_t dlg_x = (screen_w > w) ? (screen_w - w) / 2 : 0;
    uint32_t dlg_y = (screen_h > h) ? (screen_h - h) / 2 : 0;
    widget_set_position(dialog, dlg_x, dlg_y);

    widget_add_child(parent, dialog);

    if (on_close != NULL) {
        char close_button_name[128];
        build_dialog_close_button_name(parent, close_button_name, sizeof(close_button_name));
        t_widget *btn_close = widget_create(BUTTON, w - 22, 6, 16, 14, close_button_name);
        btn_close->data.button.label = "x";
        btn_close->on_click = on_close;
        btn_close->flags |= WIDGET_FLAG_NO_FOCUS;
        btn_close->flags |= WIDGET_FLAG_NO_LAYOUT; // Add flag to skip layout
        dialog->data.dialog.close_button = btn_close;
        widget_add_child(dialog, btn_close);
    }

    return dialog;
}


