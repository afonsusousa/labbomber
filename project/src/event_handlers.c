#include "hardware.h"
#include "gui.h"
#include "application.h"
#include "widget.h"
#include "draw.h"
#include "../lib/rtc/rtc.h"
#include <time.h>

void draw_debug_overlay(hw_video_t *video, const t_gui *gui, t_game_state game);

void game_set_phase(t_game_state *game, game_phase_t new_phase) {
    if (game == NULL) return;
    game->phase       = new_phase;
}

void app_set_state(t_ctx *ctx, app_state_t new_state) {
    if (ctx == NULL) return;
    ctx->state = new_state;
}

static bool app_time_is_valid(const t_time *time) {
    if (time == NULL) return false;

    return time->month >= 1 && time->month <= 12 &&
           time->day >= 1 && time->day <= 31 &&
           time->hours <= 23 &&
           time->minutes <= 59 &&
           time->seconds <= 59;
}

static uint8_t days_in_month(uint8_t month, uint8_t year) {
    static const uint8_t days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (month < 1 || month > 12) return 31;
    if (month == 2 && year % 4 == 0) return 29;

    return days[month - 1];
}

int app_update_real_time(t_ctx *ctx) {
    if (ctx == NULL) return 1;

    hw_rtc_t hw_time;
    if (hw_rtc_get_time(&hw_time) != 0) {
        time_t now = time(NULL);
        struct tm *tm_now = localtime(&now);
        if (tm_now == NULL) return 1;

        ctx->real_time.year    = (uint8_t)(tm_now->tm_year % 100);
        ctx->real_time.month   = (uint8_t)(tm_now->tm_mon + 1);
        ctx->real_time.day     = (uint8_t)tm_now->tm_mday;
        ctx->real_time.hours   = (uint8_t)tm_now->tm_hour;
        ctx->real_time.minutes = (uint8_t)tm_now->tm_min;
        ctx->real_time.seconds = (uint8_t)tm_now->tm_sec;

        return app_time_is_valid(&ctx->real_time) ? 0 : 1;
    }

    ctx->real_time.year    = hw_time.year;
    ctx->real_time.month   = hw_time.month;
    ctx->real_time.day     = hw_time.day;
    ctx->real_time.hours   = hw_time.hours;
    ctx->real_time.minutes = hw_time.minutes;
    ctx->real_time.seconds = hw_time.seconds;

    return app_time_is_valid(&ctx->real_time) ? 0 : 1;
}

void app_tick_real_time(t_ctx *ctx) {
    if (ctx == NULL || !app_time_is_valid(&ctx->real_time)) {
        app_update_real_time(ctx);
        return;
    }

    ctx->real_time.seconds++;
    if (ctx->real_time.seconds < 60) return;

    ctx->real_time.seconds = 0;
    ctx->real_time.minutes++;
    if (ctx->real_time.minutes < 60) return;

    ctx->real_time.minutes = 0;
    ctx->real_time.hours++;
    if (ctx->real_time.hours < 24) return;

    ctx->real_time.hours = 0;
    ctx->real_time.day++;
    if (ctx->real_time.day <= days_in_month(ctx->real_time.month, ctx->real_time.year)) return;

    ctx->real_time.day = 1;
    ctx->real_time.month++;
    if (ctx->real_time.month <= 12) return;

    ctx->real_time.month = 1;
    ctx->real_time.year++;
}


static void _handle_game_phases(t_ctx *ctx) {
    t_game_state *game = &ctx->game;

    switch (game->phase) {

    case GAME_PHASE_PLAYING:
        /* Advance simulation */    
        game->logical_ticks++;
        game_state_update(ctx);

        /* Check lose condition: player 1 killed by enemy */
        player_t *p = &game->players[PLAYER_1];
        if (p->invincibility_ticks > 0) {
            p->invincibility_ticks--;
        } else if (player_collides_with_enemy(game, p)) {
            p->lives--;
            if (p->lives == 0) {
                game_set_phase(game, GAME_PHASE_GAME_OVER);
            } else {
                p->invincibility_ticks = INVINCIBILITY_TICKS; 
        /* ativar a animação de morte do player */
    }
}

        /* Check win condition: all enemies dead */
        {
            int alive = 0;
            for (int i = 0; i < game->enemy_count; i++) {
                if (game->enemies[i].active) alive++;
            }
            if (alive == 0) {
                game_set_phase(game, GAME_PHASE_VICTORY);
            }
        }
        break;


    case GAME_PHASE_PAUSED:
    case GAME_PHASE_GAME_OVER:
    case GAME_PHASE_VICTORY:
        break;
    }
}


