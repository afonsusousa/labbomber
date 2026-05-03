#ifndef _GUI_H_
#define _GUI_H_

#include <stdint.h>
#include <stdbool.h>
#include "widget.h"

typedef struct {
    t_widget *current_menu;
    t_widget *start_menu;
    t_widget *single_name_menu;
    t_widget *multi_name_menu;
    t_widget *hovered_widget;

    // bool in_game;
} t_gui;

void      gui_init(t_gui *gui, uint32_t screen_width, uint32_t screen_height);
void      gui_destroy(t_gui *gui);

t_widget* widget_create_start_menu(t_gui *gui, uint32_t screen_width, uint32_t screen_height);
t_widget* widget_create_name_menu(t_gui *gui, uint32_t screen_width, uint32_t screen_height, bool is_multiplayer);

#endif /* _GUI_H_ */
