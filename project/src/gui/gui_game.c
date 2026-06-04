#include "gui/gui.h"
#include "game/game.h"
#include "core/macros.h"
#include "gui/widget.h"
#include "core/application.h"
#include "core/event_handlers.h"
#include "core/multiplayer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations for static callback functions
static void _callback_game_board_on_press(t_widget *self, void *state);
static void _callback_game_board_on_key_press(struct s_widget *self, uint8_t scancode, void *state);
static void _callback_game_view_on_key_press(struct s_widget *self, uint8_t scancode, void *state);
static void _callback_game_view_on_quit(t_widget *self, void *state);
static void _callback_game_view_on_tick(t_widget *self, void *state);
static void _handle_game_key(t_ctx *ctx, uint8_t scancode);

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
    ctx->game.match_state = MATCH_RUNNING;
    ctx->game.is_frozen = false;
    app_multiplayer_send_pause(ctx, false);
    gui_pop_view(GUI(state));
}

static void _callback_confirm_return_to_main_menu(t_widget *self, void *state) {
    (void)self;
    t_ctx *ctx = CTX(state);
    if (ctx->is_multiplayer) {
        app_multiplayer_send_cancel(ctx);
    }
    ctx->is_multiplayer = false;
    ctx->multiplayer_game_started = false;
    gui_pop_until_widget_found(GUI(state), "start_menu_view");
}

static void _callback_confirm_reset_game(t_widget *self, void *state) {
    (void)self;
    t_ctx *ctx = CTX(state);
    game_state_reset(GAME(state), ctx->real_time, ctx->is_multiplayer);
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
    bool game_over = ctx->game.match_state == MATCH_EXITING;

    t_widget *overlay = widget_create_overlay(gui->width, gui->height, _callback_resume_game, "session_overlay");
    if (overlay == NULL) return;

    uint32_t height = 240 + (message ? 40 : 0) + (game_over ? 0 : 50);
    t_widget *session_dialog = widget_add_dialog(overlay, title, 360, height, gui->width, gui->height, _callback_resume_game, "session_dialog");

    session_dialog->on_quit = _callback_resume_game;

    if (message) {
        widget_add_text(session_dialog, 0, 40, 320, 24, message, "session_message");
    }

    bool connection_lost = (strcmp(title, "CONNECTION LOST") == 0);

    if (!game_over && !connection_lost) {
        widget_add_button(session_dialog, 0, 0, 220, 40, "Resume", _callback_resume_game, "session_resume_button");
    }
    if (!connection_lost) {
        widget_add_button(session_dialog, 0, 0, 220, 40, "Reset", _callback_reset_game, "session_reset_button");
    }
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

    if (!ctx->game.is_frozen) {
        ctx->game.is_frozen = true;
        app_multiplayer_send_pause(ctx, true);
    }
    gui_show_session_menu(ctx, "PAUSED", NULL);
}

