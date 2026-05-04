#ifndef LCOM_PROJECT_GAME_VIEW_H
#define LCOM_PROJECT_GAME_VIEW_H

#include "gui.h"

t_widget* gui_init_start_menu(uint32_t screen_width, uint32_t screen_height);
t_widget* gui_init_name_menu(uint32_t screen_width, uint32_t screen_height, bool is_multiplayer);
t_widget* gui_init_game_view(uint32_t screen_width, uint32_t screen_height);
t_widget* gui_init_pause_menu(uint32_t screen_width, uint32_t screen_height);
t_widget* gui_init_confirm_menu(uint32_t screen_width, uint32_t screen_height, const char *title, const char *message, void (*on_yes)(t_widget*, void*), void (*on_no)(t_widget*, void*));

#endif /* LCOM_PROJECT_GAME_VIEW_H */
