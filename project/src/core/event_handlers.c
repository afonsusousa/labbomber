#include "core/hardware.h"
#include "gui/gui.h"
#include "core/application.h"
#include "time/app_time.h"
#include "gui/widget.h"
#include "view/draw.h"
#include "multiplayer/multiplayer.h"
#include "i8042.h"
#include <stdbool.h>
#include <stdint.h>

void draw_debug_overlay(hw_video_t *video, const t_gui *gui, t_game_state game);

void handle_timer(hardware_t *hw_state, t_ctx *ctx) {
    t_gui *gui = &ctx->gui;
    static int handshake_retry_delay = 0;

    hw_timer_int_handler(&hw_state->timer);

    if (ctx->is_multiplayer) {
        app_multiplayer_start_pending_game(ctx);
        app_multiplayer_try_start_game(ctx);
    }

    if (ctx->is_multiplayer) {
        if (handshake_retry_delay <= 0) {
            app_multiplayer_send_hello(ctx);
            ctx->multiplayer_signal_sent = true;
            handshake_retry_delay = 60;
        } else {
            handshake_retry_delay--;
        }
    } else {
        handshake_retry_delay = 0;
    }

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

    if (ctx->is_multiplayer && ctx->multiplayer_role_assigned) {
        uint8_t local_player = ctx->multiplayer_local_player;
        player_t *player = &ctx->game.players[local_player];

        if (player->lives != ctx->multiplayer_last_player_lives[local_player] ||
            player->active != ctx->multiplayer_last_player_active[local_player] ||
            player->powerups != ctx->multiplayer_last_player_powerups[local_player]) {
            app_multiplayer_send_player_state(ctx, local_player);
            ctx->multiplayer_last_player_lives[local_player] = player->lives;
            ctx->multiplayer_last_player_active[local_player] = player->active;
            ctx->multiplayer_last_player_powerups[local_player] = player->powerups;
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

void handle_keyboard(hardware_t *hw_state, t_ctx *ctx, bool *esc_was_pressed) {
    t_gui *gui = &ctx->gui;

    hw_keyboard_ih(&hw_state->keyboard);

    gui->input.shift_down = hw_state->keyboard.keys_pressed[KEY_SHIFT_LEFT] ||
                            hw_state->keyboard.keys_pressed[KEY_SHIFT_RIGHT];
    gui->input.ctrl_down = hw_state->keyboard.keys_pressed[KEY_CTRL];

    uint8_t sc = hw_state->keyboard.scancode;

    if (sc == KB_EXT_PREFIX) return;

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

    if (is_make && sc == KEY_TAB) {
        gui_handle_tab_navigation(gui, gui->input.shift_down);
        return;
    }

    if (gui->input.focused != NULL && gui->input.focused->on_key_press != NULL) {
        gui->input.focused->on_key_press(gui->input.focused, sc, ctx);
    }

    if (is_make && key_index == KEY_SPACE) {
        hw_state->keyboard.keys_pressed[KEY_SPACE] = false;
    }
}

void handle_mouse(hardware_t *hw_state, t_ctx *ctx) {
    t_gui *gui = &ctx->gui;

    if (!hw_mouse_ih(&hw_state->mouse)) return;

    gui->input.mouse_x = hw_state->mouse.x;
    gui->input.mouse_y = hw_state->mouse.y;

    bool is_pressed = hw_state->mouse.left_click;
    bool is_right_pressed = hw_state->mouse.right_click;
    t_widget *target = widget_get_at(gui_get_top_view(gui), gui->input.mouse_x, gui->input.mouse_y);

    t_widget **clicked = &gui->input.clicked_widget;
    t_widget **hovered = &gui->input.hovered;

    // Handle Left Click (GUI and Bombs)
    if (is_pressed) {
        if (!*clicked) {
            WIDGET_SET_CLICKED(gui, target);

            if (target != NULL) {
                if (WIDGET_CAN_RECEIVE_FOCUS(target)) {
                    gui_set_focus(gui, target);
                }

                if (target->on_press != NULL) {
                    target->on_press(target, ctx);
                }
            }
        } else {
            t_widget *active_drag = gui->drag.dragged_widget;

            if (active_drag != NULL && active_drag->on_drag != NULL) {
                active_drag->on_drag(active_drag, ctx);
            }
        }
    } else {
        if (*clicked != NULL) {
            if (*clicked == target && (*clicked)->on_click != NULL) {
                (*clicked)->on_click(*clicked, ctx);
            }

            WIDGET_SET_CLICKED(gui, NULL);
        }

        gui_end_drag(gui);
    }

    // Handle Right Click (Movement)
    if (is_right_pressed && target != NULL && target->type == GAME) {
        game_state_handle_click(
            &ctx->game,
            gui->input.mouse_x - target->abs_x,
            gui->input.mouse_y - target->abs_y,
            false
        );
    }

    if (gui->drag.dragged_widget == NULL && *hovered != target) {
        WIDGET_SET_HOVERED(gui, target);
    }
}

void handle_serial(hardware_t *hw_state, t_ctx *ctx) {
    (void)hw_state;
    app_multiplayer_poll_serial(ctx);
}
