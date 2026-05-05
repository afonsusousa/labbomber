#include "widget.h"
#include "gui.h"
#include "draw.h"
#include <string.h>

/*
--------------------------------------------------------------------------------
    SIMPLE TEXT DISPLAY
--------------------------------------------------------------------------------
*/

void draw_text(t_widget *self, hw_video_t *video) {
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

void draw_button(t_widget *self, hw_video_t *video) {
    uint32_t abs_x = get_abs_x(self);
    uint32_t abs_y = get_abs_y(self);
    
    hw_vbe_draw_rect(video, abs_x, abs_y, self->width, self->height, W95_GRAY);
    
    // 3D border (sunken if clicked and raised if not)
    bool is_sunken = WIDGET_IS_CLICKED(self); 
    draw_win95_border(video, abs_x, abs_y, self->width, self->height, is_sunken);

    if (self->data.button.label != NULL) {
        // scaled font is 11px wide
        int text_w = strlen(self->data.button.label) * 11;
        int text_x = abs_x + (self->width - text_w) / 2;
        int text_y = abs_y + (self->height - 11) / 2;
        draw_string(video, self->data.button.label, text_x, text_y, W95_GRAY);
    }
}

t_widget* widget_add_button(t_widget *parent, int32_t x, int32_t y, uint32_t w, uint32_t h, const char *label, void (*on_click)(t_widget*, void*), const char *name) {
    t_widget *btn = widget_create(BUTTON, x, y, w, h, name);
    btn->data.button.label = (char*)label;
    btn->on_click = on_click;
    widget_add_child(parent, btn);
    return btn;
}

/*
--------------------------------------------------------------------------------
    SIMPLE CANVAS
--------------------------------------------------------------------------------
*/

void draw_canvas(t_widget *self, hw_video_t *video) {
    hw_vbe_draw_rect(video, get_abs_x(self), get_abs_y(self), self->width, self->height, W95_TEAL);
}

/* 
--------------------------------------------------------------------------------  
    DIALOGS
--------------------------------------------------------------------------------
*/

static void on_dialog_press(t_widget *self, void *state) {
    t_gui *gui = (t_gui*)state;
    int32_t abs_y = get_abs_y(self);
    if (gui->input.mouse_y >= abs_y && gui->input.mouse_y < abs_y + 24) {
        gui->drag.dragged_widget = self;
        gui->drag.dragt_dx = gui->input.mouse_x - get_abs_x(self);
        gui->drag.dragt_dy = gui->input.mouse_y - abs_y;
    }
}

static void on_dialog_drag(t_widget *self, void *state) {
    t_gui *gui = (t_gui*)state;
    int32_t new_x = gui->input.mouse_x - gui->drag.dragt_dx;
    int32_t new_y = gui->input.mouse_y - gui->drag.dragt_dy;
    widget_set_position(self, new_x, new_y);
}

void draw_dialog(t_widget *self, hw_video_t *video) {
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
        t_widget *btn_close = widget_create(BUTTON, w - 22, 6, 16, 14, "dialog_close_button");
        btn_close->data.button.label = "x";
        btn_close->on_click = on_close;
        btn_close->flags |= WIDGET_FLAG_NO_LAYOUT; // Add flag to skip layout
        dialog->data.dialog.close_button = btn_close;
        widget_add_child(dialog, btn_close);
    }

    return dialog;
}


