#include "views.h"
#include "string.h"
#include "stdlib.h"
#include "../widgets/widget.h"

static void on_btn_start_game_click(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = (t_gui*)state;

    // Pop the name menu
    gui_pop_view(gui);

    // Push the game view
    t_widget *game_view = gui_init_game_view(gui, gui->views.view_stack[0]->width, gui->views.view_stack[0]->height);
    gui_push_view(gui, game_view);
}

static void on_text_input_click(t_widget *self, void *state) {
    t_gui *gui = (t_gui*)state;
    gui_set_focus(gui, self);
}

static void on_view_quit(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = (t_gui*)state;
    gui_pop_view(gui);
}

static void on_dialog_close_click(t_widget *self, void *state) {
    t_gui *gui = (t_gui*)state;
    if (self == NULL || self->parent == NULL || self->parent->parent == NULL)
        return;

    t_widget *view = self->parent->parent;
    if (view->on_quit != NULL) {
        view->on_quit(view, gui);
    }
}

t_widget* gui_init_name_menu(t_gui *gui, uint32_t screen_width, uint32_t screen_height, bool is_multiplayer) {
    (void)gui;
    t_widget *overlay = widget_create_overlay(screen_width, screen_height, on_view_quit);
    if (!overlay) return NULL;

    WIDGET_SET_ACTIVE(overlay, true);

    const char *title = is_multiplayer ? "Enter Player Names" : "Enter Player Name";
    t_widget *dlg_prompt = widget_add_dialog(overlay, title, 400, 300, screen_width, screen_height, on_dialog_close_click);

    widget_add_text_input(dlg_prompt, 0, 0, 300, 40, "Player 1", on_text_input_click);

    if (is_multiplayer) {
        widget_add_text_input(dlg_prompt, 0, 0, 300, 40, "Player 2", on_text_input_click);
    }

    widget_add_button(dlg_prompt, 0, 0, 150, 40, "Start", on_btn_start_game_click);

    widget_layout(dlg_prompt, 20, 40, true);

    return overlay;
}
