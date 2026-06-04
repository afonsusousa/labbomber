#include "gui/gui.h"
#include "gui/widget.h"
#include "core/application.h"

static void _callback_pop_view(t_widget *self, void *state) {
    (void)self;
    gui_pop_view(GUI(state));
}

void gui_show_info_dialog(struct s_ctx *ctx, const char *title, const char *message) {
    t_gui *gui = &ctx->gui;
    t_widget *overlay = widget_create_overlay(gui->width, gui->height, _callback_pop_view, "info_overlay");
    if (overlay == NULL) return;

    t_widget *info_dialog = widget_add_dialog(overlay, title, 360, 190, gui->width, gui->height, _callback_pop_view, "info_dialog");
    info_dialog->on_quit = _callback_pop_view;

    widget_add_text(info_dialog, 0, 40, 320, 24, message, "info_message");

    t_widget *btn_ok = widget_add_button(info_dialog, 0, 86, 120, 36, "OK", _callback_pop_view, "info_ok_button");

    int32_t btn_row_x = ((int32_t)info_dialog->width - (int32_t)btn_ok->width) / 2;
    widget_set_position(btn_ok, btn_row_x, 120);

    gui_push_overlay(gui, overlay);
}

void gui_show_confirm_dialog(struct s_ctx *ctx, const char *title, const char *message, void (*on_yes)(t_widget*, void*), void (*on_no)(t_widget*, void*)) {
    t_gui *gui = &ctx->gui;
    t_widget *overlay = widget_create_overlay(gui->width, gui->height, on_no, "confirm_overlay");
    if (overlay == NULL) return;

    t_widget *confirm_dialog = widget_add_dialog(overlay, title, 360, 190, gui->width, gui->height, on_no, "confirm_dialog");
    confirm_dialog->on_quit = _callback_pop_view;

    widget_add_text(confirm_dialog, 0, 40, 320, 24, message, "confirm_message");

    t_widget *btn_yes = widget_add_button(confirm_dialog, 0, 86, 120, 36, "Yes", on_yes, "confirm_yes_button");
    t_widget *btn_no  = widget_add_button(confirm_dialog, 0, 86, 120, 36, "No",  on_no,  "confirm_no_button");

    int32_t btn_row_x = ((int32_t)confirm_dialog->width - (int32_t)(btn_yes->width + btn_no->width + 20)) / 2;
    widget_set_position(btn_yes, btn_row_x, 120);
    widget_set_position(btn_no,  btn_row_x + (int32_t)btn_yes->width + 20, 120);

    gui_push_overlay(gui, overlay);
}
