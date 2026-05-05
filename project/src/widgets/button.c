#include "widget.h"
#include "../draw.h"
#include <string.h>

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
