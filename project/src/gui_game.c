#include "game.h"
#include "widget.h"
#include "application.h"
#include "event_handlers.h"
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

static void _callback_game_view_on_quit(t_widget *self, void *state) {
    (void)self;
    t_ctx *ctx = CTX(state);
    t_gui *gui = GUI(state);

    if (gui->input.focused != NULL && gui->input.focused != self && gui->input.focused->on_quit != NULL) {
        gui->input.focused->on_quit(gui->input.focused, state);
        return;
    }

    ctx->game.is_paused = true;
    gui_show_pause_menu(ctx);
}

static void _callback_game_view_on_tick(t_widget *self, void *state)
{
    (void)self;
    t_ctx *ctx = CTX(state);
    t_game_state *game = &ctx->game;

    if (!ctx->game.is_paused) {
        ctx->game.logical_ticks++;
        game_state_update(ctx);
    }

    bool p1_dead = (!game->players[PLAYER_1].active && game->players[PLAYER_1].lives == 0);

    if (p1_dead) {
        game->is_paused = true;
        gui_show_game_end_dialog(ctx, "GAME OVER", "You have no lives left!");
    } else if (game->door_open) {
        t_tuple player_pos = game->players[PLAYER_1].board_pos;
        if (player_pos.x == game->door_pos.x && player_pos.y == game->door_pos.y) {
            game->is_paused = true;
            gui_show_game_end_dialog(ctx, "YOU WIN!", "All enemies defeated!");
        }
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
    game_state_reset(&ctx->game, ctx->real_time, ctx->is_multiplayer);
    ctx->game.is_paused = false;
    gui_pop_until_widget_found(gui, "game_view");
}
