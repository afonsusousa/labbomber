#include "views.h"
#include "../widget.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Forward declarations
void widget_layout(t_widget *container, uint32_t spacing, uint32_t padding, bool is_vertical);

static void on_pause_reset_confirm_click(t_widget *self);
static void on_pause_main_menu_confirm_click(t_widget *self);

static void on_game_canvas_press(t_widget *self) {
    t_game_state *game = (t_game_state*)self->data.canvas.state;
    int32_t click_x = g_gui.input.mouse_x - widget_get_abs_x(self);
    int32_t click_y = g_gui.input.mouse_y - widget_get_abs_y(self);

    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            int32_t px = click_x + x;
            int32_t py = click_y + y;
            if (px >= 0 && px < (int32_t)game->width && py >= 0 && py < (int32_t)game->height) {
                game->pixel_buffer[py * game->width + px] = 0xFFFF;
            }
        }
    }
}

static void on_game_canvas_drag(t_widget *self) {
    on_game_canvas_press(self);
}

static void on_game_view_quit(t_widget *self) {
    (void)self;

    // Route to currently focused widget if it has a quit handler
    if (g_gui.input.focused != NULL && g_gui.input.focused->on_quit != NULL) {
        g_gui.input.focused->on_quit(g_gui.input.focused);
        return;
    }

    // Default view quit behavior (if no focused widget handled it)
    if (g_gui.dialogs.confirm_overlay != NULL && WIDGET_IS_ACTIVE(g_gui.dialogs.confirm_overlay)) {
        gui_set_active(g_gui.dialogs.confirm_overlay, false);
    } else if (g_gui.dialogs.pause_overlay != NULL && !WIDGET_IS_ACTIVE(g_gui.dialogs.pause_overlay)) {
        gui_set_active(g_gui.dialogs.pause_overlay, true);
        gui_set_focus(g_gui.dialogs.pause_dialog);
    }
}

static void on_pause_dialog_quit(t_widget *self) {
    (void)self;
    if (g_gui.dialogs.pause_overlay != NULL) {
        gui_set_active(g_gui.dialogs.pause_overlay, false);
    }
}

static void on_confirm_dialog_quit(t_widget *self) {
    (void)self;
    if (g_gui.dialogs.confirm_overlay != NULL) {
        gui_set_active(g_gui.dialogs.confirm_overlay, false);
    }
}

static void on_pause_resume_click(t_widget *self) {
    (void)self;
    if (g_gui.dialogs.confirm_overlay != NULL) {
        gui_set_active(g_gui.dialogs.confirm_overlay, false);
    }
    if (g_gui.dialogs.pause_overlay != NULL) {
        gui_set_active(g_gui.dialogs.pause_overlay, false);
    }
}

static void on_pause_confirm_cancel_click(t_widget *self) {
    (void)self;
    if (g_gui.dialogs.confirm_overlay != NULL) {
        gui_set_active(g_gui.dialogs.confirm_overlay, false);
    }
}

static void show_pause_confirm_dialog(const char *title, const char *message, void (*on_yes)(t_widget*)) {
    if (!g_gui.dialogs.confirm_dialog || !g_gui.dialogs.confirm_text || !g_gui.dialogs.confirm_yes || !g_gui.dialogs.confirm_no || !g_gui.dialogs.confirm_overlay) return;
    g_gui.dialogs.confirm_dialog->data.dialog.title = (char*)title;
    g_gui.dialogs.confirm_text->data.text_display.text = (char*)message;
    g_gui.dialogs.confirm_yes->on_click = on_yes;
    g_gui.dialogs.confirm_no->on_click = on_pause_confirm_cancel_click;
    gui_set_active(g_gui.dialogs.confirm_overlay, true);
    gui_set_focus(g_gui.dialogs.confirm_dialog);
}

static void on_pause_main_menu_confirm_click(t_widget *self) {
    (void)self;
    if (g_gui.dialogs.confirm_overlay != NULL) gui_set_active(g_gui.dialogs.confirm_overlay, false);
    if (g_gui.dialogs.pause_overlay != NULL) gui_set_active(g_gui.dialogs.pause_overlay, false);
    gui_set_view(g_gui.views.start_menu);
}

static void on_pause_reset_click(t_widget *self) {
    (void)self;
    show_pause_confirm_dialog("Confirm Reset", "Are you sure?", on_pause_reset_confirm_click);
}

static void on_pause_reset_confirm_click(t_widget *self) {
    (void)self;
    if (g_gui.views.game_view && g_gui.views.game_view->children && g_gui.views.game_view->children->next) {
        t_widget *game_canvas = g_gui.views.game_view->children->next;
        t_game_state *game = (t_game_state*)game_canvas->data.canvas.state;
        if (game && game->pixel_buffer) {
            memset(game->pixel_buffer, 0, sizeof(uint16_t) * game->width * game->height);
        }
    }
    if (g_gui.dialogs.pause_overlay != NULL) gui_set_active(g_gui.dialogs.pause_overlay, false);
    if (g_gui.dialogs.confirm_overlay != NULL) gui_set_active(g_gui.dialogs.confirm_overlay, false);
}

static void on_pause_main_menu_click(t_widget *self) {
    (void)self;
    show_pause_confirm_dialog("Confirm Main Menu", "Return to main menu?", on_pause_main_menu_confirm_click);
}

