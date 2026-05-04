#include "gui.h"

void trigger_pop_gui(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = (t_gui*)state;
    gui_pop_view(gui);
}

void trigger_focus_self(t_widget *self, void *state) {
    t_gui *gui = (t_gui*)state;
    gui_set_focus(gui, self);
}
