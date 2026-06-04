#include "hardware.h"
#include "gui.h"
#include "application.h"
#include "widget.h"
#include "draw.h"
#include "../lib/rtc/rtc.h"
#include "../lib/serialPort/serial_port.h"
#include "../lib/keyboard/i8042.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

void draw_debug_overlay(hw_video_t *video, const t_gui *gui, t_game_state game);

#define MP_PACKET_START        0xA5
#define MP_PACKET_HELLO        0x01
#define MP_PACKET_KEY          0x02
#define MP_PACKET_PLAYER_STATE 0x03
#define MP_PACKET_PAUSE        0x04
#define MP_PACKET_PAYLOAD_SIZE 3

static int app_multiplayer_send_packet(uint8_t type, uint8_t data0, uint8_t data1, uint8_t data2) {
    if (serial_send_byte(MP_PACKET_START) != 0) return 1;
    if (serial_send_byte(type) != 0) return 1;
    if (serial_send_byte(data0) != 0) return 1;
    if (serial_send_byte(data1) != 0) return 1;
    return serial_send_byte(data2);
}

static void app_multiplayer_log(t_ctx *ctx, const char *message) {
    FILE *log_file = fopen("/tmp/game_debug.log", "a");
    if (log_file == NULL) return;

    fprintf(log_file,
            "[MP] %s local=%u remote=%u ready=%d role=%d nonce=%u remote_nonce=%u\n",
            message,
            ctx != NULL ? ctx->multiplayer_local_player : 255,
            ctx != NULL ? ctx->multiplayer_remote_player : 255,
            ctx != NULL && ctx->multiplayer_partner_ready,
            ctx != NULL && ctx->multiplayer_role_assigned,
            ctx != NULL ? ctx->multiplayer_local_nonce : 0,
            ctx != NULL ? ctx->multiplayer_remote_nonce : 0);
    fclose(log_file);
}

static void app_multiplayer_assign_roles(t_ctx *ctx) {
    if (ctx == NULL) return;

    if (ctx->multiplayer_local_nonce > ctx->multiplayer_remote_nonce ||
        (ctx->multiplayer_local_nonce == ctx->multiplayer_remote_nonce &&
         ctx->multiplayer_local_tiebreaker >= ctx->multiplayer_remote_tiebreaker)) {
        ctx->multiplayer_local_player = PLAYER_1;
        ctx->multiplayer_remote_player = PLAYER_2;
    } else {
        ctx->multiplayer_local_player = PLAYER_2;
        ctx->multiplayer_remote_player = PLAYER_1;
    }

    ctx->game.current_player = ctx->multiplayer_local_player;
    ctx->multiplayer_role_assigned = true;
    ctx->multiplayer_partner_ready = true;
    app_multiplayer_log(ctx, "roles assigned (RTC selection fallback ready)");
}

