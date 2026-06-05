#include "multiplayer/multiplayer.h"
#include "mp_protocol.h"
#include "core/application.h"
#include "gui/gui.h"
#include "gui/widget.h"
#include "game/game.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// role assignment

void app_multiplayer_assign_roles(t_ctx *ctx) {
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
    mp_log(ctx, "roles assigned");

    t_widget *wait_conn = widget_find_by_name(&ctx->gui, "wait_conn_overlay");
    if (wait_conn != NULL) {
        gui_pop_view(&ctx->gui);
        gui_show_name_menu(ctx, true);
    }
}

// seed generator

static uint32_t mp_make_match_seed(t_ctx *ctx) {
    uint32_t a = ctx->multiplayer_local_nonce;
    uint32_t b = ctx->multiplayer_remote_nonce;

    uint32_t low = a < b ? a : b;
    uint32_t high = a < b ? b : a;

    uint32_t seed = low * 1103515245u + high * 12345u + 0xB00B5u;
    seed &= 0x00FFFFFF;
    if (seed == 0) seed = 1;
    return seed;
}

// game start sequencing 

static void mp_queue_start_game(t_ctx *ctx, uint32_t seed) {
    if (ctx == NULL || ctx->multiplayer_game_started) return;

    seed &= 0x00FFFFFF;
    ctx->multiplayer_match_seed = seed;
    ctx->game.enemy_seed = seed;
    ctx->multiplayer_start_game_pending = true;
    mp_log(ctx, "start game queued");
}

void app_multiplayer_start_pending_game(t_ctx *ctx) {
    if (ctx == NULL) return;
    if (!ctx->multiplayer_start_game_pending || ctx->multiplayer_game_started) return;
    if (widget_find_by_name(&ctx->gui, "game_view") != NULL) return;

    ctx->multiplayer_start_game_pending = false;
    ctx->multiplayer_game_started = true;

    t_widget *top = gui_get_top_view(&ctx->gui);
    if (top != NULL && top->name != NULL && strcmp(top->name, "info_overlay") == 0) {
        gui_pop_view(&ctx->gui);
    }

    gui_show_game_view(ctx);
}

void app_multiplayer_try_start_game(t_ctx *ctx) {
    if (ctx == NULL || !ctx->is_multiplayer) return;
    if (ctx->multiplayer_game_started || ctx->multiplayer_start_game_pending) return;
    if (!ctx->multiplayer_name_received || !ctx->multiplayer_remote_start_ready || !ctx->multiplayer_local_start_ready) return;

    uint32_t seed = mp_make_match_seed(ctx);
    ctx->multiplayer_start_game_sent = true;
    app_multiplayer_send_start_game(ctx, seed);
    mp_queue_start_game(ctx, seed);
}
