#include "widget.h"
#include "gui/gui.h"
#include "../draw.h"

static void on_dialog_press(t_widget *self, void *state) {
    t_gui *gui = (t_gui*)state;
    int32_t abs_y = get_abs_y(self);
    if (gui->input.mouse_y >= abs_y && gui->input.mouse_y < abs_y + 24) {
        gui->drag.dragged_widget = self;
        gui->drag.drag_offset_x = gui->input.mouse_x - get_abs_x(self);
        gui->drag.drag_offset_y = gui->input.mouse_y - abs_y;
    }
}

static void on_dialog_drag(t_widget *self, void *state) {
    t_gui *gui = (t_gui*)state;
    int32_t new_x = gui->input.mouse_x - gui->drag.drag_offset_x;
    int32_t new_y = gui->input.mouse_y - gui->drag.drag_offset_y;
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

t_widget* widget_add_dialog(t_widget *parent, const char *title, uint32_t w, uint32_t h, uint32_t screen_w, uint32_t screen_h, void (*on_close)(t_widget*, void*)) {
    t_widget *dialog = widget_create(DIALOG, 0, 0, w, h);
    dialog->data.dialog.title = (char*)title;

    // Default dialog dragging behavior
    dialog->on_press = on_dialog_press;
    dialog->on_drag = on_dialog_drag;

    // Center it
    uint32_t dlg_x = (screen_w > w) ? (screen_w - w) / 2 : 0;
    uint32_t dlg_y = (screen_h > h) ? (screen_h - h) / 2 : 0;
    widget_set_position(dialog, dlg_x, dlg_y);

    widget_add_child(parent, dialog);

    if (on_close != NULL) {
        t_widget *btn_close = widget_create(BUTTON, w - 22, 6, 16, 14);
        btn_close->data.button.label = "x";
        btn_close->on_click = on_close;
        btn_close->flags |= WIDGET_FLAG_NO_LAYOUT; // Add flag to skip layout
        dialog->data.dialog.close_button = btn_close;
        widget_add_child(dialog, btn_close);
    }

    return dialog;
}