static void app_multiplayer_process_packet(t_ctx *ctx) {
    if (ctx == NULL) return;

    switch (ctx->multiplayer_rx_type) {
        case MP_PACKET_HELLO:
            ctx->multiplayer_remote_nonce =
                (uint16_t)ctx->multiplayer_rx_data[0] |
                ((uint16_t)ctx->multiplayer_rx_data[1] << 8);
            ctx->multiplayer_remote_tiebreaker = ctx->multiplayer_rx_data[2];
            app_multiplayer_assign_roles(ctx);
            break;
        case MP_PACKET_KEY: {
            uint8_t player_id = ctx->multiplayer_rx_data[0];
            uint8_t scancode = ctx->multiplayer_rx_data[1];
            FILE *log_file = fopen("/tmp/game_debug.log", "a");
            if (log_file) {
                fprintf(log_file, "[MP] rx key player=%u scancode=0x%02X\n", player_id, scancode);
                fclose(log_file);
            }
            if (ctx->multiplayer_role_assigned && player_id != ctx->multiplayer_local_player) {
                game_state_handle_player_key(&ctx->game, player_id, scancode);
            }
            break;
        }
        case MP_PACKET_PLAYER_STATE: {
            uint8_t player_id = ctx->multiplayer_rx_data[0];
            uint8_t state_value = ctx->multiplayer_rx_data[1];
            uint8_t lives = state_value & 0x7F;
            bool active = (state_value & 0x80) != 0;
            FILE *log_file = fopen("/tmp/game_debug.log", "a");
            if (log_file) {
                fprintf(log_file, "[MP] rx state player=%u lives=%u active=%u\n", player_id, lives, active);
                fclose(log_file);
            }
            if (ctx->multiplayer_role_assigned && player_id != ctx->multiplayer_local_player && player_id < MAX_PLAYERS) {
                player_t *player = &ctx->game.players[player_id];
                player->lives = lives;
                player->active = (lives > 0);
            }
            break;
        }
        case MP_PACKET_PAUSE: {
            bool paused = ctx->multiplayer_rx_data[0] != 0;
            if (ctx->game.match_state == MATCH_RUNNING || ctx->game.match_state == MATCH_PAUSED) {
                ctx->game.is_frozen = paused;
                ctx->game.match_state = paused ? MATCH_PAUSED : MATCH_RUNNING;
            }
            break;
        }
        default:
            break;
    }
}

static bool app_is_blank_string(const char *s) {
    if (s == NULL) return true;
    while (*s != '\0') {
        if (*s != ' ' && *s != '\t' && *s != '\n' && *s != '\r' && *s != '\f' && *s != '\v') {
            return false;
        }
        s++;
    }
    return true;
}

static bool app_multiplayer_name_inputs_ready(t_ctx *ctx) {
    if (ctx == NULL || !ctx->is_multiplayer || !ctx->multiplayer_role_assigned) return false;

    t_widget *top = gui_get_top_view(&ctx->gui);
    if (top == NULL || top->type != OVERLAY || top->name == NULL || strcmp(top->name, "name_overlay") != 0) {
        return false;
    }

    t_widget *p1_input = widget_find_by_name(&ctx->gui, "player1_input");
    t_widget *p2_input = widget_find_by_name(&ctx->gui, "player2_input");
    if (p1_input == NULL || p1_input->data.text_input.buffer == NULL) return false;
    if (p2_input == NULL || p2_input->data.text_input.buffer == NULL) return false;

    return !app_is_blank_string(p1_input->data.text_input.buffer) &&
           !app_is_blank_string(p2_input->data.text_input.buffer);
}

static void app_multiplayer_receive_byte(t_ctx *ctx, uint8_t byte) {
    if (ctx == NULL) return;

    if (ctx->multiplayer_rx_state == 0 && byte == 0xAA) {
        app_multiplayer_log(ctx, "legacy hello byte seen");
        return;
    }

    switch (ctx->multiplayer_rx_state) {
        case 0:
            if (byte == MP_PACKET_START) {
                ctx->multiplayer_rx_state = 1;
                ctx->multiplayer_rx_pos = 0;
            }
            break;
        case 1:
            ctx->multiplayer_rx_type = byte;
            ctx->multiplayer_rx_state = 2;
            break;
        case 2:
            ctx->multiplayer_rx_data[ctx->multiplayer_rx_pos++] = byte;
            if (ctx->multiplayer_rx_pos >= MP_PACKET_PAYLOAD_SIZE) {
                app_multiplayer_process_packet(ctx);
                ctx->multiplayer_rx_state = 0;
                ctx->multiplayer_rx_pos = 0;
            }
            break;
        default:
            ctx->multiplayer_rx_state = 0;
            ctx->multiplayer_rx_pos = 0;
            break;
    }
}