void handle_timer(hardware_t *hw_state, t_ctx *ctx) {
    t_gui *gui = &ctx->gui;
    hw_timer_int_handler(&hw_state->timer);
    
    if (hw_state->timer.ticks % 60 == 0) {
        app_tick_real_time(ctx);
    }
    
    hw_vbe_clear_screen(&hw_state->video, 0x0);

    if (gui->views.view_count > 0) {
        int start_idx = gui->views.view_count - 1;
        while (start_idx > 0 && gui->views.is_overlay[start_idx]) {
            start_idx--;
        }
        for (int i = start_idx; i < gui->views.view_count; i++) {
            widget_tick(gui->views.view_stack[i], ctx);
            widget_draw(gui->views.view_stack[i], &hw_state->video, ctx);
        }
    }

    if (gui->input.hovered != NULL && gui->input.hovered->type == TEXT_INPUT) {
        draw_text_cursor(&hw_state->video, &hw_state->mouse);
    } else {
        draw_mouse(&hw_state->video, &hw_state->mouse);
    }

    draw_debug_overlay(&hw_state->video, gui, ctx->game);
    hw_vbe_flip_buffer(&hw_state->video);
}

//will need to check if this is enough for CTRL SHIFT and other 2byte keys
void handle_keyboard(hardware_t *hw_state, t_ctx *ctx, bool *esc_was_pressed) {
    t_gui *gui = &ctx->gui;
    hw_keyboard_ih(&hw_state->keyboard);

    gui->input.shift_down = hw_state->keyboard.keys_pressed[KEY_SHIFT_LEFT] || hw_state->keyboard.keys_pressed[KEY_SHIFT_RIGHT];
    gui->input.ctrl_down  = hw_state->keyboard.keys_pressed[KEY_CTRL];

    uint8_t sc = hw_state->keyboard.scancode;
    if (sc == KB_EXT_PREFIX) return; // ignore extended  prefix

    bool is_make = IS_MAKE_CODE(sc);
    uint8_t key_index = MAKE_FROM_BREAK(sc);

    if (key_index == KEY_ESC) {
        if (is_make && !(*esc_was_pressed)) {
            *esc_was_pressed = true;

            t_widget *top_view = gui_get_top_view(gui);
            if (gui->input.focused != NULL && gui->input.focused->on_quit != NULL) {
                gui->input.focused->on_quit(gui->input.focused, ctx);
            } else if (top_view != NULL && top_view->on_quit != NULL) {
                top_view->on_quit(top_view, ctx);
            }
        } else if (!is_make) {
            *esc_was_pressed = false;
        }
    }

    if (is_make) {
        if (sc == KEY_TAB) {
            gui_handle_tab_navigation(gui, gui->input.shift_down);
            return;
        }
    }

    if (gui->input.focused != NULL && gui->input.focused->on_key_press != NULL) {
        gui->input.focused->on_key_press(gui->input.focused, sc, ctx);
    }

    if (is_make && key_index == KEY_E) {
        hw_state->keyboard.keys_pressed[KEY_E] = false;
    }
}

void handle_mouse(hardware_t *hw_state, t_ctx *ctx) {
    t_gui *gui = &ctx->gui;
    if (!hw_mouse_ih(&hw_state->mouse)) return;

    gui->input.mouse_x = hw_state->mouse.x;
    gui->input.mouse_y = hw_state->mouse.y;

    bool is_pressed = hw_state->mouse.left_click;
    t_widget *target = widget_get_at(gui_get_top_view(gui), gui->input.mouse_x, gui->input.mouse_y);

    t_widget **clicked = &gui->input.clicked_widget;
    t_widget **hovered = &gui->input.hovered;

    if (is_pressed) {
        if (!*clicked) {
            ctx->game.click_count++;
            WIDGET_SET_CLICKED(gui, target);
            if (target) {
                if (WIDGET_CAN_RECEIVE_FOCUS(target)) {
                    gui_set_focus(gui, target);
                }
                if (target->on_press) target->on_press(target, ctx);
            }
        } else {
            t_widget *active_drag = gui->drag.dragged_widget;
            if (active_drag != NULL && active_drag->on_drag != NULL) {
                active_drag->on_drag(active_drag, ctx);
            }
        }
    } else {
        if (*clicked) {
            if (*clicked == target && (*clicked)->on_click) {
                (*clicked)->on_click(*clicked, ctx);
            }
            WIDGET_SET_CLICKED(gui, NULL);
        }

        gui_end_drag(gui);
    }

    if (gui->drag.dragged_widget == NULL && *hovered != target) {
        WIDGET_SET_HOVERED(gui, target);
    }
}
