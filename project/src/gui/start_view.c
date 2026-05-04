#include "views.h"
#include "../widgets/widget.h"
#include <stdio.h>

static void on_btn_singleplayer_click(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = (t_gui*)state;
    t_widget *name_menu = gui_init_name_menu(gui, gui->views.view_stack[0]->width, gui->views.view_stack[0]->height, false);
    gui_push_view(gui, name_menu);
}

static void on_btn_multiplayer_click(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = (t_gui*)state;
    t_widget *name_menu = gui_init_name_menu(gui, gui->views.view_stack[0]->width, gui->views.view_stack[0]->height, true);
    gui_push_view(gui, name_menu);
}

static void on_btn_scoreboard_click(t_widget *self, void *state) {
    (void)self;
    (void)state;
    printf("Scoreboard Button Clicked!\n");
}

t_widget* gui_init_start_menu(t_gui *gui, uint32_t screen_width, uint32_t screen_height) {
    (void)gui;
    t_widget *menu = widget_create(CANVAS, 0, 0, screen_width, screen_height);
    if (!menu) return NULL;

    widget_add_button(menu, 0, 0, 300, 50, "Singleplayer", on_btn_singleplayer_click);
    widget_add_button(menu, 0, 0, 300, 50, "Multiplayer", on_btn_multiplayer_click);
    widget_add_button(menu, 0, 0, 300, 50, "Scoreboard", on_btn_scoreboard_click);

    widget_layout(menu, 30, 100, true);

    return menu;
}
