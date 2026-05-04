#include "widget.h"
#include "draw.h"
#include "gui/gui.h" // For t_game_state
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Win95 16-bit RGB 5:6:5 Color Palette
#define W95_TEAL       0x0410 
#define W95_GRAY       0xC618 
#define W95_LIGHT_GRAY 0xDEFB 
#define W95_DARK_GRAY  0x8410 
#define W95_WHITE      0xFFFF 
#define W95_BLACK      0x0000 

widget_draw_func default_draw_funcs[] = {
    [CANVAS]     = draw_canvas,
    [BUTTON]     = draw_button,
    [TEXT]       = draw_text,
    [TEXT_INPUT] = draw_text_input,
    [DIALOG]     = draw_dialog
};

static int32_t get_abs_x(t_widget *self) {
    if (!self) return 0;
    if (self->parent == NULL) return self->x;
    return get_abs_x(self->parent) + self->x;
}

static int32_t get_abs_y(t_widget *self) {
    if (!self) return 0;
    if (self->parent == NULL) return self->y;
    return get_abs_y(self->parent) + self->y;
}

int32_t widget_get_abs_x(t_widget *widget) {
    return get_abs_x(widget);
}

int32_t widget_get_abs_y(t_widget *widget) {
    return get_abs_y(widget);
}

t_widget* widget_get_at(t_widget *root, int32_t x, int32_t y) {
    if (root == NULL || !WIDGET_IS_ACTIVE(root)) {
        return NULL;
    }

    t_widget *child = root->children;
    t_widget *last_child = NULL;
    while(child) {
        last_child = child;
        child = child->next;
    }

    while(last_child) {
        t_widget *hit = widget_get_at(last_child, x, y);
        if (hit) return hit;
        last_child = last_child->prev;
    }

    int32_t abs_x = get_abs_x(root);
    int32_t abs_y = get_abs_y(root);
    bool is_inside = (x >= abs_x && x < (abs_x + (int32_t)root->width) &&
                      y >= abs_y && y < (abs_y + (int32_t)root->height));

    return is_inside ? root : NULL;
}

t_widget* widget_create(e_widget_type type, int32_t x, int32_t y, uint32_t width, uint32_t height) {
    t_widget *widget = (t_widget*)malloc(sizeof(t_widget));
    if (widget == NULL) {
        return NULL;
    }

    memset(widget, 0, sizeof(t_widget));

    widget->type = type;
    widget->x = x;
    widget->y = y;
    widget->width = width;
    widget->height = height;
    widget->flags = 0;
    WIDGET_SET_ACTIVE(widget, true);
    widget->h_align = ALIGN_START;
    widget->v_align = ALIGN_START;
    widget->draw = default_draw_funcs[type];
    return widget;
}

void widget_add_child(t_widget *parent, t_widget *child) {
    if (parent == NULL || child == NULL) return;

    child->parent = parent;
    child->next = NULL;

    if (parent->children == NULL) {
        parent->children = child;
        child->prev = NULL;
    } else {
        t_widget *last = parent->children;
        while (last->next != NULL) {
            last = last->next;
        }
        last->next = child;
        child->prev = last;
    }
    child->next = NULL;
}

t_widget* widget_set_position(t_widget *widget, int32_t x, int32_t y) {
    if (widget == NULL)
        return NULL;

    widget->x = x;
    widget->y = y;
    return widget;
}

void widget_destroy(t_widget *widget) {
    if (widget == NULL) return;

    // Trigger lifecycle hook before destroying children
    if (widget->on_destroy != NULL) {
        widget->on_destroy(widget);
    }

    t_widget *child = widget->children;
    while (child != NULL) {
        t_widget *next_child = child->next;
        widget_destroy(child);
        child = next_child;
    }

    if (widget->parent != NULL) {
        if (widget->parent->children == widget)
            widget->parent->children = widget->next;
        if (widget->prev != NULL) 
            widget->prev->next = widget->next;
        if (widget->next != NULL)
            widget->next->prev = widget->prev;
    }
    free(widget);
}

t_widget* widget_find_first_focusable(t_widget *root) {
    if (root == NULL || !WIDGET_IS_ACTIVE(root))
        return NULL;

    if (WIDGET_CAN_RECEIVE_FOCUS(root))
        return root;

    t_widget *child = root->children;
    while (child != NULL) {
        t_widget *match = widget_find_first_focusable(child);
        if (match != NULL)
            return match;
        child = child->next;
    }

    return NULL;
}

