#include "views.h"
#include "string.h"
#include "stdlib.h"
#include "../widget.h"

static void on_btn_start_game_click(t_widget *self, void *state) {
    (void)self;
    (void)state;

    // Pop the name menu
    gui_pop_view();

    // Push the game view
    t_widget *game_view = gui_init_game_view(g_gui.views.view_stack[0]->width, g_gui.views.view_stack[0]->height);
    gui_push_view(game_view);
}

static void on_text_input_click(t_widget *self, void *state) {
    (void)state;
    gui_set_focus(self);
}

static void on_view_quit(t_widget *self, void *state) {
    (void)self;
    (void)state;
    gui_pop_view();
}

static void on_dialog_close_click(t_widget *self, void *state) {
    (void)state;
    if (self == NULL || self->parent == NULL || self->parent->parent == NULL)
        return;

    t_widget *view = self->parent->parent;
    if (view->on_quit != NULL) {
        view->on_quit(view, NULL);
    }
}

t_widget* gui_init_name_menu(uint32_t screen_width, uint32_t screen_height, bool is_multiplayer) {
    t_widget *menu = widget_create(CANVAS, 0, 0, screen_width, screen_height);
    if (!menu) return NULL;

    // Dim the background
    menu->flags &= ~WIDGET_FLAG_ACTIVE; // Temporarily unset active so it doesn't catch clicks on the clear background
    // wait we cant unset active or it won't draw. We need to just not give it click handlers.
    WIDGET_SET_ACTIVE(menu, true);

    t_widget *dlg_prompt = widget_create(DIALOG, 0, 0, 400, 300);
    dlg_prompt->data.dialog.title = is_multiplayer ? "Enter Player Names" : "Enter Player Name";
    dlg_prompt->on_press = on_dialog_press;
    dlg_prompt->on_drag = on_dialog_drag;
    widget_add_child(menu, dlg_prompt);

    uint32_t dlg_x = (screen_width > dlg_prompt->width) ? (screen_width - dlg_prompt->width) / 2 : 0;
    uint32_t dlg_y = (screen_height > dlg_prompt->height) ? (screen_height - dlg_prompt->height) / 2 : 0;
    widget_set_position(dlg_prompt, dlg_x, dlg_y);

    t_widget *input_p1 = widget_create(TEXT_INPUT, 0, 0, 300, 40);
    input_p1->data.text_input.buffer = (char*)malloc(256);
    memset(input_p1->data.text_input.buffer, 0, 256);
    strcpy(input_p1->data.text_input.buffer, "Player 1");
    input_p1->data.text_input.max_length = 255;
    input_p1->on_click = on_text_input_click;

    // Free the buffer when the input is destroyed
    input_p1->on_destroy = (void (*)(struct s_widget*)) free;

    widget_add_child(dlg_prompt, input_p1);

    if (is_multiplayer) {
        t_widget *input_p2 = widget_create(TEXT_INPUT, 0, 0, 300, 40);
        input_p2->data.text_input.buffer = (char*)malloc(256);
        memset(input_p2->data.text_input.buffer, 0, 256);
        strcpy(input_p2->data.text_input.buffer, "Player 2");
        input_p2->data.text_input.max_length = 255;
        input_p2->on_click = on_text_input_click;
        // Free the buffer when the input is destroyed
        input_p2->on_destroy = (void (*)(struct s_widget*)) free;

        widget_add_child(dlg_prompt, input_p2);
    }

    t_widget *btn_start = widget_create(BUTTON, 0, 0, 150, 40);
    btn_start->data.button.label = "Start";
    btn_start->on_click = on_btn_start_game_click;
    widget_add_child(dlg_prompt, btn_start);
    widget_layout(dlg_prompt, 20, 40, true);

    t_widget *btn_close = widget_create(BUTTON, 0, 0, 16, 16);
    btn_close->data.button.label = "X";
    btn_close->on_click = on_dialog_close_click;
    widget_add_child(dlg_prompt, btn_close);
    widget_set_position(btn_close, (int32_t)dlg_prompt->width - 22, 6);
    dlg_prompt->data.dialog.close_button = btn_close;

    menu->on_quit = on_view_quit;
    return menu;
}
