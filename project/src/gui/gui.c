#include "gui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

t_gui g_gui;

void gui_set_focus(t_widget *widget) {
    if (g_gui.input.focused != widget) {
        if (g_gui.input.focused != NULL) {
            WIDGET_SET_FOCUSED(g_gui.input.focused, false);
            // Only update previous_focus if the currently focused widget is still active
            if (WIDGET_IS_ACTIVE(g_gui.input.focused)) {
                g_gui.input.previous_focus = g_gui.input.focused;
            }
        }
        g_gui.input.focused = widget;
        if (g_gui.input.focused != NULL) {
            WIDGET_SET_FOCUSED(g_gui.input.focused, true);
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

void gui_set_active(t_widget *widget, bool active) {
    if (widget == NULL) return;

    WIDGET_SET_ACTIVE(widget, active);

    if (!active) {
        // Robust cleanup: if any global input state belongs to this widget or its children, clear it.
        if (is_descendant(widget, g_gui.input.hovered)) g_gui.input.hovered = NULL;
        if (is_descendant(widget, g_gui.input.clicked_widget)) g_gui.input.clicked_widget = NULL;
        if (is_descendant(widget, g_gui.drag.dragged_widget)) g_gui.drag.dragged_widget = NULL;

        if (is_descendant(widget, g_gui.input.previous_focus)) g_gui.input.previous_focus = NULL;

        if (is_descendant(widget, g_gui.input.focused)) {
            t_widget *prev = g_gui.input.previous_focus;

            // Clear focus directly to avoid triggering a previous_focus save of this hidden widget
            if (g_gui.input.focused) {
                WIDGET_SET_FOCUSED(g_gui.input.focused, false);
                g_gui.input.focused = NULL;
            }

            // Restore focus to the previous element if it's still visible
            if (prev != NULL && WIDGET_IS_ACTIVE(prev)) {
                gui_set_focus(prev);
            }
        }
    }
}

void gui_set_view(t_widget *view) {
    g_gui.views.current_view = view;
    if (g_gui.views.current_view) {
        gui_set_focus(widget_find_first_focusable(g_gui.views.current_view));
    }
}

void on_dialog_press(t_widget *self) {
    int32_t abs_y = widget_get_abs_y(self);
    if (g_gui.input.mouse_y >= abs_y && g_gui.input.mouse_y < abs_y + 24) {
        g_gui.drag.dragged_widget = self;
        g_gui.drag.drag_offset_x = g_gui.input.mouse_x - widget_get_abs_x(self);
        g_gui.drag.drag_offset_y = g_gui.input.mouse_y - abs_y;
    }
}

void on_dialog_drag(t_widget *self) {
    int32_t new_x = g_gui.input.mouse_x - g_gui.drag.drag_offset_x;
    int32_t new_y = g_gui.input.mouse_y - g_gui.drag.drag_offset_y;
    widget_set_position(self, new_x, new_y);
}

void gui_init(uint32_t screen_width, uint32_t screen_height) {
    memset(&g_gui, 0, sizeof(g_gui));

    gui_init_start_menu(screen_width, screen_height);
    gui_init_name_menu(screen_width, screen_height, false);
    gui_init_name_menu(screen_width, screen_height, true);
    gui_init_game_view(screen_width, screen_height);

    gui_set_view(g_gui.views.start_menu);
}

void gui_destroy(void) {
    widget_destroy(g_gui.views.start_menu);
    widget_destroy(g_gui.views.single_name_menu);
    widget_destroy(g_gui.views.multi_name_menu);

    if (g_gui.views.game_view) {
        if (g_gui.views.game_view->children && g_gui.views.game_view->children->next) {
            t_widget *game_canvas = g_gui.views.game_view->children->next;
            if (game_canvas->data.canvas.state) {
                t_game_state *game_state = (t_game_state*)game_canvas->data.canvas.state;
                free(game_state->pixel_buffer);
                free(game_state);
            }
        }
        widget_destroy(g_gui.views.game_view);
    }

    memset(&g_gui, 0, sizeof(g_gui));
}

void widget_layout(t_widget *container, uint32_t spacing, uint32_t padding, bool is_vertical) {
    if (!container || !is_vertical) return;

    uint32_t total_children_h = 0;
    uint32_t child_count = 0;
    t_widget *child = container->children;
    while (child) {
        total_children_h += child->height;
        child_count++;
        child = child->next;
    }
    if (child_count > 1)
        total_children_h += (child_count - 1) * spacing;

    uint32_t current_y = (container->height > total_children_h) ? (container->height - total_children_h) / 2 : padding;

    child = container->children;
    while (child) {
        uint32_t child_x = (container->width > child->width) ? (container->width - child->width) / 2 : 0;
        widget_set_position(child, child_x, current_y);
        current_y += child->height + spacing;
        child = child->next;
    }
}
