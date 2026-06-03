#include "gui.h"
#include "assets_cache.h"
#include <lcom/xpm.h>
#include <lcom/lcf.h>
#include "game.h"
#include "application.h"
#include "event_handlers.h"
#include "../lib/serialPort/serial_port.h"
#include "../lib/serialPort/i8250.h"
#include "../lib/utils/utils.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define isspace(c) ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r' || (c) == '\f' || (c) == '\v')

// Forward declarations for static callback functions
static void _callback_pop_view(t_widget *self, void *state);
static void _callback_focus_self(t_widget *self, void *state);
static void _callback_confirm_quit(t_widget *self, void *state);
static void _callback_quit(t_widget *self, void *state);
static void _callback_show_singleplayer_name_menu(t_widget *self, void *state);
static void _callback_show_multiplayer_name_menu(t_widget *self, void *state);
static void _callback_show_scoreboard(t_widget *self, void *state);
static void _callback_start_game(t_widget *self, void *state);
static void _callback_resume_game(t_widget *self, void *state);
static void _callback_confirm_return_to_main_menu(t_widget *self, void *state);
static void _callback_confirm_reset_game(t_widget *self, void *state);
static void _callback_reset_game(t_widget *self, void *state);
static void _callback_return_to_main_menu(t_widget *self, void *state);
static void _callback_close_scoreboard(t_widget *self, void *state);

t_widget* widget_create_overlay(uint32_t screen_w, uint32_t screen_h, void (*on_quit)(t_widget*, void*), const char *name) {
    t_widget *overlay = widget_create(OVERLAY, 0, 0, screen_w, screen_h, name);
    overlay->on_quit = on_quit;
    return overlay;
}

// ENTRY POINT

void gui_init(struct s_ctx *ctx, uint32_t screen_width, uint32_t screen_height) {
    memset(&ctx->gui, 0, sizeof(ctx->gui));
    ctx->gui.width = screen_width;
    ctx->gui.height = screen_height;

    gui_show_start_menu(ctx);
    ctx->gui.is_running = true;
}

// =============================================================================
// Generic Callbacks
// =============================================================================

static void _callback_pop_view(t_widget *self, void *state) {
    (void)self;
    gui_pop_view(GUI(state));
}

