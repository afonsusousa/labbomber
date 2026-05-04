#include "views.h"
#include "../widget.h"
#include <stdio.h>

static void on_btn_singleplayer_click(t_widget *self, void *state) {
    (void)self;
    (void)state;

    t_widget *name_menu = gui_init_name_menu(g_gui.views.view_stack[0]->width, g_gui.views.view_stack[0]->height, false);
    gui_push_view(name_menu);
}

static void on_btn_multiplayer_click(t_widget *self, void *state) {
    (void)self;
    (void)state;

    t_widget *name_menu = gui_init_name_menu(g_gui.views.view_stack[0]->width, g_gui.views.view_stack[0]->height, true);
    gui_push_view(name_menu);
}

static void on_btn_scoreboard_click(t_widget *self, void *state) {
    (void)self;
    (void)state;
    printf("Scoreboard Button Clicked!\n");
}

t_widget* gui_init_start_menu(uint32_t screen_width, uint32_t screen_height) {
    t_widget *menu = widget_create(CANVAS, 0, 0, screen_width, screen_height);
    if (!menu) return NULL;

    t_widget *btn_single = widget_create(BUTTON, 0, 0, 300, 50);
    btn_single->data.button.label = "Singleplayer";
    btn_single->on_click = on_btn_singleplayer_click;
    widget_add_child(menu, btn_single);

    t_widget *btn_multi = widget_create(BUTTON, 0, 0, 300, 50);
    btn_multi->data.button.label = "Multiplayer";
    btn_multi->on_click = on_btn_multiplayer_click;
    widget_add_child(menu, btn_multi);

    t_widget *btn_score = widget_create(BUTTON, 0, 0, 300, 50);
    btn_score->data.button.label = "Scoreboard";
    btn_score->on_click = on_btn_scoreboard_click;
    widget_add_child(menu, btn_score);

    widget_layout(menu, 30, 100, true);

    return menu;
}
