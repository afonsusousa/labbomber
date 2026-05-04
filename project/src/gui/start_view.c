#include "views.h"
#include "../widget.h"
#include <stdio.h>

// Forward declaration for layout function in gui.c
void widget_layout(t_widget *container, uint32_t spacing, uint32_t padding, bool is_vertical);

static void on_btn_singleplayer_click(t_widget *self) {
    (void)self;
    gui_set_view(g_gui.views.single_name_menu);
}

static void on_btn_multiplayer_click(t_widget *self) {
    (void)self;
    gui_set_view(g_gui.views.multi_name_menu);
}

static void on_btn_scoreboard_click(t_widget *self) {
    (void)self;
    printf("Scoreboard Button Clicked!\n");
}

void gui_init_start_menu(uint32_t screen_width, uint32_t screen_height) {
    t_widget *menu = widget_create(CANVAS, 0, 0, screen_width, screen_height);
    if (!menu) return;

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

    menu->on_quit = NULL; // Special case for start menu
    g_gui.views.start_menu = menu;
}
