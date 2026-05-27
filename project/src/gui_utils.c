#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "widget.h"
#include "gui.h"

static bool widget_is_descendant_of(t_widget *widget, t_widget *ancestor) {
    while (widget != NULL) {
        if (widget == ancestor) {
            return true;
        }
        widget = widget->parent;
    }

    return false;
}

static void gui_clear_widget_refs(t_gui *gui, t_widget *root) {
    if (gui == NULL || root == NULL) return;

    if (widget_is_descendant_of(gui->input.focused, root)) {
        WIDGET_SET_FOCUSED(gui, NULL);
    }

    if (widget_is_descendant_of(gui->input.clicked_widget, root)) {
        WIDGET_SET_CLICKED(gui, NULL);
    }

    if (widget_is_descendant_of(gui->input.hovered, root)) {
        WIDGET_SET_HOVERED(gui, NULL);
    }

    if (widget_is_descendant_of(gui->drag.dragged_widget, root)) {
        gui_end_drag(gui);
    }
}

static t_widget* gui_get_initial_focus(t_widget *root);

void gui_begin_drag(t_gui *gui, t_widget *widget, int32_t mouse_x, int32_t mouse_y) {
    if (gui == NULL || widget == NULL) return;

    gui->drag.dragged_widget = widget;
    gui->drag.drag_offset_x = mouse_x - widget->abs_x;
    gui->drag.drag_offset_y = mouse_y - widget->abs_y;
}

void gui_end_drag(t_gui *gui) {
    if (gui == NULL) return;

    gui->drag.dragged_widget = NULL;
    gui->drag.drag_offset_x = 0;
    gui->drag.drag_offset_y = 0;
}

t_widget* gui_pop_until_widget_found(t_gui *gui, const char *widget_name) {
    if (gui == NULL || widget_name == NULL) return NULL;

    while (gui->views.view_count > 0) {
        t_widget *top_view = gui->views.view_stack[gui->views.view_count - 1];
        t_widget *found = widget_find_by_name(top_view, widget_name);

        if (found != NULL) {
            return found;
        }

        gui_pop_view(gui);
    }

    return NULL;
}

void gui_set_focus(t_gui *gui, t_widget *widget) {
    if (gui == NULL || WIDGET_IS_FOCUSED(gui, widget)) return;

    WIDGET_SET_FOCUSED(gui, widget);
}

static t_widget* gui_get_initial_focus(t_widget *root) {
    if (root == NULL) return NULL;

    t_widget *focusable = widget_first_focusable(root);
    return focusable != NULL ? focusable : root;
}

void gui_handle_tab_navigation(t_gui *gui, bool shift_down) {
    if (gui == NULL) return;

    t_widget *top_view = gui_get_top_view(gui);
    if (top_view == NULL) return;

    if (gui->input.focused == NULL) {
        gui_set_focus(gui, gui_get_initial_focus(top_view));
        return;
    }

    t_widget *next_focus = shift_down
        ? widget_get_prev_focusable_sibling(gui->input.focused)
        : widget_get_next_focusable_sibling(gui->input.focused);

    if (next_focus == NULL) {
        next_focus = gui_get_initial_focus(top_view);
    }

    gui_set_focus(gui, next_focus);
}

void gui_push_view(t_gui *gui, t_widget *view) {
    if (gui->views.view_count >= MAX_VIEWS) return;

    gui_set_focus(gui, NULL);
    gui->views.view_stack[gui->views.view_count] = view;
    gui->views.is_overlay[gui->views.view_count] = false;
    gui->views.view_count++;

    gui_set_focus(gui, gui_get_initial_focus(view));
}

void gui_push_overlay(t_gui *gui, t_widget *overlay) {
    if (gui->views.view_count >= MAX_VIEWS) return;

    gui_set_focus(gui, NULL);
    gui->views.view_stack[gui->views.view_count] = overlay;
    gui->views.is_overlay[gui->views.view_count] = true;
    gui->views.view_count++;

    gui_set_focus(gui, gui_get_initial_focus(overlay));
}

void gui_pop_view(t_gui *gui) {
    if (gui->views.view_count <= 0) return;

    gui->views.view_count--;
    t_widget *popped = gui->views.view_stack[gui->views.view_count];

    gui_clear_widget_refs(gui, popped);
    widget_destroy(popped);

    if (gui->views.view_count > 0) {
        t_widget *new_top = gui->views.view_stack[gui->views.view_count - 1];
        gui_set_focus(gui, gui_get_initial_focus(new_top));
    }
}

t_widget* gui_get_top_view(t_gui *gui) {
    if (gui->views.view_count == 0) return NULL;
    return gui->views.view_stack[gui->views.view_count - 1];
}

void gui_destroy(t_gui *gui) {
    while (gui->views.view_count > 0) {
        gui_pop_view(gui);
    }
}

int gui_get_curr_time(t_gui *gui, hw_rtc_t *out) {
    if (out == NULL) return 1;

    hw_rtc_t tmp;
    hw_rtc_t *source = &tmp;

    if (gui != NULL && gui->rtc != NULL) {
        source = gui->rtc;
    }

    int ret = hw_rtc_get_time(source);
    if (ret != 0) return ret;

    *out = *source;
    return 0;
}
