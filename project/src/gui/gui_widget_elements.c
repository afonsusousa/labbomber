#include "gui/widget.h"
#include "gui/gui.h"
#include "core/application.h"
#include "view/draw.h"
#include <stdio.h>
#include <string.h>

// Forward declarations for static callback functions
static void _callback_button_on_tick(t_widget *self, void *state);
static void _callback_button_internal_on_click(t_widget *self, void *state);
static void _callback_button_on_key_press(t_widget *self, uint8_t scancode, void *state);
static void _callback_dialog_on_press(t_widget *self, void *state);
static void _callback_dialog_on_drag(t_widget *self, void *state);

/*
--------------------------------------------------------------------------------
    SIMPLE TEXT DISPLAY
--------------------------------------------------------------------------------
*/

void draw_text(t_widget *self, hw_video_t *video, void *state) {
    (void)state;
    hw_vbe_draw_rect(video, self->abs_x, self->abs_y, self->width, self->height, UI_PANEL_COLOR);
    if (self->data.text_display.text != NULL) {
        draw_string(video, self->data.text_display.text, self->abs_x + 4, self->abs_y + 4, UI_PANEL_COLOR);
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

static void _callback_button_on_tick(t_widget *self, void *state) {
    if (self->data.button.action_delay_timer > 0) {
        self->data.button.action_delay_timer--;
        if (self->data.button.action_delay_timer == 0) {
            if (self->data.button.on_click_action != NULL) {
                self->data.button.on_click_action(self, state);
            }
        }
    }
}

static void _callback_button_internal_on_click(t_widget *self, void *state) {
    self->data.button.action_delay_timer = 5;
}

static void _callback_button_on_key_press(t_widget *self, uint8_t scancode, void *state) {
    t_gui *gui = GUI(state);
    if (scancode == 0x1C) {
        WIDGET_SET_CLICKED(gui, self);
    } 
    else if (scancode == 0x9C) {
        if (WIDGET_IS_CLICKED(gui, self)) {
            WIDGET_SET_CLICKED(gui, NULL);
            _callback_button_internal_on_click(self, state);
        }
    }
}

static void draw_rounded_rect(hw_video_t *video, int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t r, uint32_t color) {
    if (w < 2*r || h < 2*r) r = (w<h ? w : h) / 2;

    hw_vbe_draw_rect(video, x+r, y, w-2 *r, h, color);
    hw_vbe_draw_rect(video, x, y+r, r, h-2 *r, color);
    hw_vbe_draw_rect(video, x+w-r, y+r, r, h-2 *r, color);

    for (uint32_t dy = 0; dy < r; dy++) {
        for (uint32_t dx = 0; dx < r; dx++) {
            int32_t cx = r-dx-1, cy = r-dy-1;

            if (cx * cx + cy * cy <= (int32_t)(r * r)) {
                hw_vbe_draw_pixel(video, x+dx, y+dy, color);
                hw_vbe_draw_pixel(video, x+w-1-dx, y+dy, color);
                hw_vbe_draw_pixel(video, x + dx, y+h-1-dy, color);
                hw_vbe_draw_pixel(video, x+w-1-dx, y+h-1-dy, color);
            }
        }
    }
}

void draw_button(t_widget *self, hw_video_t *video, void *state) {
    t_gui *gui = GUI(state);
    int32_t abs_x = self->abs_x;
    int32_t abs_y = self->abs_y;

    bool pressed = WIDGET_IS_CLICKED(gui, self);
    bool hovered = WIDGET_IS_HOVERED(gui, self);
    bool focused = WIDGET_IS_FOCUSED(gui, self);

    uint32_t fill = pressed ? BTN_FILL_PRESS : hovered ? BTN_FILL_HOVER : BTN_FILL;

    bool in_dialog = (self->parent != NULL && self->parent->type == DIALOG);
    uint32_t border_color;

    if (focused || hovered) {
        border_color = in_dialog ? 0x000000 : 0xFFFFFF;
        draw_rounded_rect(video, abs_x - 2, abs_y - 2, self->width + 4, self->height + 4, BTN_RADIUS, border_color);
        draw_rounded_rect(video, abs_x, abs_y, self->width, self->height, BTN_RADIUS, fill);
    } 
    else {
        border_color = in_dialog ? 0xFFFFFF : 0x000000;
        draw_rounded_rect(video, abs_x, abs_y, self->width, self->height, BTN_RADIUS + 2, border_color);
        draw_rounded_rect(video, abs_x + 1, abs_y + 1, self->width - 2, self->height - 2, BTN_RADIUS, fill);
    }

    if (self->data.button.label != NULL) {
        int text_w = strlen(self->data.button.label) * 11;
        int text_x = abs_x + ((int32_t)self->width - text_w) / 2;
        int text_y = abs_y + ((int32_t)self->height - 11) / 2;
        if (pressed) { text_x += 1; text_y += 1; }
        draw_string(video, self->data.button.label, text_x, text_y, fill);
    }
}

t_widget* widget_add_button(t_widget *parent, int32_t x, int32_t y, uint32_t w, uint32_t h, const char *label, void (*on_click)(t_widget*, void*), const char *name) {
    t_widget *btn = widget_create(BUTTON, x, y, w, h, name);
    
    btn->data.button.label = (char*)label;
    btn->data.button.action_delay_timer = 0;
    btn->data.button.on_click_action = on_click;
    
    btn->on_click = _callback_button_internal_on_click;
    btn->on_key_press = _callback_button_on_key_press;
    btn->on_tick = _callback_button_on_tick;
    
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
    hw_vbe_draw_rect(video, self->abs_x, self->abs_y, self->width, self->height, UI_BG_COLOR);
}

/* 
--------------------------------------------------------------------------------  
    DIALOGS
--------------------------------------------------------------------------------
*/

static void _callback_dialog_on_press(t_widget *self, void *state) {
    t_gui *gui = GUI(state);
    int32_t abs_y = self->abs_y;
    if (gui->input.mouse_y >= abs_y && gui->input.mouse_y < abs_y + 24) {
        gui_begin_drag(gui, self, gui->input.mouse_x, gui->input.mouse_y);
    }
}

static void _callback_dialog_on_drag(t_widget *self, void *state) {
    t_gui *gui = GUI(state);
    if (gui->drag.dragged_widget != self)
        return;

    int32_t new_x = gui->input.mouse_x - gui->drag.drag_offset_x;
    int32_t new_y = gui->input.mouse_y - gui->drag.drag_offset_y;
    widget_set_position(self, new_x, new_y);
}

static void build_dialog_close_button_name(const t_widget *parent, char *buffer, size_t buffer_size) {
    const char *parent_name = (parent != NULL && parent->name != NULL) ? parent->name : "dialog";
    snprintf(buffer, buffer_size, "%s_close_button", parent_name);
}

void draw_dialog(t_widget *self, hw_video_t *video, void *state) {
    (void)state;
    uint32_t abs_x = self->abs_x;
    uint32_t abs_y = self->abs_y;
    
    draw_rounded_rect(video, abs_x - 3, abs_y - 3, self->width + 6, self->height + 6, 21, 0x000000);
    draw_rounded_rect(video, abs_x, abs_y, self->width, self->height, 18, UI_PANEL_FLASH);

    if (self->data.dialog.title != NULL) {
        draw_rounded_rect(video, abs_x + DIALOG_TITLE_X_OFFSET, abs_y + DIALOG_TITLE_Y_OFFSET, self->width - (DIALOG_TITLE_X_OFFSET * 2), DIALOG_TITLE_HEIGHT, 8, UI_TITLE_BAR_COLOR);
        draw_string(video, self->data.dialog.title, abs_x + 8, abs_y + DIALOG_TITLE_Y_OFFSET + (DIALOG_TITLE_HEIGHT - 11) / 2, UI_TEXT_COLOR);
    }
}

t_widget* widget_create_overlay(uint32_t screen_w, uint32_t screen_h, void (*on_quit)(t_widget*, void*), const char *name) {
    t_widget *overlay = widget_create(OVERLAY, 0, 0, screen_w, screen_h, name);
    overlay->on_quit = on_quit;
    return overlay;
}

t_widget* widget_add_dialog(t_widget *parent, const char *title, uint32_t w, uint32_t h, uint32_t screen_w, uint32_t screen_h, void (*on_close)(t_widget*, void*), const char *name) {
    t_widget *dialog = widget_create(DIALOG, 0, 0, w, h, name);
    dialog->data.dialog.title = (char*)title;

    // Default dialog dragging behavior
    dialog->on_press = _callback_dialog_on_press;
    dialog->on_drag = _callback_dialog_on_drag;

    uint32_t dlg_x = (screen_w > w) ? (screen_w - w) / 2 : 0;
    uint32_t dlg_y = (screen_h > h) ? (screen_h - h) / 2 : 0;
    widget_set_position(dialog, dlg_x, dlg_y);

    widget_add_child(parent, dialog);

    if (on_close != NULL) {
        char close_button_name[128];
        build_dialog_close_button_name(parent, close_button_name, sizeof(close_button_name));
        t_widget *btn_close = widget_create(BUTTON, w - 22, DIALOG_TITLE_Y_OFFSET + (DIALOG_TITLE_HEIGHT - 14) / 2, 16, 14, close_button_name);
        btn_close->data.button.label = "x";
        btn_close->on_click = on_close;
        btn_close->flags |= WIDGET_FLAG_NO_FOCUS;
        btn_close->flags |= WIDGET_FLAG_NO_LAYOUT; // Add flag to skip layout
        dialog->data.dialog.close_button = btn_close;
        widget_add_child(dialog, btn_close);
    }

    return dialog;
}
