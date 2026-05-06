#include "game.h"
#include "application.h"
#include "widget.h"
#include "gui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void free_text_label_on_destroy(t_widget *self);
static void update_status_date(t_widget *status_bar, t_ctx *ctx);

// FUNÇÕES QUE VÃO CONDUZIR O JOGO
int game_state_init(t_game_state *game, uint32_t width, uint32_t height) {
    if (game == NULL || width == 0 || height == 0) {
        return 1;
    }

    game_state_destroy(game);
    game->pixel_buffer = (uint16_t*)calloc((size_t)width * height, sizeof(uint16_t));
    if (game->pixel_buffer == NULL) {
        game->width = 0;
        game->height = 0;
        return 1;
    }

    game->width = width;
    game->height = height;
    game->logical_ticks = 0;
    game->is_paused = false;
    return 0;
}

void game_state_reset(t_game_state *game) {
    if (game == NULL || game->pixel_buffer == NULL) {
        return;
    }

    memset(game->pixel_buffer, 0, sizeof(uint16_t) * game->width * game->height);
    game->logical_ticks = 0;
    game->is_paused = false;
}

void game_state_destroy(t_game_state *game) {
    if (game == NULL) {
        return;
    }

    free(game->pixel_buffer);
    game->pixel_buffer = NULL;
    game->width = 0;
    game->height = 0;
}

void update_game_logic(t_ctx *ctx) {
    (void)ctx;
}


// API QUE EXPORTA PARA A INTERFACE GRAFICA

// =============================================================================
// Game View
// =============================================================================

    // -------------------------------------------------------------------------
    // Game View Callbacks
    // -------------------------------------------------------------------------

// aqui desenha-se conforme o state, as posicoes, o mapa etc
// usar hw_vbe_draw_xpm
// opcional  (futuro): inicializar as xpms das sprites como se faz com as letras
void draw_game_canvas(t_widget *self, hw_video_t *video, void *state) {
    if (self == NULL) {
        return;
    }

    t_ctx *ctx = (t_ctx*)state;
    if (ctx == NULL || ctx->game.pixel_buffer == NULL) {
        return;
    }

    // Lixo, só ta aqui para a logica atual (que nao tem nada a ver com o jogo)
    //--------------------------------------------------------------------------
    int32_t abs_x = get_abs_x(self);
    int32_t abs_y = get_abs_y(self);

    hw_vbe_draw_rect(video, abs_x, abs_y, self->width, self->height, 0x0);
    draw_win95_border(video, abs_x, abs_y, self->width, self->height, false);

    for (uint32_t y = 0; y < ctx->game.height; ++y) {
        for (uint32_t x = 0; x < ctx->game.width; ++x) {
            uint16_t color = ctx->game.pixel_buffer[y * ctx->game.width + x];
            if (color != 0) {
                hw_vbe_draw_pixel(video, abs_x + (int32_t)x, abs_y + (int32_t)y, color);
            }
        }
    }
    //--------------------------------------------------------------------------
}

static void on_game_canvas_press(t_widget *self, void *state) {
    // aqui lida-se com o rato dentro do jogo
    t_ctx *ctx = (t_ctx*)state;
    t_gui *gui = &ctx->gui;
    t_game_state *game = &ctx->game;
    int32_t click_x = gui->input.mouse_x - get_abs_x(self);
    int32_t click_y = gui->input.mouse_y - get_abs_y(self);

    // Lixo, só ta aqui para a logica atual (que nao tem nada a ver com o jogo)
    //--------------------------------------------------------------------------
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            int32_t px = click_x + x;
            int32_t py = click_y + y;
            if (px >= 0 && px < (int32_t)game->width && py >= 0 && py < (int32_t)game->height) {
                game->pixel_buffer[py * game->width + px] = 0xFFFF;
            }
        }
    }
    //--------------------------------------------------------------------------
}

static void status_bar_on_tick(t_widget *self, void *state) {
    if (self == NULL) return;
    t_ctx *ctx = (t_ctx*)state;
    if (ctx == NULL) return;
    update_status_date(self, ctx);
}

void gui_reset_game(t_ctx *ctx) {
    if (ctx == NULL)
        return;
    t_gui *gui = &ctx->gui;
    (void)game_canvas;
    game_state_reset(&ctx->game);
}

// manter exatamente igual
static void on_game_view_quit(t_widget *self, void *state) {
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

    // -------------------------------------------------------------------------
    // Game View Displays
    // -------------------------------------------------------------------------

// AQUI: o Launcher do jogo - o botao start dochama isto
// Falta receber argumentos tipo o nome dos players, etc - para iniciar o game state
void init_game(t_ctx *ctx) {
    t_gui *gui = &ctx->gui;
    t_widget *view = widget_create(CANVAS, 0, 0, gui->width, gui->height, "game_view");
    if (view == NULL) return;

    //mexer depois, para já ta a mostrar a hora
    t_widget *status_bar = widget_create(CANVAS, 0, 0, gui->width, 40, "game_status_bar");
    if (status_bar != NULL) {
        update_status_date(status_bar, ctx);
        status_bar->on_tick = status_bar_on_tick;
    }
    widget_add_child(view, status_bar);

    uint32_t canvas_h = gui->height - 40;
    t_widget *game_canvas = widget_create(GAME, 0, 40, gui->width, canvas_h, "game_canvas");
    game_canvas->draw = draw_game_canvas;
    game_canvas->on_press = on_game_canvas_press;
    game_canvas->on_drag = on_game_canvas_press;

    //o game state vai levar o board, os players, start time, etc
    if (game_state_init(&ctx->game, gui->width, canvas_h) != 0) {
        widget_destroy(view);
        return;
    }

    widget_add_child(view, game_canvas);
    view->on_quit = on_game_view_quit;
    gui_push_view(gui, view);
}




// ignorar mais ou menos

static void free_text_label_on_destroy(t_widget *self) {
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
            date_w->on_destroy = free_text_label_on_destroy;
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
