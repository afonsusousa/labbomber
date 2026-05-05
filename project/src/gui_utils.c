#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "widget.h"
#include "gui.h"
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

void gui_push_view(t_gui *gui, t_widget *view) {
    if (gui->views.view_count >= MAX_VIEWS) return;

    gui_set_focus(gui, NULL);
    gui->views.view_stack[gui->views.view_count] = view;
    gui->views.is_overlay[gui->views.view_count] = false;
    gui->views.view_count++;

    gui_set_focus(gui, widget_first_focusable(view));
}

void gui_push_overlay(t_gui *gui, t_widget *overlay) {
    if (gui->views.view_count >= MAX_VIEWS) return;

    gui_set_focus(gui, NULL);
    gui->views.view_stack[gui->views.view_count] = overlay;
    gui->views.is_overlay[gui->views.view_count] = true;
    gui->views.view_count++;

    gui_set_focus(gui, widget_first_focusable(overlay));
}

void gui_pop_view(t_gui *gui) {
    if (gui->views.view_count <= 0) return;

    gui->views.view_count--;
    t_widget *popped = gui->views.view_stack[gui->views.view_count];
    widget_destroy(popped);

    if (gui->views.view_count > 0) {
        t_widget *new_top = gui->views.view_stack[gui->views.view_count - 1];
        gui_set_focus(gui, widget_first_focusable(new_top));
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
