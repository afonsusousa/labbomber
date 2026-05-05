#include "widget.h"
#include "../draw.h"

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
