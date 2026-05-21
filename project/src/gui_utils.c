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
        WIDGET_SET_FOCUSED(gui->input.focused, false);
        gui->input.focused = NULL;
    }

    if (widget_is_descendant_of(gui->input.previous_focus, root)) {
        gui->input.previous_focus = NULL;
    }

    if (widget_is_descendant_of(gui->input.clicked_widget, root)) {
        WIDGET_SET_CLICKED(gui->input.clicked_widget, false);
        gui->input.clicked_widget = NULL;
    }

    if (widget_is_descendant_of(gui->input.hovered, root)) {
        WIDGET_SET_HOVERED(gui->input.hovered, false);
        gui->input.hovered = NULL;
    }

    if (widget_is_descendant_of(gui->drag.dragged_widget, root)) {
        gui_end_drag(gui);
    }
}

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

void gui_handle_tab_navigation(t_gui *gui, bool shift_down) {
    if (gui == NULL) return;

    t_widget *top_view = gui_get_top_view(gui);
    if (top_view == NULL) return;

    if (gui->input.focused != NULL) {
        if (gui->input.focused == top_view) {
            t_widget *first = widget_first_focusable(top_view);
            if (first != NULL) {
                gui_set_focus(gui, first);
                first->focus_cue = 1;
            }
            return;
        }

        if (gui->input.focused->focus_cue == 0) {
            gui->input.focused->focus_cue = 1;
            return;
        }

        t_widget *next_focus = shift_down 
            ? widget_get_prev_focusable_sibling(gui->input.focused)
            : widget_get_next_focusable_sibling(gui->input.focused);

        if (next_focus != NULL) {
            gui->input.focused->focus_cue = (gui->input.focused->focus_cue == 2) ? 2 : 0;
            gui_set_focus(gui, next_focus);
            next_focus->focus_cue = 1; 
        }
    }
    else {
        t_widget *first = widget_first_focusable(top_view);
        if (first != NULL) {
            gui_set_focus(gui, first);
            first->focus_cue = 1;
        }
    }
}

void gui_push_view(t_gui *gui, t_widget *view) {
    if (gui->views.view_count >= MAX_VIEWS) return;

    gui->show_focus_cues = false;
    gui_set_focus(gui, NULL);
    gui->views.view_stack[gui->views.view_count] = view;
    gui->views.is_overlay[gui->views.view_count] = false;
    gui->views.view_count++;

    gui_set_focus(gui, view);
}

void gui_push_overlay(t_gui *gui, t_widget *overlay) {
    if (gui->views.view_count >= MAX_VIEWS) return;

    gui->show_focus_cues = false;
    gui_set_focus(gui, NULL);
    gui->views.view_stack[gui->views.view_count] = overlay;
    gui->views.is_overlay[gui->views.view_count] = true;
    gui->views.view_count++;

    gui_set_focus(gui, overlay);
}

void gui_pop_view(t_gui *gui) {
    if (gui->views.view_count <= 0) return;

    gui->views.view_count--;
    t_widget *popped = gui->views.view_stack[gui->views.view_count];

    gui_clear_widget_refs(gui, popped);
    widget_destroy(popped);

    if (gui->views.view_count > 0) {
        t_widget *new_top = gui->views.view_stack[gui->views.view_count - 1];
        gui_set_focus(gui, new_top);
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
