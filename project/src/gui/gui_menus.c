#include "gui/gui.h"
#include "view/assets_cache.h"
#include <lcom/xpm.h>
#include <lcom/lcf.h>
#include "game/game.h"
#include "core/application.h"
#include "core/event_handlers.h"
#include "multiplayer/multiplayer.h"
#include "serial_port.h"
#include "scoreboard/scoreboard_controller.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// Forward declarations
static void _callback_pop_view(t_widget *self, void *state);
static void _callback_focus_self(t_widget *self, void *state);
static void _callback_multiplayer_abort(t_widget *self, void *state);
static void _callback_start_game(t_widget *self, void *state);

// =============================================================================
// Entry Point
// =============================================================================

void gui_init(struct s_ctx *ctx, uint32_t screen_width, uint32_t screen_height) {
    memset(&ctx->gui, 0, sizeof(ctx->gui));
    ctx->gui.width      = screen_width;
    ctx->gui.height     = screen_height;
    ctx->gui.is_running = true;

    gui_show_start_menu(ctx);
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

// =============================================================================
// Start Menu
// =============================================================================

static void _callback_confirm_quit(t_widget *self, void *state) {
    (void)self;
    GUI(state)->is_running = false;
}

static void _callback_quit(t_widget *self, void *state) {
    (void)self;
    gui_show_confirm_dialog(CTX(state), "QUIT", "DO YOU REALLY WANT TO QUIT",
                            _callback_confirm_quit, _callback_pop_view);
}

static void _callback_multiplayer_abort(t_widget *self, void *state) {
    (void)self;
    t_ctx    *ctx = CTX(state);
    t_widget *top = gui_get_top_view(&ctx->gui);

    app_multiplayer_send_cancel(ctx);
    gui_pop_view(&ctx->gui);

    if (top != NULL && top->name != NULL && strcmp(top->name, "info_overlay") == 0)
        ctx->multiplayer_local_start_ready = false;
    else
        ctx->is_multiplayer = false;
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

    ctx->is_multiplayer                      = true;
    ctx->multiplayer_partner_ready           = false;
    ctx->multiplayer_signal_sent             = false;
    ctx->multiplayer_role_assigned           = false;
    ctx->multiplayer_local_start_ready       = false;
    ctx->multiplayer_remote_start_ready      = false;
    ctx->multiplayer_start_game_sent         = false;
    ctx->multiplayer_start_game_pending      = false;
    ctx->multiplayer_game_started            = false;
    ctx->multiplayer_name_sent               = false;
    ctx->multiplayer_name_received           = false;
    ctx->multiplayer_local_player            = PLAYER_1;
    ctx->multiplayer_remote_player           = PLAYER_2;
    ctx->multiplayer_match_seed              = 0;
    ctx->game.enemy_seed                     = 0;
    ctx->multiplayer_remote_nonce            = 0;
    ctx->multiplayer_remote_tiebreaker       = 0;
    ctx->multiplayer_rx_state                = 0;
    ctx->multiplayer_rx_type                 = 0;
    ctx->multiplayer_rx_pos                  = 0;
    ctx->multiplayer_last_player_lives[0]    = 0;
    ctx->multiplayer_last_player_lives[1]    = 0;
    ctx->multiplayer_last_player_active[0]   = false;
    ctx->multiplayer_last_player_active[1]   = false;
    ctx->multiplayer_last_player_powerups[0] = 0;
    ctx->multiplayer_last_player_powerups[1] = 0;
    ctx->game.multiplayer_last_contact_ticks = 0;
    memset(ctx->multiplayer_local_name,  0, sizeof(ctx->multiplayer_local_name));
    memset(ctx->multiplayer_remote_name, 0, sizeof(ctx->multiplayer_remote_name));
    ctx->multiplayer_local_nonce =
        (uint16_t)((ctx->real_time.seconds * 251u) ^
                   (ctx->real_time.minutes * 61u)  ^
                   (ctx->real_time.hours   * 17u)  ^
                   (ctx->real_time.day     * 7u)   ^
                   ((uint32_t)clock()    & 0xFFFFu) ^
                   ((uintptr_t)ctx       & 0xFFFFu));
    ctx->multiplayer_local_tiebreaker =
        (uint8_t)(((unsigned long long)ctx ^ (unsigned long long)ctx->multiplayer_local_nonce) & 0xFFu);

    serial_flush_rx();
    app_multiplayer_send_hello(ctx);

    t_widget *overlay = widget_create_overlay(ctx->gui.width, ctx->gui.height, _callback_multiplayer_abort, "wait_conn_overlay");
    if (overlay != NULL) {
        WIDGET_SET_ACTIVE(overlay, true);
        t_widget *dlg = widget_add_dialog(overlay, "Connecting", 400, 200,
                                          ctx->gui.width, ctx->gui.height,
                                          _callback_multiplayer_abort, "wait_conn_dialog");
        widget_add_text  (dlg, 0, 0, 300, 40, "Waiting for connection...", "wait_conn_text");
        widget_add_button(dlg, 0, 0, 150, 40, "Abort", _callback_multiplayer_abort, "wait_conn_abort_btn");
        widget_layout(dlg, 16, 48, true);
        gui_push_overlay(&ctx->gui, overlay);
    }
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
    t_gui    *gui  = &ctx->gui;
    t_widget *menu = widget_create(CANVAS, 0, 0, gui->width, gui->height, "start_menu_view");
    if (menu == NULL) return;

    menu->draw    = _draw_start_menu;
    menu->on_quit = _callback_quit;

    widget_add_button(menu, 0, 0, 300, 50, "Singleplayer", _callback_show_singleplayer_name_menu, "start_single_button");
    widget_add_button(menu, 0, 0, 300, 50, "Multiplayer",  _callback_show_multiplayer_name_menu,  "start_multi_button");
    widget_add_button(menu, 0, 0, 300, 50, "Scoreboard",   _callback_show_scoreboard,             "start_scoreboard_button");
    widget_add_button(menu, 0, 0, 300, 50, "QUIT",         _callback_quit,                        "start_quit_button");

    widget_layout(menu, 24, 450, true);
    gui_push_view(gui, menu);
}

// =============================================================================
// Name Menu
// =============================================================================

static void _callback_start_game(t_widget *self, void *state) {
    (void)self;
    t_ctx    *ctx   = CTX(state);
    t_gui    *gui   = GUI(state);
    t_widget *input = widget_find_by_name(gui, "player1_input");

    if (input == NULL || input->data.text_input.buffer == NULL ||
        is_blank_string(input->data.text_input.buffer)) {
        gui_show_info_dialog(ctx, "Invalid Name", "Please enter a name");
        return;
    }

    if (ctx->is_multiplayer) {
        strncpy(ctx->multiplayer_local_name, input->data.text_input.buffer, 31);
        ctx->multiplayer_local_name[31] = '\0';
        app_multiplayer_send_name(ctx);
        ctx->multiplayer_local_start_ready = true;
        app_multiplayer_send_start_ready(ctx);

        t_widget *overlay = widget_create_overlay(gui->width, gui->height, _callback_multiplayer_abort, "info_overlay");
        if (overlay != NULL) {
            t_widget *dlg = widget_add_dialog(overlay, "Waiting", 400, 200,
                                              gui->width, gui->height,
                                              _callback_multiplayer_abort, "info_dialog");
            widget_add_text  (dlg, 0, 0, 300, 40, "Waiting for other player...", "wait_text");
            widget_add_button(dlg, 0, 0, 150, 40, "Abort", _callback_multiplayer_abort, "wait_abort_btn");
            widget_layout(dlg, 16, 48, true);
            gui_push_overlay(gui, overlay);
        }
        return;
    }

    char saved_name[32];
    strncpy(saved_name, input->data.text_input.buffer, sizeof(saved_name) - 1);
    saved_name[31] = '\0';
    scoreboard_set_current_player(saved_name);

    gui_pop_view(gui);
    gui_show_game_view(ctx);
}

void gui_show_name_menu(struct s_ctx *ctx, bool is_multiplayer) {
    t_gui *gui     = &ctx->gui;
    void  (*on_quit)(t_widget*, void*) = is_multiplayer ? _callback_multiplayer_abort : _callback_pop_view;

    t_widget *overlay = widget_create_overlay(gui->width, gui->height, on_quit, "name_overlay");
    if (overlay == NULL) return;

    WIDGET_SET_ACTIVE(overlay, true);

    const char *title = is_multiplayer ? "Enter Your Name" : "Enter Player Name";
    t_widget   *dlg   = widget_add_dialog(overlay, title, 400, 200, gui->width, gui->height, on_quit, "name_dialog");

    widget_add_text_input(dlg, 0, 0, 300, 40, "Name",  _callback_focus_self, "player1_input");
    widget_add_button    (dlg, 0, 0, 150, 40, "Start", _callback_start_game, "start_game_button");

    widget_layout(dlg, 16, 48, true);
    gui_push_overlay(gui, overlay);
}
