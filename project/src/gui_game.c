#include "game.h"
#include "macros.h"
#include "widget.h"
#include "application.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations for static callback functions
static void _callback_game_board_on_press(t_widget *self, void *state);
static void _callback_game_board_on_key_press(struct s_widget *self, uint8_t scancode, void *state);
static void _callback_game_view_on_key_press(struct s_widget *self, uint8_t scancode, void *state);
static void _callback_game_view_on_quit(t_widget *self, void *state);
static void _callback_game_view_on_tick(t_widget *self, void *state);

// =============================================================================
// Game View
// =============================================================================

static void _callback_pop_view(t_widget *self, void *state) {
    (void)self;
    gui_pop_view(GUI(state));
}

static void _callback_resume_game(t_widget *self, void *state) {
    (void)self;
    t_ctx *ctx = CTX(state);
    ctx->game.is_frozen = false;
    gui_pop_view(GUI(state));
}

static void _callback_confirm_return_to_main_menu(t_widget *self, void *state) {
    (void)self;
    gui_pop_until_widget_found(GUI(state), "start_menu_view");
}

static void _callback_confirm_reset_game(t_widget *self, void *state) {
    (void)self;
    t_ctx *ctx = CTX(state);
    game_state_reset(GAME(state), ctx->real_time);
    ctx->game.is_frozen = false;
    gui_pop_until_widget_found(GUI(state), "game_view");
}

static void _callback_reset_game(t_widget *self, void *state) {
    (void)self;
    gui_show_confirm_dialog(CTX(state), "Confirm Reset", "Are you sure?", _callback_confirm_reset_game, _callback_pop_view);
}

static void _callback_return_to_main_menu(t_widget *self, void *state) {
    (void)self;
    gui_show_confirm_dialog(CTX(state), "Confirm Main Menu", "Return to main menu?", _callback_confirm_return_to_main_menu, _callback_pop_view);
}

void gui_show_session_menu(t_ctx *ctx, const char *title, const char *message) {
    t_gui *gui = &ctx->gui;
    bool game_over = ctx->game.match_state != MATCH_RUNNING;

    t_widget *overlay = widget_create_overlay(gui->width, gui->height, _callback_resume_game, "session_overlay");
    if (overlay == NULL) return;

    uint32_t height = 240 + (message ? 40 : 0) + (game_over ? 0 : 50);
    t_widget *session_dialog = widget_add_dialog(overlay, title, 360, height, gui->width, gui->height, _callback_resume_game, "session_dialog");

    session_dialog->on_quit = _callback_resume_game;

    if (message) {
        widget_add_text(session_dialog, 0, 40, 320, 24, message, "session_message");
    }

    if (!game_over) {
        widget_add_button(session_dialog, 0, 0, 220, 40, "Resume", _callback_resume_game, "session_resume_button");
    }
    widget_add_button(session_dialog, 0, 0, 220, 40, "Reset", _callback_reset_game, "session_reset_button");
    widget_add_button(session_dialog, 0, 0, 220, 40, "Main Menu", _callback_return_to_main_menu, "session_menu_button");

    widget_layout(session_dialog, 12, message ? 80 : 32, true);
    gui_push_overlay(gui, overlay);
}

static void _callback_game_view_on_quit(t_widget *self, void *state) {
    (void)self;
    t_ctx *ctx = CTX(state);
    t_gui *gui = GUI(state);

    if (gui->input.focused != NULL && gui->input.focused != self && gui->input.focused->on_quit != NULL) {
        gui->input.focused->on_quit(gui->input.focused, state);
        return;
    }

    if (!ctx->game.is_frozen)
    	ctx->game.is_frozen = true;
    gui_show_session_menu(ctx, "PAUSED", NULL);
}

static void _callback_game_view_on_tick(t_widget *self, void *state) {
    (void)self;
    t_ctx *ctx = CTX(state);
    t_game_state *game = &ctx->game;

    if (game->match_state == MATCH_RUNNING && !game->is_frozen) {
        game->logical_ticks++;
        game_state_update(ctx);
    }
    
    if (game->match_state == MATCH_LOST) {
        game->is_frozen = true;
        update_player_death_animation(game, &game->players[PLAYER_1]);
        if (game->animation_timer > 10000) { //if (game->animation_timer == 0) {
            gui_show_session_menu(ctx, "PAUSED", NULL);
        }
        return;
   }
   
    if (game->match_state == MATCH_WON) {
        game->is_frozen = true;
        update_player_win_animation(game, &game->players[PLAYER_1]);
        if (game->animation_timer > 10000) { //if (game->animation_timer == 0) {
            gui_show_session_menu(
                ctx,
                "YOU WIN!",
                "All enemies defeated!"
            );
        }
        return;  
    }
}

static void _callback_game_board_on_press(t_widget *self, void *state) {
    t_gui *gui = GUI(state);
    t_game_state *game = GAME(state);

    game_state_handle_click(
        game,
        gui->input.mouse_x - self->abs_x,
        gui->input.mouse_y - self->abs_y
    );
}

static void _callback_game_board_on_key_press(struct s_widget *self, uint8_t scancode, void *state) {
    game_state_handle_key_press(GAME(state), scancode);
}

static void _callback_game_view_on_key_press(struct s_widget *self, uint8_t scancode, void *state) {
    (void)self;
    game_state_handle_key_press(GAME(state), scancode);
}

void gui_show_game_view(t_ctx *ctx) {
    t_gui *gui = &ctx->gui;
    app_update_real_time(ctx);

    t_widget *view = widget_create(CANVAS, 0, 0, gui->width, gui->height, "game_view");
    if (view == NULL) return;

    t_widget *game_canvas = widget_create(GAME, 0, 0, gui->width, gui->height, "game_canvas");
    game_canvas->draw = draw_game_board;
    game_canvas->on_press = _callback_game_board_on_press;
    game_canvas->on_key_press = _callback_game_board_on_key_press;

    //o game state vai levar o board, os players, start time, etc
    if (game_state_init(&ctx->game, gui->width, gui->height, ctx->real_time) != 0) {
        widget_destroy(view);
        return;
    }

    // Retrieve player names from the name menu inputs (AFTER INIT)
    t_widget *p1_input = widget_find_by_name(gui, "player1_input");
    t_widget *p2_input = widget_find_by_name(gui, "player2_input");

    if (p1_input && p1_input->data.text_input.buffer) {
        strncpy(ctx->game.players[PLAYER_1].name, p1_input->data.text_input.buffer, 31);
        ctx->game.players[PLAYER_1].name[31] = '\0';
    } else {
        strcpy(ctx->game.players[PLAYER_1].name, "P1");
    }
    if (p2_input && p2_input->data.text_input.buffer) {
        strncpy(ctx->game.players[PLAYER_2].name, p2_input->data.text_input.buffer, 31);
        ctx->game.players[PLAYER_2].name[31] = '\0';
    } else {
        strcpy(ctx->game.players[PLAYER_2].name, "P2");
    }

    widget_add_child(view, game_canvas);
    view->on_quit = _callback_game_view_on_quit;
    view->on_key_press = _callback_game_view_on_key_press;
    view->on_tick  = _callback_game_view_on_tick;
    gui_push_view(gui, view);
}

void gui_reset_game_view(t_ctx *ctx) {
    t_gui *gui = &ctx->gui;
    game_state_reset(&ctx->game, ctx->real_time);
    ctx->game.match_state = MATCH_RUNNING;
    gui_pop_until_widget_found(gui, "game_view");
}
