#include "gui.h"
#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define isspace(c) ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r' || (c) == '\f' || (c) == '\v')

t_widget* widget_create_overlay(uint32_t screen_w, uint32_t screen_h, void (*on_quit)(t_widget*, void*), const char *name) {
    t_widget *overlay = widget_create(OVERLAY, 0, 0, screen_w, screen_h, name);
    overlay->on_quit = on_quit;
    return overlay;
}

// ENTRY POINT

void gui_init(t_gui *gui, uint32_t screen_width, uint32_t screen_height) {
    memset(gui, 0, sizeof(*gui));
    gui->width = screen_width;
    gui->height = screen_height;

    gui_show_start_menu(gui);
    gui->is_running = true;
}

// =============================================================================
// Generic Callbacks
// =============================================================================

void pop_the_view(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = (t_gui*)state;
    gui_pop_view(gui);
}

void focus_self(t_widget *self, void *state) {
    t_gui *gui = (t_gui*)state;
    gui_set_focus(gui, self);
}

static bool is_blank_string(const char *s) {
    if (s == NULL) return true;
    for (const char *p = s; *p != '\0'; ++p) {
        if (!isspace((unsigned char)*p)) {
            return false;
        }
    }
    return true;
}

// =============================================================================
// Shared Dialogs
// =============================================================================

    // -------------------------------------------------------------------------
    // Shared Dialog Displays
    // -------------------------------------------------------------------------

void gui_show_info_dialog(t_gui *gui, const char *title, const char *message) {
    t_widget *overlay = widget_create_overlay(gui->width, gui->height, pop_the_view, "info_overlay");
    if (overlay == NULL) return;

    t_widget *info_dialog = widget_add_dialog(overlay, title, 360, 190, gui->width, gui->height, pop_the_view, "info_dialog");
    info_dialog->on_quit = pop_the_view;

    widget_add_text(info_dialog, 0, 40, 320, 24, message, "info_message");

    t_widget *btn_ok = widget_add_button(info_dialog, 0, 86, 120, 36, "OK", pop_the_view, "info_ok_button");

    int32_t btn_row_x = ((int32_t)info_dialog->width - (int32_t)btn_ok->width) / 2;
    widget_set_position(btn_ok, btn_row_x, 120);

    gui_push_overlay(gui, overlay);
}

void gui_show_confirm_dialog(t_gui *gui, const char *title, const char *message, void (*on_yes)(t_widget*, void*), void (*on_no)(t_widget*, void*)) {
    t_widget *overlay = widget_create_overlay(gui->width, gui->height, pop_the_view, "confirm_overlay");
    if (overlay == NULL) return;

    t_widget *confirm_dialog = widget_add_dialog(overlay, title, 360, 190, gui->width, gui->height, on_no, "confirm_dialog");
    confirm_dialog->on_quit = pop_the_view;

    widget_add_text(confirm_dialog, 0, 40, 320, 24, message, "confirm_message");

    t_widget *btn_yes = widget_add_button(confirm_dialog, 0, 86, 120, 36, "Yes", on_yes, "confirm_yes_button");
    t_widget *btn_no = widget_add_button(confirm_dialog, 0, 86, 120, 36, "No", on_no, "confirm_no_button");

    int32_t btn_row_x = ((int32_t)confirm_dialog->width - (int32_t)(btn_yes->width + btn_no->width + 20)) / 2;
    widget_set_position(btn_yes, btn_row_x, 120);
    widget_set_position(btn_no, btn_row_x + (int32_t)btn_yes->width + 20, 120);

    gui_push_overlay(gui, overlay);
}

// =============================================================================
// Start Menu
// =============================================================================

    // -------------------------------------------------------------------------
    // Start Menu Callbacks
    // -------------------------------------------------------------------------

static void on_quit_confirm(t_widget *self, void *state) {
    t_gui *gui = (t_gui *)state;
    (void)self;
    gui->is_running = false;
}

static void on_quit(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = (t_gui *)state;
    gui_show_confirm_dialog(gui, "QUIT", "DO YOU REALLY WANT TO QUIT", on_quit_confirm, pop_the_view);
}

static void on_btn_singleplayer_click(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = (t_gui *)state;
    gui_show_name_menu(gui, false);
}

static void on_btn_multiplayer_click(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = (t_gui *)state;
    gui_show_name_menu(gui, true);
}

static void on_btn_scoreboard_click(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = (t_gui *)state;
    gui_show_scoreboard(gui);
}

    // -------------------------------------------------------------------------
    // Start Menu Displays
    // -------------------------------------------------------------------------

void gui_show_start_menu(t_gui *gui) {
    t_widget *menu = widget_create(CANVAS, 0, 0, gui->width, gui->height, "start_menu_view");
    if (menu == NULL) return;

    widget_add_button(menu, 0, 0, 300, 50, "Singleplayer", on_btn_singleplayer_click, "start_single_button");
    widget_add_button(menu, 0, 0, 300, 50, "Multiplayer", on_btn_multiplayer_click, "start_multi_button");
    widget_add_button(menu, 0, 0, 300, 50, "Scoreboard", on_btn_scoreboard_click, "start_scoreboard_button");
    menu->on_quit = on_quit;

    widget_layout(menu, 30, 100, true);
    gui_push_view(gui, menu);
}

// =============================================================================
// Name Menu
// =============================================================================

    // -------------------------------------------------------------------------
    // Name Menu Callbacks
    // -------------------------------------------------------------------------