static void _callback_game_view_on_tick(t_widget *self, void *state) {
    (void)self;
    t_ctx *ctx = CTX(state);
    t_game_state *game = &ctx->game;

    if (game->match_state == MATCH_RUNNING && !game->is_frozen) {
        if (ctx->is_multiplayer) {
            app_multiplayer_send_ping(ctx);

            uint32_t timeout_ticks = GAME_TICKS_PER_SECOND * 2;
            if (game->logical_ticks - ctx->multiplayer_last_contact_ticks > timeout_ticks) {
                game->is_frozen = true;
                gui_show_session_menu(ctx, "CONNECTION LOST", "Peer is not responding");
                return;
            }
        }

        game->logical_ticks++;
        game_state_update(ctx);
    }

    if (game->match_state == MATCH_LOST) {
        game->is_frozen = true;
        update_player_death_animation(game, &game->players[PLAYER_1]);
        if (game->players[PLAYER_2].active) update_player_death_animation(game, &game->players[PLAYER_2]);
        if (game->animation_timer <= 0) {
            game->match_state = MATCH_EXITING;
            gui_show_session_menu(ctx, "GAME OVER", "Better luck next time!");
        }
        return;
   }

    if (game->match_state == MATCH_WON) {
        game->is_frozen = true;
        if (game->players[PLAYER_1].board_pos.x == game->door_pos.x && game->players[PLAYER_1].board_pos.y == game->door_pos.y) update_player_win_animation(game, &game->players[PLAYER_1]);
        else update_player_win_animation(game, &game->players[PLAYER_2]);
        if (game->animation_timer <= 0) {
            game->match_state = MATCH_EXITING;
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
    (void)self;
    _handle_game_key(CTX(state), scancode);
}

static void _callback_game_view_on_key_press(struct s_widget *self, uint8_t scancode, void *state) {
    (void)self;
    _handle_game_key(CTX(state), scancode);
}

static void _handle_game_key(t_ctx *ctx, uint8_t scancode) {
    if (ctx == NULL) return;

    if (ctx->is_multiplayer && ctx->multiplayer_role_assigned) {
        app_multiplayer_send_key(ctx, scancode);
        game_state_handle_player_key(&ctx->game, ctx->multiplayer_local_player, scancode);
        return;
    }

    game_state_handle_key_press(&ctx->game, scancode);
}

void gui_show_game_view(t_ctx *ctx) {
    t_gui *gui = &ctx->gui;
    if (!ctx->is_multiplayer || ctx->multiplayer_local_player == PLAYER_1) {
        app_update_real_time(ctx);
    }
    t_widget *view = widget_create(CANVAS, 0, 0, gui->width, gui->height, "game_view");
    if (view == NULL) return;

    t_widget *game_canvas = widget_create(GAME, 0, 0, gui->width, gui->height, "game_canvas");
    game_canvas->draw = draw_game_board;
    game_canvas->on_press = _callback_game_board_on_press;
    game_canvas->on_key_press = _callback_game_board_on_key_press;

    //o game state vai levar o board, os players, start time, etc


    if (game_state_init(&ctx->game, gui->width, gui->height, ctx->real_time, ctx->is_multiplayer) != 0) {
        widget_destroy(view);
        return;
    }
    if (ctx->is_multiplayer && ctx->multiplayer_role_assigned) {
        ctx->game.current_player = ctx->multiplayer_local_player;
    }

    if (ctx->is_multiplayer) {
        strncpy(ctx->game.players[ctx->multiplayer_local_player].name, ctx->multiplayer_local_name, 31);
        ctx->game.players[ctx->multiplayer_local_player].name[31] = '\0';
        strncpy(ctx->game.players[ctx->multiplayer_remote_player].name, ctx->multiplayer_remote_name, 31);
        ctx->game.players[ctx->multiplayer_remote_player].name[31] = '\0';
    } else {
        t_widget *p1_input = widget_find_by_name(gui, "player1_input");
        if (p1_input && p1_input->data.text_input.buffer && !is_blank_string(p1_input->data.text_input.buffer)) {
            strncpy(ctx->game.players[PLAYER_1].name, p1_input->data.text_input.buffer, 31);
            ctx->game.players[PLAYER_1].name[31] = '\0';
        } else {
            strcpy(ctx->game.players[PLAYER_1].name, "Player 1");
        }
        strcpy(ctx->game.players[PLAYER_2].name, "CPU");
    }

    widget_add_child(view, game_canvas);
    view->on_quit = _callback_game_view_on_quit;
    view->on_key_press = _callback_game_view_on_key_press;
    view->on_tick  = _callback_game_view_on_tick;
    gui_push_view(gui, view);
}

void gui_reset_game_view(t_ctx *ctx) {
    t_gui *gui = &ctx->gui;
    game_state_reset(&ctx->game, ctx->real_time, ctx->is_multiplayer);
    ctx->game.match_state = MATCH_RUNNING;
    gui_pop_until_widget_found(gui, "game_view");
}