static void draw_win95_border(hw_video_t *video, int32_t x, int32_t y, uint16_t w, uint16_t h, bool sunken) {
    uint32_t tl_outer = sunken ? W95_DARK_GRAY : W95_WHITE;
    uint32_t tl_inner = sunken ? W95_BLACK : W95_LIGHT_GRAY;
    uint32_t br_outer = sunken ? W95_WHITE : W95_BLACK;
    uint32_t br_inner = sunken ? W95_LIGHT_GRAY : W95_DARK_GRAY;

    // Outer Top & Left
    hw_vbe_draw_hline(video, x, y, w, tl_outer);
    hw_vbe_draw_vline(video, x, y, h, tl_outer);

    // Outer Bottom & Right
    hw_vbe_draw_hline(video, x, y + (int32_t)h - 1, w, br_outer);
    hw_vbe_draw_vline(video, x + (int32_t)w - 1, y, h, br_outer);

    // Inner Top & Left
    if (w > 2 && h > 2) {
        hw_vbe_draw_hline(video, x + 1, y + 1, (uint16_t)(w - 2), tl_inner);
        hw_vbe_draw_vline(video, x + 1, y + 1, (uint16_t)(h - 2), tl_inner);

        // Inner Bottom & Right
        hw_vbe_draw_hline(video, x + 1, y + (int32_t)h - 2, (uint16_t)(w - 2), br_inner);
        hw_vbe_draw_vline(video, x + (int32_t)w - 2, y + 1, (uint16_t)(h - 2), br_inner);
    }
}

void draw_canvas(t_widget *self, hw_video_t *video) {
    hw_vbe_draw_rect(video, get_abs_x(self), get_abs_y(self), self->width, self->height, W95_TEAL);
}

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

void draw_text(t_widget *self, hw_video_t *video) {
    hw_vbe_draw_rect(video, get_abs_x(self), get_abs_y(self), self->width, self->height, W95_LIGHT_GRAY);
    if (self->data.text_display.text != NULL) {
        draw_string(video, self->data.text_display.text, get_abs_x(self) + 4, get_abs_y(self) + 4, W95_LIGHT_GRAY);
    }
}

void draw_text_input(t_widget *self, hw_video_t *video) {
    uint32_t abs_x = get_abs_x(self);
    uint32_t abs_y = get_abs_y(self);
    
    uint32_t color = WIDGET_IS_CLICKED(self) ? W95_WHITE : W95_GRAY;
    hw_vbe_draw_rect(video, abs_x, abs_y, self->width, self->height, color);
    
    // text inputs are always sunken
    draw_win95_border(video, abs_x, abs_y, self->width, self->height, true);

    if (self->data.text_input.buffer != NULL) {
        draw_string(video, self->data.text_input.buffer, abs_x + 4, abs_y + (self->height - 11) / 2, color);
    }
}

void draw_dialog(t_widget *self, hw_video_t *video) {
    uint32_t abs_x = get_abs_x(self);
    uint32_t abs_y = get_abs_y(self);
    
    hw_vbe_draw_rect(video, abs_x, abs_y, self->width, self->height, W95_GRAY);
    
    // dialog windows are always raised
    draw_win95_border(video, abs_x, abs_y, self->width, self->height, false);

    if (self->data.dialog.title != NULL) {
        hw_vbe_draw_rect(video, abs_x + 4, abs_y + 4, self->width - 8, 20, 0x000080); // Title bar background
        draw_string(video, self->data.dialog.title, abs_x + 8, abs_y + 8, 0xFFFFFF);
    }
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

void widget_draw(t_widget *widget, hw_video_t *video) {
    if (widget == NULL || !WIDGET_IS_ACTIVE(widget)) return;

    if (widget->draw != NULL)
        widget->draw(widget, video);

    t_widget *child = widget->children;
    while (child != NULL) {
        widget_draw(child, video);
        child = child->next;
    }
}

void widget_tick(t_widget *widget) {
    if (widget == NULL || !WIDGET_IS_ACTIVE(widget)) return;

    if (widget->on_tick != NULL)
        widget->on_tick(widget, NULL);

    t_widget *child = widget->children;
    while (child != NULL) {
        widget_tick(child);
        child = child->next;
    }
}