int app_multiplayer_send_hello(t_ctx *ctx) {
    if (ctx == NULL) return 1;

    int result = app_multiplayer_send_packet(
        MP_PACKET_HELLO,
        (uint8_t)(ctx->multiplayer_local_nonce & 0xFF),
        (uint8_t)(ctx->multiplayer_local_nonce >> 8),
        ctx->multiplayer_local_tiebreaker
    );
    serial_send_byte(0xAA);
    app_multiplayer_log(ctx, result == 0 ? "hello sent" : "hello send failed");
    return result;
}

int app_multiplayer_send_key(t_ctx *ctx, uint8_t scancode) {
    if (ctx == NULL || !ctx->is_multiplayer || !ctx->multiplayer_role_assigned) return 1;

    uint8_t key_index = MAKE_FROM_BREAK(scancode);
    if (key_index != KEY_W && key_index != KEY_A && key_index != KEY_D &&
        key_index != KEY_S && key_index != KEY_E) {
        return 0;
    }

    int result = app_multiplayer_send_packet(MP_PACKET_KEY, ctx->multiplayer_local_player, scancode, 0);
    FILE *log_file = fopen("/tmp/game_debug.log", "a");
    if (log_file) {
        fprintf(log_file, "[MP] tx key player=%u scancode=0x%02X result=%d\n",
                ctx->multiplayer_local_player, scancode, result);
        fclose(log_file);
    }
    return result;
}

void app_multiplayer_poll_serial(t_ctx *ctx) {
    if (ctx == NULL || !ctx->is_multiplayer) return;

    for (int i = 0; i < 32 && serial_has_byte(); i++) {
        uint8_t received;
        if (serial_read_byte(&received) == 0) {
            app_multiplayer_receive_byte(ctx, received);
        }
    }
}

int app_multiplayer_send_player_state(t_ctx *ctx, uint8_t player_id) {
    if (ctx == NULL || !ctx->is_multiplayer || !ctx->multiplayer_role_assigned) return 1;
    if (player_id >= MAX_PLAYERS) return 1;

    player_t *player = &ctx->game.players[player_id];
    uint8_t state_value = player->lives & 0x7F;
    if (player->active) state_value |= 0x80;

    int result = app_multiplayer_send_packet(MP_PACKET_PLAYER_STATE, player_id, state_value, 0);
    FILE *log_file = fopen("/tmp/game_debug.log", "a");
    if (log_file) {
        fprintf(log_file, "[MP] tx state player=%u lives=%u active=%u result=%d\n",
                player_id, player->lives, player->active, result);
        fclose(log_file);
    }
    return result;
}

int app_multiplayer_send_pause(t_ctx *ctx, bool paused) {
    if (ctx == NULL || !ctx->is_multiplayer || !ctx->multiplayer_role_assigned) return 1;

    int result = app_multiplayer_send_packet(MP_PACKET_PAUSE, paused ? 1 : 0, 0, 0);
    FILE *log_file = fopen("/tmp/game_debug.log", "a");
    if (log_file) {
        fprintf(log_file, "[MP] tx pause paused=%u result=%d\n", paused, result);
        fclose(log_file);
    }
    return result;
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


void handle_timer(hardware_t *hw_state, t_ctx *ctx) {
    t_gui *gui = &ctx->gui;
    static int handshake_retry_delay = 0;
    hw_timer_int_handler(&hw_state->timer);

    app_multiplayer_poll_serial(ctx);

    if (app_multiplayer_name_inputs_ready(ctx)) {
        gui_pop_view(&ctx->gui);
        app_update_real_time(ctx);
        gui_show_game_view(ctx);
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
            player->active != ctx->multiplayer_last_player_active[local_player]) {
            app_multiplayer_send_player_state(ctx, local_player);
            ctx->multiplayer_last_player_lives[local_player] = player->lives;
            ctx->multiplayer_last_player_active[local_player] = player->active;
        }
    }

    if (gui->input.hovered != NULL && gui->input.hovered->type == TEXT_INPUT) {
        draw_text_cursor(&hw_state->video, &hw_state->mouse);
    } 
    else {
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
