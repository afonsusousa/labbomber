#include "widget.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static uint32_t get_abs_x(t_widget *self) {
    if (!self) return 0;
    uint32_t abs_x = self->x;
    t_widget *curr = self->parent;
    while (curr != NULL) {
        abs_x += curr->x;
        curr = curr->parent;
    }
    return abs_x;
}

static uint32_t get_abs_y(t_widget *self) {
    if (!self) return 0;
    uint32_t abs_y = self->y;
    t_widget *curr = self->parent;
    while (curr != NULL) {
        abs_y += curr->y;
        curr = curr->parent;
    }
    return abs_y;
}

t_widget* widget_get_at(t_widget *root, uint32_t x, uint32_t y) {
    if (root == NULL || !root->active)
        return NULL;

    uint32_t abs_x = get_abs_x(root);
    uint32_t abs_y = get_abs_y(root);

    bool is_inside = (x >= abs_x && x < (abs_x + root->width) &&
                      y >= abs_y && y < (abs_y + root->height));

    if (!is_inside)
        return NULL;

    t_widget *hit_child = NULL;
    t_widget *child = root->children;
    
    while (child != NULL) {
        t_widget *hit = widget_get_at(child, x, y);
        if (hit != NULL)
            hit_child = hit;
        child = child->next;
    }

    if (hit_child != NULL)
        return hit_child;

    return root;
}

t_widget* widget_create(e_widget_type type, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
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
    widget->active = true;
    widget->is_clicked = false;
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

void widget_destroy(t_widget *widget) {
    if (widget == NULL) return;

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

void widget_hide(t_widget *widget) {
    if (widget == NULL) return;

    widget->active = false;

    t_widget *child = widget->children;
    while (child != NULL) {
        widget_hide(child);
        child = child->next;
    }
}

void draw_canvas(t_widget *self, hw_video_t *video) {
    if (!self || !self->active) return;
    // Example background color
    hw_vbe_draw_rect(video, get_abs_x(self), get_abs_y(self), self->width, self->height, 0x111111);
}

void draw_button(t_widget *self, hw_video_t *video) {
    if (!self || !self->active) return;
    uint32_t color = self->hovered ? 0x888888 : 0x444444;
    hw_vbe_draw_rect(video, get_abs_x(self), get_abs_y(self), self->width, self->height, color);
}

void draw_text(t_widget *self, hw_video_t *video) {
    if (!self || !self->active) return;
    // Placeholder since there is no string drawing yet
    hw_vbe_draw_rect(video, get_abs_x(self), get_abs_y(self), self->width, self->height, 0xDDDDDD);
}

void draw_text_input(t_widget *self, hw_video_t *video) {
    if (!self || !self->active) return;
    // White if active/clicked, gray if not
    uint32_t color = self->is_clicked ? 0xFFFFFF : 0xCCCCCC;
    hw_vbe_draw_rect(video, get_abs_x(self), get_abs_y(self), self->width, self->height, color);
}

void draw_dialog(t_widget *self, hw_video_t *video) {
    if (!self || !self->active) return;
    hw_vbe_draw_rect(video, get_abs_x(self), get_abs_y(self), self->width, self->height, 0x222222);
}

void widget_draw(t_widget *widget, hw_video_t *video) {
    if (widget == NULL || !widget->active) return;

    if (widget->draw != NULL) {
        widget->draw(widget, video);
    }

    t_widget *child = widget->children;
    while (child != NULL) {
        widget_draw(child, video);
        child = child->next;
    }
}
