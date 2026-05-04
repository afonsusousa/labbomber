#include "widget.h"
#include "../draw.h"

void draw_text(t_widget *self, hw_video_t *video) {
    hw_vbe_draw_rect(video, widget_get_abs_x(self), widget_get_abs_y(self), self->width, self->height, W95_LIGHT_GRAY);
    if (self->data.text_display.text != NULL) {
        draw_string(video, self->data.text_display.text, widget_get_abs_x(self) + 4, widget_get_abs_y(self) + 4, W95_LIGHT_GRAY);
    }
}

t_widget* widget_add_text(t_widget *parent, int32_t x, int32_t y, uint32_t w, uint32_t h, const char *text) {
    t_widget *txt = widget_create(TEXT, x, y, w, h);
    txt->data.text_display.text = (char*)text;
    widget_add_child(parent, txt);
    return txt;
}
