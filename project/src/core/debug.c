#include "gui/widget.h"
#include "gui/gui.h"
#include "view/draw.h"
#include <stdio.h>

static const char *widget_name_or_null(const t_widget *widget) {
    return (widget != NULL && widget->name != NULL) ? widget->name : "NULL";
}

static void draw_debug_line(hw_video_t *video, int32_t x, int32_t y, const char *label, const t_widget *widget) {
    char line[256];
    if (widget != NULL) {
        snprintf(line, sizeof(line), "%s: %s (%d,%d %ux%u)", label, widget_name_or_null(widget), widget->x, widget->y, widget->width, widget->height);
    } else {
        snprintf(line, sizeof(line), "%s: NULL", label);
    }

    draw_string(video, line, x, y, 0x000000);
}

void draw_debug_overlay(hw_video_t *video, const t_gui *gui, t_game_state game) {
    if (video == NULL || gui == NULL) return;

    int32_t x = 8;
    int32_t y = 8;
    char line[256];

    snprintf(line, sizeof(line), "Focus: %s", widget_name_or_null(gui->input.focused));
    draw_string(video, line, x, y, 0x000000);
    y += 14;

    draw_debug_line(video, x, y, "Hovered", gui->input.hovered);
    y += 14;

    draw_debug_line(video, x, y, "Clicked", gui->input.clicked_widget);
    y += 14;

    draw_debug_line(video, x, y, "Dragged", gui->drag.dragged_widget);
    y += 14;

    snprintf(line, sizeof(line), "Mouse: (%d,%d) Ctrl:%d Shift:%d",
             gui->input.mouse_x, gui->input.mouse_y,
             gui->input.ctrl_down ? 1 : 0,
             gui->input.shift_down ? 1 : 0);
    draw_string(video, line, x, y, 0x000000);

    y += 14;

    snprintf(line, sizeof(line), "Player Pos: (%d,%d)", game.players[0].pos.x, game.players[0].pos.y);
    draw_string(video, line, x, y, 0x000000);

    y += 14;

    snprintf(line, sizeof(line), "board pos: (%d,%d)", game.players[0].board_pos.x, game.players[0].board_pos.y);
    draw_string(video, line, x, y, 0x000000);

    y+=14;

    snprintf(line, sizeof(line), "Click Count: (%d)", game.click_count);
    draw_string(video, line, x, y, 0x000000);

    y+=14;

    snprintf(line, sizeof(line), "door_pos: (%d,%d)", game.door_pos.x, game.door_pos.y);
    draw_string(video, line, x, y, 0x000000);

    y += 14;

    snprintf(line, sizeof(line), "Logical Ticks: %d", game.logical_ticks);
    draw_string(video, line, x, y, 0x000000);
}
