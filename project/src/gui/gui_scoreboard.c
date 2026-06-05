#include "gui/gui_scoreboard.h"
#include "gui/gui.h"
#include "gui/widget.h"
#include "core/application.h"
#include "scoreboard/scoreboard_controller.h"
#include <stdio.h>

static void _callback_close_scoreboard(t_widget *self, void *state) {
    (void)self;
    gui_pop_view(GUI(state));
}

static void _callback_clear_scoreboard(t_widget *self, void *state) {
    (void)self;
    scoreboard_init();
    scoreboard_save(SCOREBOARD_PATH);
    gui_pop_view(GUI(state));
}

void gui_show_scoreboard(struct s_ctx *ctx) {
    t_gui    *gui     = &ctx->gui;
    t_widget *overlay = widget_create_overlay(gui->width, gui->height, _callback_close_scoreboard, "scoreboard_overlay");

    if (overlay == NULL) return;

    t_widget *board = widget_add_dialog(overlay, "Scoreboard", 520, 460, gui->width, gui->height, _callback_close_scoreboard, "scoreboard_dialog");

    board->on_quit = _callback_close_scoreboard;

    const score_entry_t *entries = scoreboard_entries();
    uint32_t n = scoreboard_count();

    char line[80];
    char id[16];

    for (uint32_t i = 0; i < n; i++) {
        uint32_t secs = entries[i].duration_ticks / 60;

        snprintf(line, sizeof(line), "%u. %-6.6s.  %u pts  time %um%02us  %02u.%02u.%02u",
            i + 1,
            entries[i].player_name,
            entries[i].score,
            secs / 60, secs % 60,
            entries[i].day, entries[i].month, entries[i].year);

        snprintf(id, sizeof(id), "sb_row_%u", i + 1);
        
        widget_add_text(board, 0, 60 + (int32_t)(i * 30), 460, 20, line, id);
    }

    if (n == 0) widget_add_text(board, 0, 60, 450, 20, "No scores yet!", "sb_empty");

    widget_add_button(board, 0, 0, 150, 40, "Close", _callback_close_scoreboard, "scoreboard_close_button");
    widget_add_button(board, 0, 0, 150, 40, "Clear", _callback_clear_scoreboard,  "scoreboard_clear_button");

    widget_layout(board, 12, 40, true);
    gui_push_overlay(gui, overlay);
}