void gui_init_game_view(uint32_t screen_width, uint32_t screen_height) {
    t_widget *view = widget_create(CANVAS, 0, 0, screen_width, screen_height);
    if (!view) return;

    t_widget *status_bar = widget_create(CANVAS, 0, 0, screen_width, 40);
    widget_add_child(view, status_bar);

    uint32_t canvas_h = screen_height - 40;
    t_widget *game_canvas = widget_create(CANVAS, 0, 40, screen_width, canvas_h);
    game_canvas->draw = draw_game_canvas;
    game_canvas->on_press = on_game_canvas_press;
    game_canvas->on_drag = on_game_canvas_drag;

    t_game_state *game_state = (t_game_state*)malloc(sizeof(t_game_state));
    game_state->width = screen_width;
    game_state->height = canvas_h;
    game_state->pixel_buffer = (uint16_t*)malloc(sizeof(uint16_t) * screen_width * canvas_h);
    memset(game_state->pixel_buffer, 0, sizeof(uint16_t) * screen_width * canvas_h);
    game_canvas->data.canvas.state = game_state;
    widget_add_child(view, game_canvas);

    t_widget *pause_overlay = widget_create(CANVAS, 0, 0, screen_width, screen_height);
    pause_overlay->draw = NULL;
    gui_set_active(pause_overlay, false);
    widget_add_child(view, pause_overlay);

    t_widget *pause_dialog = widget_create(DIALOG, 0, 0, 360, 260);
    pause_dialog->data.dialog.title = "Paused";
    pause_dialog->on_press = on_dialog_press;
    pause_dialog->on_drag = on_dialog_drag;
    pause_dialog->on_quit = on_pause_dialog_quit;
    widget_add_child(pause_overlay, pause_dialog);

    uint32_t dlg_x = (screen_width - pause_dialog->width) / 2;
    uint32_t dlg_y = (screen_height - pause_dialog->height) / 2;
    widget_set_position(pause_dialog, dlg_x, dlg_y);

    t_widget *btn_resume = widget_create(BUTTON, 0, 0, 220, 40);
    btn_resume->data.button.label = "Resume";
    btn_resume->on_click = on_pause_resume_click;
    widget_add_child(pause_dialog, btn_resume);

    t_widget *btn_reset = widget_create(BUTTON, 0, 0, 220, 40);
    btn_reset->data.button.label = "Reset";
    btn_reset->on_click = on_pause_reset_click;
    widget_add_child(pause_dialog, btn_reset);

    t_widget *btn_main_menu = widget_create(BUTTON, 0, 0, 220, 40);
    btn_main_menu->data.button.label = "Main Menu";
    btn_main_menu->on_click = on_pause_main_menu_click;
    widget_add_child(pause_dialog, btn_main_menu);

    widget_layout(pause_dialog, 16, 48, true);

    t_widget *btn_close = widget_create(BUTTON, 0, 0, 16, 16);
    btn_close->data.button.label = "X";
    btn_close->on_click = on_pause_resume_click;
    widget_add_child(pause_dialog, btn_close);
    widget_set_position(btn_close, (int32_t)pause_dialog->width - 22, 6);
    pause_dialog->data.dialog.close_button = btn_close;

    t_widget *confirm_overlay = widget_create(CANVAS, 0, 0, screen_width, screen_height);
    confirm_overlay->draw = NULL;
    gui_set_active(confirm_overlay, false);
    widget_add_child(pause_overlay, confirm_overlay);

    t_widget *confirm_dialog = widget_create(DIALOG, 0, 0, 360, 190);
    confirm_dialog->data.dialog.title = "Confirm";
    confirm_dialog->on_press = on_dialog_press;
    confirm_dialog->on_drag = on_dialog_drag;
    confirm_dialog->on_quit = on_confirm_dialog_quit;
    widget_add_child(confirm_overlay, confirm_dialog);

    uint32_t confirm_x = (screen_width - confirm_dialog->width) / 2;
    uint32_t confirm_y = (screen_height - confirm_dialog->height) / 2;
    widget_set_position(confirm_dialog, confirm_x, confirm_y);

    t_widget *txt_confirm = widget_create(TEXT, 0, 40, 320, 24);
    txt_confirm->data.text_display.text = "Are you sure?";
    widget_add_child(confirm_dialog, txt_confirm);

    t_widget *btn_yes = widget_create(BUTTON, 0, 86, 120, 36);
    btn_yes->data.button.label = "Yes";
    btn_yes->on_click = on_pause_reset_confirm_click;
    widget_add_child(confirm_dialog, btn_yes);

    t_widget *btn_no = widget_create(BUTTON, 0, 86, 120, 36);
    btn_no->data.button.label = "No";
    btn_no->on_click = on_pause_confirm_cancel_click;
    widget_add_child(confirm_dialog, btn_no);

    int32_t btn_row_x = ((int32_t)confirm_dialog->width - (int32_t)(btn_yes->width + btn_no->width + 20)) / 2;
    widget_set_position(btn_yes, btn_row_x, 120);
    widget_set_position(btn_no, btn_row_x + (int32_t)btn_yes->width + 20, 120);

    t_widget *btn_confirm_close = widget_create(BUTTON, 0, 0, 16, 16);
    btn_confirm_close->data.button.label = "X";
    btn_confirm_close->on_click = on_pause_confirm_cancel_click;
    widget_add_child(confirm_dialog, btn_confirm_close);
    widget_set_position(btn_confirm_close, (int32_t)confirm_dialog->width - 22, 6);
    confirm_dialog->data.dialog.close_button = btn_confirm_close;

    g_gui.dialogs.pause_overlay = pause_overlay;
    g_gui.dialogs.confirm_overlay = confirm_overlay;
    g_gui.dialogs.confirm_dialog = confirm_dialog;
    g_gui.dialogs.confirm_text = txt_confirm;
    g_gui.dialogs.confirm_yes = btn_yes;
    g_gui.dialogs.confirm_no = btn_no;

    view->on_quit = on_game_view_quit;
    g_gui.views.game_view = view;
}
