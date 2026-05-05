#include "widget.h"

t_widget* widget_create_overlay(uint32_t screen_w, uint32_t screen_h, void (*on_quit)(t_widget*, void*), const char *name) {
    t_widget *overlay = widget_create(CANVAS, 0, 0, screen_w, screen_h, name);
    overlay->on_quit = on_quit;
    return overlay;
}
