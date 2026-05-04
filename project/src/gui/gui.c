#include "gui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void gui_set_focus(t_gui *gui, t_widget *widget) {
    if (gui->input.focused != widget) {
        if (gui->input.focused != NULL) {
            WIDGET_SET_FOCUSED(gui->input.focused, false);
            if (WIDGET_IS_ACTIVE(gui->input.focused)) {
                gui->input.previous_focus = gui->input.focused;
            }
        }
        gui->input.focused = widget;
        if (gui->input.focused != NULL) {
            WIDGET_SET_FOCUSED(gui->input.focused, true);
        }
    }
}

static bool is_descendant(t_widget *ancestor, t_widget *widget) {
    while (widget != NULL) {
        if (widget == ancestor) return true;
        widget = widget->parent;
    }
    return false;
}

void gui_set_active(t_gui *gui, t_widget *widget, bool active) {
    if (widget == NULL) return;

    WIDGET_SET_ACTIVE(widget, active);

    if (!active) {
        if (is_descendant(widget, gui->input.hovered)) {
            WIDGET_SET_HOVERED(gui->input.hovered, false);
            gui->input.hovered = NULL;
        }
        if (is_descendant(widget, gui->input.clicked_widget)) {
            WIDGET_SET_CLICKED(gui->input.clicked_widget, false);
            gui->input.clicked_widget = NULL;
        }
        if (is_descendant(widget, gui->drag.dragged_widget)) {
            gui->drag.dragged_widget = NULL;
        }

        if (is_descendant(widget, gui->input.previous_focus)) {
            gui->input.previous_focus = NULL;
        }

        if (is_descendant(widget, gui->input.focused)) {
            t_widget *prev = gui->input.previous_focus;

            if (gui->input.focused) {
                WIDGET_SET_FOCUSED(gui->input.focused, false);
                gui->input.focused = NULL;
            }

            if (prev != NULL && WIDGET_IS_ACTIVE(prev)) {
                gui_set_focus(gui, prev);
            }
        }
    }
}

void gui_push_view(t_gui *gui, t_widget *view) {
    if (gui->views.view_count >= MAX_VIEWS) return;

    gui_set_focus(gui, NULL);
    gui->views.view_stack[gui->views.view_count] = view;
    gui->views.is_overlay[gui->views.view_count] = false;
    gui->views.view_count++;

    gui_set_focus(gui, widget_find_first_focusable(view));
}

void gui_push_overlay(t_gui *gui, t_widget *overlay) {
    if (gui->views.view_count >= MAX_VIEWS) return;

    gui_set_focus(gui, NULL);
    gui->views.view_stack[gui->views.view_count] = overlay;
    gui->views.is_overlay[gui->views.view_count] = true;
    gui->views.view_count++;

    gui_set_focus(gui, widget_find_first_focusable(overlay));
}

void gui_pop_view(t_gui *gui) {
    if (gui->views.view_count <= 0) return;

    gui->views.view_count--;
    t_widget *popped = gui->views.view_stack[gui->views.view_count];
    widget_destroy(popped);

    if (gui->views.view_count > 0) {
        t_widget *new_top = gui->views.view_stack[gui->views.view_count - 1];
        gui_set_focus(gui, widget_find_first_focusable(new_top));
    }
}

t_widget* gui_get_top_view(t_gui *gui) {
    if (gui->views.view_count == 0) return NULL;
    return gui->views.view_stack[gui->views.view_count - 1];
}

void on_dialog_press(t_widget *self, void *state) {
    t_gui *gui = (t_gui*)state;
    int32_t abs_y = widget_get_abs_y(self);
    if (gui->input.mouse_y >= abs_y && gui->input.mouse_y < abs_y + 24) {
        gui->drag.dragged_widget = self;
        gui->drag.drag_offset_x = gui->input.mouse_x - widget_get_abs_x(self);
        gui->drag.drag_offset_y = gui->input.mouse_y - abs_y;
    }
}

void on_dialog_drag(t_widget *self, void *state) {
    t_gui *gui = (t_gui*)state;
    int32_t new_x = gui->input.mouse_x - gui->drag.drag_offset_x;
    int32_t new_y = gui->input.mouse_y - gui->drag.drag_offset_y;
    widget_set_position(self, new_x, new_y);
}

void gui_init(t_gui *gui, uint32_t screen_width, uint32_t screen_height) {
    memset(gui, 0, sizeof(*gui));

    t_widget *start = gui_init_start_menu(gui, screen_width, screen_height);
    gui_push_view(gui, start);
}

void gui_destroy(t_gui *gui) {
    while (gui->views.view_count > 0) {
        gui_pop_view(gui);
    }
}

void widget_layout(t_widget *container, uint32_t spacing, uint32_t padding, bool is_vertical) {
    if (!container || !is_vertical) return;

    uint32_t total_children_h = 0;
    uint32_t child_count = 0;
    t_widget *child = container->children;
    while (child) {
        if (!WIDGET_HAS_FLAG(child, WIDGET_FLAG_NO_LAYOUT)) {
            total_children_h += child->height;
            child_count++;
        }
        child = child->next;
    }
    if (child_count > 1)
        total_children_h += (child_count - 1) * spacing;

    uint32_t current_y = (container->height > total_children_h) ? (container->height - total_children_h) / 2 : padding;

    child = container->children;
    while (child) {
        if (!WIDGET_HAS_FLAG(child, WIDGET_FLAG_NO_LAYOUT)) {
            uint32_t child_x = (container->width > child->width) ? (container->width - child->width) / 2 : 0;
            widget_set_position(child, child_x, current_y);
            current_y += child->height + spacing;
        }
        child = child->next;
    }
}
