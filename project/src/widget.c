#include "widget.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

t_widget* widget_get_at(t_widget *root, uint32_t x, uint32_t y) {
    if (root == NULL || !root->active)
        return NULL;

    bool is_inside = (x >= root->x && x < (root->x + root->width) &&
                      y >= root->y && y < (root->y + root->height));

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
