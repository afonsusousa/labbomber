#include "widget.h"
#include "../gui/gui.h"

#include <string.h>

void draw_canvas(t_widget *self, hw_video_t *video) {
    hw_vbe_draw_rect(video, get_abs_x(self), get_abs_y(self), self->width, self->height, W95_TEAL);
}

void draw_game_canvas(t_widget *self, hw_video_t *video) {
    t_game_state *game = (t_game_state*)self->data.canvas.state;
    int32_t abs_x = get_abs_x(self);
    int32_t abs_y = get_abs_y(self);

    uint8_t *src_buf = (uint8_t*)game->pixel_buffer;
    uint8_t *dest_buf = (uint8_t*)video->double_buffer + (abs_y * video->screen_width + abs_x) * video->bytes_per_pixel;

    for (uint32_t y = 0; y < game->height; ++y) {
        memcpy(dest_buf, src_buf, game->width * video->bytes_per_pixel);
        src_buf += game->width * video->bytes_per_pixel;
        dest_buf += video->screen_width * video->bytes_per_pixel;
    }
}

t_widget* widget_create_overlay(uint32_t screen_w, uint32_t screen_h, void (*on_quit)(t_widget*, void*), const char *name) {
    t_widget *overlay = widget_create(CANVAS, 0, 0, screen_w, screen_h, name);
    overlay->draw = NULL; // Transparent overlay
    overlay->on_quit = on_quit;
    return overlay;
}
