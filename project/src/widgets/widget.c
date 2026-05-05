#include "widget.h"
#include "../draw.h"
#include "../gui/gui.h" // For t_game_state
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

widget_draw_func default_draw_funcs[] = {
    [CANVAS]     = draw_canvas,
    [BUTTON]     = draw_button,
    [TEXT]       = draw_text,
    [TEXT_INPUT] = draw_text_input,
    [DIALOG]     = draw_dialog,
    [OVERLAY]    = NULL
};

int32_t get_abs_x(t_widget *self) {
    if (!self) return 0;
    if (self->parent == NULL) return self->x;
    return get_abs_x(self->parent) + self->x;
}

int32_t get_abs_y(t_widget *self) {
    if (!self) return 0;
    if (self->parent == NULL) return self->y;
    return get_abs_y(self->parent) + self->y;
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

t_widget* widget_create(e_widget_type type, int32_t x, int32_t y, uint32_t width, uint32_t height, const char *name) {
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
    widget->name = NULL;
    if (name != NULL) {
        widget->name = (char*)malloc(strlen(name) + 1);
        if (widget->name != NULL) {
            strcpy(widget->name, name);
        }
    }
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

t_widget* widget_find_by_name(t_widget *root, const char *name) {
    if (root == NULL || name == NULL)
        return NULL;

    if (root->name != NULL && strcmp(root->name, name) == 0) {
        return root;
    }

    t_widget *child = root->children;
    while (child != NULL) {
        t_widget *match = widget_find_by_name(child, name);
        if (match != NULL)
            return match;
        child = child->next;
    }

    return NULL;
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

    if (widget->name != NULL) {
        free(widget->name);
    }

    free(widget);
}

t_widget* widget_first_focusable(t_widget *root) {
    if (root == NULL || !WIDGET_IS_ACTIVE(root))
        return NULL;

    if (WIDGET_CAN_RECEIVE_FOCUS(root))
        return root;

    t_widget *child = root->children;
    while (child != NULL) {
        t_widget *match = widget_first_focusable(child);
        if (match != NULL)
            return match;
        child = child->next;
    }

    return NULL;
}

void draw_win95_border(hw_video_t *video, int32_t x, int32_t y, uint16_t w, uint16_t h, bool sunken) {
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

void widget_tick(t_widget *widget, void *state) {
    if (widget == NULL || !WIDGET_IS_ACTIVE(widget)) return;

    if (widget->on_tick != NULL)
        widget->on_tick(widget, state);

    t_widget *child = widget->children;
    while (child != NULL) {
        widget_tick(child, state);
        child = child->next;
    }
}

