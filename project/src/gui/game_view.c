#include "views.h"
#include "../widgets/widget.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void on_pause_reset_confirm_click(t_widget *self, void *state);
static void on_pause_main_menu_confirm_click(t_widget *self, void *state);

static void on_game_canvas_press(t_widget *self, void *state) {
    t_gui *gui = (t_gui*)state;
    t_game_state *game = (t_game_state*)self->data.canvas.state;
    int32_t click_x = gui->input.mouse_x - widget_get_abs_x(self);
    int32_t click_y = gui->input.mouse_y - widget_get_abs_y(self);

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

static void on_game_canvas_drag(t_widget *self, void *state) {
    on_game_canvas_press(self, state);
}

static void free_game_state(t_widget *self) {
    t_game_state *game_state = (t_game_state*)self->data.canvas.state;
    if (game_state) {
        free(game_state->pixel_buffer);
        free(game_state);
    }
}

static void on_game_view_quit(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = (t_gui*)state;

    // Route to currently focused widget if it has a quit handler
    if (gui->input.focused != NULL && gui->input.focused->on_quit != NULL) {
        gui->input.focused->on_quit(gui->input.focused, state);
        return;
    }

    // Push the pause menu!
    t_widget *pause_menu = gui_init_pause_menu(gui, gui->views.view_stack[0]->width, gui->views.view_stack[0]->height);
    gui_push_overlay(gui, pause_menu);
}


static void on_pause_dialog_quit(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = (t_gui*)state;
    gui_pop_view(gui);
}


static void on_pause_resume_click(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = (t_gui*)state;
    gui_pop_view(gui);
}

static void on_confirm_cancel_click(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = (t_gui*)state;
    gui_pop_view(gui);
}

static void on_pause_main_menu_confirm_click(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = (t_gui*)state;
    gui_pop_view(gui); // pop confirm
    gui_pop_view(gui); // pop pause
    gui_pop_view(gui); // pop game
}

static void on_pause_reset_click(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = (t_gui*)state;
    t_widget *confirm = gui_init_confirm_menu(gui, gui->views.view_stack[0]->width, gui->views.view_stack[0]->height, "Confirm Reset", "Are you sure?", on_pause_reset_confirm_click, on_confirm_cancel_click);
    gui_push_overlay(gui, confirm);
}

static void on_pause_reset_confirm_click(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = (t_gui*)state;

    // Look for the game view from the bottom of the stack up
    for (int i = 0; i < gui->views.view_count; i++) {
        t_widget *view = gui->views.view_stack[i];
        // The game view has a specific structure: its second child is the game canvas
        if (view && view->children && view->children->next && view->children->next->draw == draw_game_canvas) {
            t_widget *game_canvas = view->children->next;
            t_game_state *game = (t_game_state*)game_canvas->data.canvas.state;
            if (game && game->pixel_buffer) {
                memset(game->pixel_buffer, 0, sizeof(uint16_t) * game->width * game->height);
            }
            break;
        }
    }

    gui_pop_view(gui); // pop confirm
    gui_pop_view(gui); // pop pause
}

static void on_pause_main_menu_click(t_widget *self, void *state) {
    (void)self;
    t_gui *gui = (t_gui*)state;
    t_widget *confirm = gui_init_confirm_menu(gui, gui->views.view_stack[0]->width, gui->views.view_stack[0]->height, "Confirm Main Menu", "Return to main menu?", on_pause_main_menu_confirm_click, on_confirm_cancel_click);
    gui_push_overlay(gui, confirm);
}

t_widget* gui_init_game_view(t_gui *gui, uint32_t screen_width, uint32_t screen_height) {
    (void)gui;
    t_widget *view = widget_create(CANVAS, 0, 0, screen_width, screen_height);
    if (!view) return NULL;

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
    game_canvas->on_destroy = free_game_state; // Free the state when the view is popped

    widget_add_child(view, game_canvas);

    view->on_quit = on_game_view_quit;
    return view;
}

t_widget* gui_init_pause_menu(t_gui *gui, uint32_t screen_width, uint32_t screen_height) {
    (void)gui;
    t_widget *overlay = widget_create_overlay(screen_width, screen_height, on_pause_dialog_quit);
    t_widget *pause_dialog = widget_add_dialog(overlay, "Paused", 360, 260, screen_width, screen_height, on_pause_resume_click);
    pause_dialog->on_quit = on_pause_dialog_quit;

    widget_add_button(pause_dialog, 0, 0, 220, 40, "Resume", on_pause_resume_click);
    widget_add_button(pause_dialog, 0, 0, 220, 40, "Reset", on_pause_reset_click);
    widget_add_button(pause_dialog, 0, 0, 220, 40, "Main Menu", on_pause_main_menu_click);

    widget_layout(pause_dialog, 16, 48, true);

    return overlay;
}

t_widget* gui_init_confirm_menu(t_gui *gui, uint32_t screen_width, uint32_t screen_height, const char *title, const char *message, void (*on_yes)(t_widget*, void*), void (*on_no)(t_widget*, void*)) {
    (void)gui;
    t_widget *overlay = widget_create_overlay(screen_width, screen_height, on_confirm_cancel_click);
    t_widget *confirm_dialog = widget_add_dialog(overlay, title, 360, 190, screen_width, screen_height, on_no);
    confirm_dialog->on_quit = on_confirm_cancel_click;

    widget_add_text(confirm_dialog, 0, 40, 320, 24, message);

    t_widget *btn_yes = widget_add_button(confirm_dialog, 0, 86, 120, 36, "Yes", on_yes);
    t_widget *btn_no = widget_add_button(confirm_dialog, 0, 86, 120, 36, "No", on_no);

    int32_t btn_row_x = ((int32_t)confirm_dialog->width - (int32_t)(btn_yes->width + btn_no->width + 20)) / 2;
    widget_set_position(btn_yes, btn_row_x, 120);
    widget_set_position(btn_no, btn_row_x + (int32_t)btn_yes->width + 20, 120);

    return overlay;
}
