#include "game.h"
#include "widget.h"
#include "application.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations for static callback functions
static void _callback_game_board_on_press(t_widget *self, void *state);
static void _callback_game_board_on_key_press(struct s_widget *self, uint8_t scancode, void *state);
static void _callback_game_view_on_key_press(struct s_widget *self, uint8_t scancode, void *state);
static void _callback_status_bar_on_tick(t_widget *self, void *state);
static void _callback_game_view_on_quit(t_widget *self, void *state);
static void _callback_text_label_on_destroy(t_widget *self);

// Forward declarations for helpers
static void update_status_date(t_widget *status_bar, t_ctx *ctx);

// =============================================================================
// Game View
// =============================================================================

// manter exatamente igual
static void _callback_game_view_on_quit(t_widget *self, void *state) {
    (void)self;
    t_ctx *ctx = (t_ctx*)state;
    t_gui *gui = &ctx->gui;

    //isto esta feio mas não mexer até absoluta necessidade
    if (gui->input.focused != NULL && gui->input.focused != self && gui->input.focused->on_quit != NULL) {
        gui->input.focused->on_quit(gui->input.focused, state);
        return;
    }
    ctx->game.is_paused = true;
    gui_show_pause_menu(ctx);
}

static void _callback_game_board_on_press(t_widget *self, void *state) {
    t_ctx *ctx = (t_ctx*)state;
    t_gui *gui = &ctx->gui;
    t_game_state *game = &ctx->game;
   
    game_state_handle_click(
        game,
        gui->input.mouse_x - self->abs_x,
        gui->input.mouse_y - self->abs_y
    );
}

static void _callback_game_board_on_key_press(struct s_widget *self, uint8_t scancode, void *state) {
    t_ctx *ctx = (t_ctx*)state;
    t_game_state *game = &ctx->game;
    game_state_handle_key_press(game, scancode);
}

static void _callback_game_view_on_key_press(struct s_widget *self, uint8_t scancode, void *state) {
    (void)self;
    t_ctx *ctx = (t_ctx*)state;
    t_game_state *game = &ctx->game;
    game_state_handle_key_press(game, scancode);
}

// -------------------------------------------------------------------------
// Game View
// -------------------------------------------------------------------------

// AQUI: o Launcher do jogo - o botao start dochama isto
void gui_show_game_view(t_ctx *ctx) {
    t_gui *gui = &ctx->gui;
    t_widget *view = widget_create(CANVAS, 0, 0, gui->width, gui->height, "game_view");
    if (view == NULL) return;

    //mexer depois, para já ta a mostrar a hora
    t_widget *status_bar = widget_create(CANVAS, 0, 0, gui->width, 40, "game_status_bar");
    if (status_bar != NULL) {
        update_status_date(status_bar, ctx);
        status_bar->on_tick = _callback_status_bar_on_tick;
    }
    widget_add_child(view, status_bar);

    uint32_t canvas_h = gui->height - 40;
    t_widget *game_canvas = widget_create(GAME, 0, 40, gui->width, canvas_h, "game_canvas");
    game_canvas->draw = draw_game_board;
    game_canvas->on_press = _callback_game_board_on_press;
    game_canvas->on_key_press = _callback_game_board_on_key_press;

    //TODO: REVISIT ON_DRAG LATER
    game_canvas->on_drag = _callback_game_board_on_press;

    //o game state vai levar o board, os players, start time, etc
    if (game_state_init(&ctx->game, gui->width, canvas_h, ctx->real_time) != 0) {
        widget_destroy(view);
        return;
    }

    widget_add_child(view, game_canvas);
    view->on_quit = _callback_game_view_on_quit;
    view->on_key_press = _callback_game_view_on_key_press;
    gui_push_view(gui, view);
}

// -------------------------------------------------------------------------
// Game Status Bar
// -------------------------------------------------------------------------

static void _callback_status_bar_on_tick(t_widget *self, void *state) {
    if (self == NULL) return;
    t_ctx *ctx = (t_ctx*)state;
    if (ctx == NULL) return;
    update_status_date(self, ctx);
}

// ignorar mais ou menos
static void _callback_text_label_on_destroy(t_widget *self) {
    if (self == NULL) return;
    if (self->data.text_display.text != NULL) {
        free(self->data.text_display.text);
        self->data.text_display.text = NULL;
    }
}

static void update_status_date(t_widget *status_bar, t_ctx *ctx) {
    if (status_bar == NULL || ctx == NULL) return;

    char date_buf[64];
    snprintf(date_buf, sizeof(date_buf), "20%02u-%02u-%02u %02u:%02u:%02u",
             ctx->real_time.year, ctx->real_time.month, ctx->real_time.day,
             ctx->real_time.hours, ctx->real_time.minutes, ctx->real_time.seconds);

    t_widget *date_w = widget_find_by_name(status_bar, "status_date");
    if (date_w == NULL) {
        char *label = strdup(date_buf);
        if (label == NULL) return;
        date_w = widget_add_text(status_bar, (int32_t)ctx->gui.width - 300, 8, 215, 24, label, "status_date");
        if (date_w != NULL) {
            date_w->on_destroy = _callback_text_label_on_destroy;
        } else {
            free(label);
        }
        return;
    }

    const char *cur = date_w->data.text_display.text;
    if (cur == NULL || strcmp(cur, date_buf) != 0) {
        char *new_label = strdup(date_buf);
        if (new_label == NULL) return;
        if (date_w->data.text_display.text != NULL) free(date_w->data.text_display.text);
        date_w->data.text_display.text = new_label;
    }
}
