#include "game.h"
#include "widget.h"
#include "gui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void free_text_label_on_destroy(t_widget *self);
static void update_status_date(t_widget *status_bar, t_gui *gui);



// =============================================================================
// Game View
// =============================================================================

    // -------------------------------------------------------------------------
    // Game View Callbacks
    // -------------------------------------------------------------------------

    // aqui desenha-se conforme o state, as posicoes, o mapa etc
    // usar hw_vbe_draw_xpm
    //      opcional  (futuro): inicializar as xpms das sprites como se faz com as letras
void draw_game_canvas(t_widget *self, hw_video_t *video) {
    if (self == NULL) {
        return;
    }

    t_game_state *state = (t_game_state *)(self->data.game.state);
    if (state == NULL || state->pixel_buffer == NULL) {
        return;
    }

    // Lixo, só ta aqui para a logica atual (que nao tem nada a ver com o jogo)
    //--------------------------------------------------------------------------
    int32_t abs_x = get_abs_x(self);
    int32_t abs_y = get_abs_y(self);

    hw_vbe_draw_rect(video, abs_x, abs_y, self->width, self->height, 0x0);
    draw_win95_border(video, abs_x, abs_y, self->width, self->height, false);

    for (uint32_t y = 0; y < state->height; ++y) {
        for (uint32_t x = 0; x < state->width; ++x) {
            uint16_t color = state->pixel_buffer[y * state->width + x];
            if (color != 0) {
                hw_vbe_draw_pixel(video, abs_x + (int32_t)x, abs_y + (int32_t)y, color);
            }
        }
    }
    //--------------------------------------------------------------------------
}

static void on_game_canvas_press(t_widget *self, void *state) {
    // aqui lida-se com o rato dentro do jogo
    t_gui *gui = (t_gui*)state;
    t_game_state *game = (t_game_state*)self->data.game.state;
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

static void free_game_state(t_widget *self) {
    if (self == NULL) {
        return;
    }

    t_game_state *game = (t_game_state*)self->data.game.state;
    if (game != NULL) {
        free(game->pixel_buffer);
        free(game);
        self->data.game.state = NULL;
    }
}

// UPDATES DO JOGO AQUI - SPRITES, TIMERS, TUDO
static void game_canvas_on_tick(t_widget *self, void *state) {
    (void)self;
    (void)state;
    // Placeholder for per-frame game updates (animations, timers)
}

static void status_bar_on_tick(t_widget *self, void *state) {
    if (self == NULL) return;
    t_gui *gui = (t_gui*)state;
    if (gui == NULL) return;
    update_status_date(self, gui);
}

void gui_reset_game(t_gui *gui) {
    if (gui == NULL) {
        return;
    }

    t_widget *game_canvas = gui_pop_until_widget_found(gui, "game_canvas");
    if (game_canvas == NULL) {
        return;
    }

    
    // Lixo, só ta aqui para a logica atual (que nao tem nada a ver com o jogo)
    //--------------------------------------------------------------------------
    t_game_state *game = (t_game_state*)game_canvas->data.game.state;
    if (game != NULL && game->pixel_buffer != NULL) {
        memset(game->pixel_buffer, 0, sizeof(uint16_t) * game->width * game->height);
    }
    //--------------------------------------------------------------------------
}

// manter exatamente igual
static void on_game_view_quit(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = (t_gui*)state;

    if (gui->input.focused != NULL && gui->input.focused->on_quit != NULL) {
        gui->input.focused->on_quit(gui->input.focused, state);
        return;
    }

    //o menu de pause ta declarado noutro ficheiro, mas para já não há necessidade de o mover
    gui_show_pause_menu(gui);
}

    // -------------------------------------------------------------------------
    // Game View Displays
    // -------------------------------------------------------------------------

// AQUI: o Launcher do jogo - o botao start dochama isto
// Falta receber argumentos tipo o nome dos players, etc - para iniciar o game state
void init_game(t_gui *gui) {
    t_widget *view = widget_create(CANVAS, 0, 0, gui->width, gui->height, "game_view");
    if (view == NULL) return;

    //mexer depois, para já ta a mostrar a hora
    t_widget *status_bar = widget_create(CANVAS, 0, 0, gui->width, 40, "game_status_bar");
    if (status_bar != NULL) {
        update_status_date(status_bar, gui);
        status_bar->on_tick = status_bar_on_tick;
    }
    widget_add_child(view, status_bar);

    uint32_t canvas_h = gui->height - 40;
    t_widget *game_canvas = widget_create(GAME, 0, 40, gui->width, canvas_h, "game_canvas");
    game_canvas->draw = draw_game_canvas;
    game_canvas->on_press = on_game_canvas_press;
    game_canvas->on_drag = on_game_canvas_press;
    game_canvas->on_tick = game_canvas_on_tick;

    //o game state vai levar o board, os players, start time, etc
    t_game_state *game_state = (t_game_state*)calloc(1, sizeof(t_game_state));
    if (game_state == NULL) {
        widget_destroy(view);
        return;
    }

    game_state->width = gui->width;
    game_state->height = canvas_h;
    game_state->pixel_buffer = (uint16_t*)calloc((size_t)gui->width * canvas_h, sizeof(uint16_t));
    if (game_state->pixel_buffer == NULL) {
        free(game_state);
        widget_destroy(view);
        return;
    }

    game_canvas->data.game.state = game_state;
    game_canvas->on_destroy = free_game_state;

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

static void update_status_date(t_widget *status_bar, t_gui *gui) {
    if (status_bar == NULL || gui == NULL) return;

    hw_rtc_t rtc_info;
    if (gui_get_curr_time(gui, &rtc_info) != 0) return;

    char date_buf[64];
    snprintf(date_buf, sizeof(date_buf), "20%02u-%02u-%02u %02u:%02u:%02u",
             rtc_info.year, rtc_info.month, rtc_info.day,
             rtc_info.hours, rtc_info.minutes, rtc_info.seconds);

    t_widget *date_w = widget_find_by_name(status_bar, "status_date");
    if (date_w == NULL) {
        char *label = strdup(date_buf);
        if (label == NULL) return;
        date_w = widget_add_text(status_bar, (int32_t)gui->width - 300, 8, 215, 24, label, "status_date");
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