static void on_btn_start_game_click(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = (t_gui*)state;

    t_widget *top_view = gui_get_top_view(gui);
    t_widget *player1_input = widget_find_by_name(top_view, "player1_input");
    t_widget *player2_input = widget_find_by_name(top_view, "player2_input");

    if (player1_input == NULL || player1_input->data.text_input.buffer == NULL) {
        gui_show_info_dialog(gui, "Invalid Name", "Please enter Player 1 name");
        return;
    }

    bool player1_empty = is_blank_string(player1_input->data.text_input.buffer);
    if (player1_empty) {
        gui_show_info_dialog(gui, "Invalid Name", "Please enter Player 1 name");
        return;
    }

    if (player2_input != NULL) {
        if (player2_input->data.text_input.buffer == NULL) {
            gui_show_info_dialog(gui, "Invalid Name", "Please enter Player 2 name");
            return;
        }

        bool player2_empty = is_blank_string(player2_input->data.text_input.buffer);
        if (player2_empty) {
            gui_show_info_dialog(gui, "Invalid Name", "Please enter Player 2 name");
            return;
        }
    }

    gui_pop_view(gui);
    init_game(gui);
}

    // -------------------------------------------------------------------------
    // Name Menu Displays
    // -------------------------------------------------------------------------

void gui_show_name_menu(t_gui *gui, bool is_multiplayer) {
    t_widget *overlay = widget_create_overlay(gui->width, gui->height, pop_the_view, "name_overlay");
    if (overlay == NULL) return;

    WIDGET_SET_ACTIVE(overlay, true);

    const char *title = is_multiplayer ? "Enter Player Names" : "Enter Player Name";
    t_widget *dlg_prompt = widget_add_dialog(overlay, title, 400, 300, gui->width, gui->height, pop_the_view, "name_dialog");

    widget_add_text_input(dlg_prompt, 0, 0, 300, 40, "Player 1", focus_self, "player1_input");

    if (is_multiplayer) {
        widget_add_text_input(dlg_prompt, 0, 0, 300, 40, "Player 2", focus_self, "player2_input");
    }

    widget_add_button(dlg_prompt, 0, 0, 150, 40, "Start", on_btn_start_game_click, "start_game_button");

    widget_layout(dlg_prompt, 20, 40, true);
    gui_push_overlay(gui, overlay);
}

// =============================================================================
// Pause Menu
// =============================================================================

    // -------------------------------------------------------------------------
    // Pause Menu Callbacks
    // -------------------------------------------------------------------------

static void on_pause_main_menu_confirm_click(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = (t_gui*)state;
    gui_pop_until_widget_found(gui, "start_menu_view");
}

static void on_pause_reset_confirm_click(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = (t_gui*)state;
    gui_reset_game(gui);
}

static void on_pause_reset_click(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = (t_gui*)state;
    gui_show_confirm_dialog(gui, "Confirm Reset", "Are you sure?", on_pause_reset_confirm_click, pop_the_view);
}

static void on_pause_main_menu_click(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = (t_gui*)state;
    gui_show_confirm_dialog(gui, "Confirm Main Menu", "Return to main menu?", on_pause_main_menu_confirm_click, pop_the_view);
}

    // -------------------------------------------------------------------------
    // Pause Menu Displays
    // -------------------------------------------------------------------------

void gui_show_pause_menu(t_gui *gui) {
    t_widget *overlay = widget_create_overlay(gui->width, gui->height, pop_the_view, "pause_overlay");
    if (overlay == NULL) return;

    t_widget *pause_dialog = widget_add_dialog(overlay, "Paused", 360, 260, gui->width, gui->height, pop_the_view, "pause_dialog");
    pause_dialog->on_quit = pop_the_view;

    widget_add_button(pause_dialog, 0, 0, 220, 40, "Resume", pop_the_view, "pause_resume_button");
    widget_add_button(pause_dialog, 0, 0, 220, 40, "Reset", on_pause_reset_click, "pause_reset_button");
    widget_add_button(pause_dialog, 0, 0, 220, 40, "Main Menu", on_pause_main_menu_click, "pause_menu_button");

    widget_layout(pause_dialog, 16, 48, true);
    gui_push_overlay(gui, overlay);
}

static void on_scoreboard_close(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = (t_gui*)state;
    gui_pop_view(gui);
}

// =============================================================================
// Scoreboard
// =============================================================================

    // -------------------------------------------------------------------------
    // Scoreboard Callbacks
    // -------------------------------------------------------------------------
        //defaults
    // -------------------------------------------------------------------------
    // Scoreboard Display
    // -------------------------------------------------------------------------

void gui_show_scoreboard(t_gui *gui) {
    t_widget *overlay = widget_create_overlay(gui->width, gui->height, on_scoreboard_close, "scoreboard_overlay");
    if (overlay == NULL) return;

    t_widget *scoreboard = widget_add_dialog(overlay, "Scoreboard", 500, 400, gui->width, gui->height, pop_the_view, "scoreboard_dialog");
    scoreboard->on_quit = on_scoreboard_close;

    widget_add_text(scoreboard, 0, 30, 450, 24, "Top Players:", "scoreboard_title");
    widget_add_text(scoreboard, 0, 60, 450, 20, "1. Player One    - 15000 pts", "scoreboard_row_1");
    widget_add_text(scoreboard, 0, 85, 450, 20, "2. Player Two    - 12500 pts", "scoreboard_row_2");
    widget_add_text(scoreboard, 0, 110, 450, 20, "3. Player Three  - 10000 pts", "scoreboard_row_3");
    widget_add_text(scoreboard, 0, 135, 450, 20, "4. Player Four   -  8500 pts", "scoreboard_row_4");
    widget_add_text(scoreboard, 0, 160, 450, 20, "5. Player Five   -  7000 pts", "scoreboard_row_5");

    widget_add_button(scoreboard, 0, 0, 150, 40, "Close", on_scoreboard_close, "scoreboard_close_button");

    widget_layout(scoreboard, 16, 48, true);
    gui_push_overlay(gui, overlay);
}