static void _callback_focus_self(t_widget *self, void *state) {
    gui_set_focus(GUI(state), self);
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

void gui_show_info_dialog(struct s_ctx *ctx, const char *title, const char *message) {
    t_gui *gui = &ctx->gui;
    t_widget *overlay = widget_create_overlay(gui->width, gui->height, _callback_pop_view, "info_overlay");
    if (overlay == NULL) return;

    t_widget *info_dialog = widget_add_dialog(overlay, title, 360, 190, gui->width, gui->height, _callback_pop_view, "info_dialog");
    info_dialog->on_quit = _callback_pop_view;

    widget_add_text(info_dialog, 0, 40, 320, 24, message, "info_message");

    t_widget *btn_ok = widget_add_button(info_dialog, 0, 86, 120, 36, "OK", _callback_pop_view, "info_ok_button");

    int32_t btn_row_x = ((int32_t)info_dialog->width - (int32_t)btn_ok->width) / 2;
    widget_set_position(btn_ok, btn_row_x, 120);

    gui_push_overlay(gui, overlay);
}

void gui_show_game_end_dialog(struct s_ctx *ctx, const char *title, const char *message) {
    t_gui *gui = &ctx->gui;
    t_widget *overlay = widget_create_overlay(gui->width, gui->height, _callback_pop_view, "end_overlay");
    if (overlay == NULL) return;

    t_widget *dialog = widget_add_dialog(overlay, title, 360, 190, gui->width, gui->height, _callback_pop_view, "end_dialog");
    dialog->on_quit = _callback_pop_view;

    widget_add_text(dialog, 0, 40, 320, 24, message, "end_message");

    t_widget *btn_ok = widget_add_button(dialog, 0, 86, 120, 36, "OK", _callback_confirm_return_to_main_menu, "end_ok_button");

    int32_t btn_row_x = ((int32_t)dialog->width - (int32_t)btn_ok->width) / 2;
    widget_set_position(btn_ok, btn_row_x, 120);

    gui_push_overlay(gui, overlay);
}

void gui_show_confirm_dialog(struct s_ctx *ctx, const char *title, const char *message, void (*on_yes)(t_widget*, void*), void (*on_no)(t_widget*, void*)) {
    t_gui *gui = &ctx->gui;
    t_widget *overlay = widget_create_overlay(gui->width, gui->height, _callback_pop_view, "confirm_overlay");
    if (overlay == NULL) return;

    t_widget *confirm_dialog = widget_add_dialog(overlay, title, 360, 190, gui->width, gui->height, on_no, "confirm_dialog");
    confirm_dialog->on_quit = _callback_pop_view;

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

static void _callback_confirm_quit(t_widget *self, void *state) {
    t_gui *gui = GUI(state);
    (void)self;
    gui->is_running = false;
}

static void _callback_quit(t_widget *self, void *state) {
    (void)self;
    gui_show_confirm_dialog(CTX(state), "QUIT", "DO YOU REALLY WANT TO QUIT", _callback_confirm_quit, _callback_pop_view);
}

static void _callback_show_singleplayer_name_menu(t_widget *self, void *state) {
    (void)self;
    t_ctx *ctx = CTX(state);
    ctx->is_multiplayer = false;
    gui_show_name_menu(ctx, false);
}

static void _callback_show_multiplayer_name_menu(t_widget *self, void *state) {
    (void)self;
    t_ctx *ctx = CTX(state);

    ctx->is_multiplayer = true;
    ctx->multiplayer_partner_ready = false;
    ctx->multiplayer_signal_sent = false;
    ctx->multiplayer_role_assigned = false;
    ctx->multiplayer_local_player = PLAYER_1;
    ctx->multiplayer_remote_player = PLAYER_2;
    ctx->multiplayer_remote_nonce = 0;
    ctx->multiplayer_remote_tiebreaker = 0;
    ctx->multiplayer_rx_state = 0;
    ctx->multiplayer_rx_type = 0;
    ctx->multiplayer_rx_pos = 0;
    ctx->multiplayer_last_player_lives[0] = 0;
    ctx->multiplayer_last_player_lives[1] = 0;
    ctx->multiplayer_last_player_active[0] = false;
    ctx->multiplayer_last_player_active[1] = false;
    ctx->multiplayer_local_nonce =
        (uint16_t)((ctx->real_time.seconds * 251u) ^
                   (ctx->real_time.minutes * 61u) ^
                   (ctx->real_time.hours * 17u) ^
                   (ctx->real_time.day * 7u) ^
                   ((uint32_t)clock() & 0xFFFFu) ^
                   ((uintptr_t)ctx & 0xFFFFu));
    ctx->multiplayer_local_tiebreaker = (uint8_t)(((unsigned long long)ctx ^ (unsigned long long)ctx->multiplayer_local_nonce) & 0xFFu);
    
    FILE *log_file = fopen("/tmp/game_debug.log", "a");
    
    if (serial_init() != 0) {
        if (log_file) {
            fprintf(log_file, "[ERROR] serial_init() failed\n");
            fflush(log_file);
            fclose(log_file);
        }
        gui_show_info_dialog(ctx, "Serial Error", "Failed to initialize serial port");
        ctx->is_multiplayer = false;
        return;
    }
    serial_flush_rx();
    
    // Wait for the host pipe / TCP bridge to settle before sending the first handshake.
    for (int i = 0; i < 1000; i++) {
        uint8_t dummy;
        if (util_sys_inb(COM1_ADDR + SERP_LSR, &dummy) == 0) {
            if (log_file && (i % 200 == 0)) {
                fprintf(log_file, "[SERIAL] waiting for connection, LSR=0x%02X\n", dummy);
            }
        }
    }

    int initial_result = app_multiplayer_send_hello(ctx);
    if (log_file) {
        fprintf(log_file, "[SERIAL] initial handshake send result=%d\n", initial_result);
        fprintf(log_file, "[INFO] Serial initialized, handshake send initiated\n");
        fflush(log_file);
        fclose(log_file);
    }

    if (log_file) {
        fprintf(log_file, "[INFO] Serial initialized, handshake byte sent\n");
        fflush(log_file);
        fclose(log_file);
    }
    gui_show_name_menu(ctx, true);
}

static void _callback_show_scoreboard(t_widget *self, void *state) {
    (void)self;
    gui_show_scoreboard(CTX(state));
}

static void _draw_start_menu(t_widget *self, hw_video_t *video, void *state) {
    (void)state;
    if (sprites_initialized && sprite_cache[SPRITE_MENU_BACKGROUND].bytes != NULL) {
        xpm_image_t bg = sprite_cache[SPRITE_MENU_BACKGROUND];
        hw_vbe_draw_xpm(video, bg.bytes, bg,
            self->abs_x + (int32_t)(bg.width / 2),
            self->abs_y + (int32_t)(bg.height / 2));
    } else {
        hw_vbe_draw_rect(video, self->abs_x, self->abs_y, self->width, self->height, UI_BG_COLOR);
    }
}

void gui_show_start_menu(struct s_ctx *ctx) {
    t_gui *gui = &ctx->gui;
    t_widget *menu = widget_create(CANVAS, 0, 0, gui->width, gui->height, "start_menu_view");
    if (menu == NULL) return;

    menu->draw = _draw_start_menu;

    widget_add_button(menu, 0, 0, 300, 50, "Singleplayer", _callback_show_singleplayer_name_menu, "start_single_button");
    widget_add_button(menu, 0, 0, 300, 50, "Multiplayer", _callback_show_multiplayer_name_menu, "start_multi_button");
    widget_add_button(menu, 0, 0, 300, 50, "Scoreboard", _callback_show_scoreboard, "start_scoreboard_button");
    widget_add_button(menu, 0, 0, 300, 50, "QUIT", _callback_quit, "start_scoreboard_button");
    menu->on_quit = _callback_quit;

    widget_layout(menu, 24, 80, true);
    gui_push_view(gui, menu);
}

// =============================================================================
// Name Menu
// =============================================================================

static void _callback_start_game(t_widget *self, void *state) {
    (void)self;
    t_ctx *ctx = CTX(state);
    t_gui *gui = GUI(state);

    if (ctx->is_multiplayer && !ctx->multiplayer_role_assigned) {
        gui_show_info_dialog(ctx, "Waiting", "Waiting for multiplayer handshake...");
        return;
    }

    t_widget *player1_input = widget_find_by_name(gui, "player1_input");
    t_widget *player2_input = widget_find_by_name(gui, "player2_input");

    if (player1_input == NULL || player1_input->data.text_input.buffer == NULL) {
        gui_show_info_dialog(CTX(state), "Invalid Name", "Please enter Player 1 name");
        return;
    }

    bool player1_empty = is_blank_string(player1_input->data.text_input.buffer);
    if (player1_empty) {
        gui_show_info_dialog(CTX(state), "Invalid Name", "Please enter Player 1 name");
        return;
    }

    if (player2_input != NULL) {
        if (player2_input->data.text_input.buffer == NULL) {
            gui_show_info_dialog(CTX(state), "Invalid Name", "Please enter Player 2 name");
            return;
        }

        bool player2_empty = is_blank_string(player2_input->data.text_input.buffer);
        if (player2_empty) {
            gui_show_info_dialog(CTX(state), "Invalid Name", "Please enter Player 2 name");
            return;
        }
    }

    gui_pop_view(gui);
    gui_show_game_view(CTX(state));
}

void gui_show_name_menu(struct s_ctx *ctx, bool is_multiplayer) {
    t_gui *gui = &ctx->gui;
    t_widget *overlay = widget_create_overlay(gui->width, gui->height, _callback_pop_view, "name_overlay");
    if (overlay == NULL) return;

    WIDGET_SET_ACTIVE(overlay, true);

    const char *title = is_multiplayer ? "Enter Player Names" : "Enter Player Name";
    t_widget *dlg_prompt = widget_add_dialog(overlay, title, 400, 300, gui->width, gui->height, _callback_pop_view, "name_dialog");

    widget_add_text_input(dlg_prompt, 0, 0, 300, 40, "Player 1", _callback_focus_self, "player1_input");

    if (is_multiplayer) {
        widget_add_text_input(dlg_prompt, 0, 0, 300, 40, "Player 2", _callback_focus_self, "player2_input");
    }

    widget_add_button(dlg_prompt, 0, 0, 150, 40, "Start", _callback_start_game, "start_game_button");

    widget_layout(dlg_prompt, 16, 48, true);
    gui_push_overlay(gui, overlay);
}

// =============================================================================
// Pause Menu
// =============================================================================

static void _callback_resume_game(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = GUI(state);
    GAME(state)->is_paused = false;
    gui_pop_view(gui);
}

static void _callback_confirm_return_to_main_menu(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = GUI(state);
    GAME(state)->is_paused = false;
    gui_pop_until_widget_found(gui, "start_menu_view");
}

static void _callback_confirm_reset_game(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = GUI(state);
    game_state_reset(GAME(state), CTX(state)->real_time, CTX(state)->is_multiplayer);
    GAME(state)->is_paused = false;
    gui_pop_until_widget_found(gui, "game_view");
}

static void _callback_reset_game(t_widget *self, void *state) {
    (void)self;
    gui_show_confirm_dialog(CTX(state), "Confirm Reset", "Are you sure?", _callback_confirm_reset_game, _callback_pop_view);
}

static void _callback_return_to_main_menu(t_widget *self, void *state) {
    (void)self;
    gui_show_confirm_dialog(CTX(state), "Confirm Main Menu", "Return to main menu?", _callback_confirm_return_to_main_menu, _callback_pop_view);
}

void gui_show_pause_menu(struct s_ctx *ctx) {
    t_gui *gui = &ctx->gui;
    
    t_widget *overlay = widget_create_overlay(gui->width, gui->height, _callback_resume_game, "pause_overlay");
    if (overlay == NULL) return;

    t_widget *pause_dialog = widget_add_dialog(overlay, "Paused", 360, 260, gui->width, gui->height, _callback_resume_game, "pause_dialog");
    
    pause_dialog->on_quit = _callback_resume_game;

    widget_add_button(pause_dialog, 0, 0, 220, 40, "Resume", _callback_resume_game, "pause_resume_button");
    widget_add_button(pause_dialog, 0, 0, 220, 40, "Reset", _callback_reset_game, "pause_reset_button");
    widget_add_button(pause_dialog, 0, 0, 220, 40, "Main Menu", _callback_return_to_main_menu, "pause_menu_button");

    widget_layout(pause_dialog, 12, 32, true);
    gui_push_overlay(gui, overlay);
}

// =============================================================================
// Scoreboard
// =============================================================================

static void _callback_close_scoreboard(t_widget *self, void *state) {
    (void)self;
    gui_pop_view(GUI(state));
}

void gui_show_scoreboard(struct s_ctx *ctx) {
    t_gui *gui = &ctx->gui;
    t_widget *overlay = widget_create_overlay(gui->width, gui->height, _callback_close_scoreboard, "scoreboard_overlay");
    if (overlay == NULL) return;

    t_widget *scoreboard = widget_add_dialog(overlay, "Scoreboard", 500, 400, gui->width, gui->height, _callback_pop_view, "scoreboard_dialog");
    scoreboard->on_quit = _callback_close_scoreboard;

    widget_add_text(scoreboard, 0, 30, 450, 24, "Top Players:", "scoreboard_title");
    widget_add_text(scoreboard, 0, 60, 450, 20, "1. Player One    - 15000 pts", "scoreboard_row_1");
    widget_add_text(scoreboard, 0, 85, 450, 20, "2. Player Two    - 12500 pts", "scoreboard_row_2");
    widget_add_text(scoreboard, 0, 110, 450, 20, "3. Player Three  - 10000 pts", "scoreboard_row_3");
    widget_add_text(scoreboard, 0, 135, 450, 20, "4. Player Four   -  8500 pts", "scoreboard_row_4");
    widget_add_text(scoreboard, 0, 160, 450, 20, "5. Player Five   -  7000 pts", "scoreboard_row_5");

    widget_add_button(scoreboard, 0, 0, 150, 40, "Close", _callback_close_scoreboard, "scoreboard_close_button");

    widget_layout(scoreboard, 12, 40, true);
    gui_push_overlay(gui, overlay);
}
