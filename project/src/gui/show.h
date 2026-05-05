#ifndef LCOM_PROJECT_GUI_SHOW_H
#define LCOM_PROJECT_GUI_SHOW_H

#include "gui.h"

void gui_show_start_menu(t_gui *gui);
void gui_show_name_menu(t_gui *gui, bool is_multiplayer);
void gui_show_game_view(t_gui *gui);
void gui_show_pause_menu(t_gui *gui);
void gui_show_scoreboard(t_gui *gui);
void gui_show_info_dialog(t_gui *gui, const char *title, const char *message);
void gui_show_confirm_dialog(t_gui *gui, const char *title, const char *message, void (*on_yes)(t_widget*, void*), void (*on_no)(t_widget*, void*));

#endif /* LCOM_PROJECT_GUI_SHOW_H */
