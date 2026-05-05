#ifndef LCOM_PROJECT_MENUS_H
#define LCOM_PROJECT_MENUS_H

#include "widget.h"
#include "state.h"

void gui_show_start_menu(t_state *gui);
void gui_show_name_menu(t_state *gui, bool is_multiplayer);
void init_game(t_state *gui);
void gui_show_pause_menu(t_state *gui);
void gui_show_scoreboard(t_state *gui);
void gui_show_info_dialog(t_state *gui, const char *title, const char *message);
void gui_show_confirm_dialog(t_state *gui, const char *title, const char *message, void (*on_yes)(t_widget*, void*), void (*on_no)(t_widget*, void*));

#endif /* LCOM_PROJECT_MENUS_H */
