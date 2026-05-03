#include "gui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void gui_set_focus(t_gui *gui, t_widget *widget) {
    if (gui == NULL) return;
    if (gui->focused != NULL) {
        WIDGET_SET_FOCUSED(gui->focused, false);
    }
    gui->focused = widget;
    if (gui->focused != NULL) {
        WIDGET_SET_FOCUSED(gui->focused, true);
    }
}

void gui_set_view(t_gui *gui, t_widget *view) {
    if (gui == NULL) return;
    gui->current_view = view;
    if (gui->current_view) {
        gui_set_focus(gui, widget_find_first_focusable(gui->current_view));
    }
}

void on_btn_singleplayer_click(t_widget *self, void *state) {
    t_gui *gui = (t_gui*)state;
    gui_set_view(gui, gui->single_name_menu);
}

void on_btn_multiplayer_click(t_widget *self, void *state) {
    t_gui *gui = (t_gui*)state;
    gui_set_view(gui, gui->multi_name_menu);
}

void on_btn_scoreboard_click(t_widget *self, void *state) {
    printf("Scoreboard Button Clicked!\n");
}

void on_btn_start_game_click(t_widget *self, void *state) {
    printf("Start Game Button Clicked!\n");
}

void on_text_input_click(t_widget *self, void *state) {
    t_gui *gui = (t_gui*)state;
    gui_set_focus(gui, self);
}

void on_view_quit(t_widget *self, void *state) {
    t_gui *gui = (t_gui*)state;
    gui_set_view(gui, gui->start_menu);
}

void gui_init(t_gui *gui, uint32_t screen_width, uint32_t screen_height) {
    if (gui == NULL) return;

    memset(gui, 0, sizeof(*gui));

    gui->start_menu = widget_create_start_menu(gui, screen_width, screen_height);
    gui->start_menu->on_quit = NULL; // Special case for start menu

    gui->single_name_menu = widget_create_name_menu(gui, screen_width, screen_height, false);
    gui->single_name_menu->on_quit = on_view_quit;

    gui->multi_name_menu = widget_create_name_menu(gui, screen_width, screen_height, true);
    gui->multi_name_menu->on_quit = on_view_quit;

    gui_set_view(gui, gui->start_menu);
}

void gui_destroy(t_gui *gui) {
    if (gui == NULL) return;
    widget_destroy(gui->start_menu);
    widget_destroy(gui->single_name_menu);
    widget_destroy(gui->multi_name_menu);
    memset(gui, 0, sizeof(*gui));
}

void widget_layout(t_widget *container, uint32_t spacing, uint32_t padding, bool is_vertical) {
    if (!container || !is_vertical) return; // Only vertical layout is supported for now

    // 1. Calculate total height of all children
    uint32_t total_children_h = 0;
    uint32_t child_count = 0;
    t_widget *child = container->children;
    while (child) {
        total_children_h += child->height;
        child_count++;
        child = child->next;
    }
    if (child_count > 1) {
        total_children_h += (child_count - 1) * spacing;
    }

    // 2. Determine starting Y position for vertical centering
    uint32_t current_y = (container->height > total_children_h) ? (container->height - total_children_h) / 2 : padding;

    // 3. Position each child
    child = container->children;
    while (child) {
        // For horizontal centering
        uint32_t child_x = (container->width > child->width) ? (container->width - child->width) / 2 : 0;
        widget_set_position(child, child_x, current_y);
        current_y += child->height + spacing;
        child = child->next;
    }
}

t_widget* widget_create_start_menu(t_gui *gui, uint32_t screen_width, uint32_t screen_height) {
    t_widget *menu = widget_create(CANVAS, 0, 0, screen_width, screen_height);
    if (!menu) return NULL;
    menu->data.canvas.state = gui;

    t_widget *btn_single = widget_create(BUTTON, 0, 0, 300, 50);
    btn_single->data.button.label = "Singleplayer";
    btn_single->on_click = on_btn_singleplayer_click;
    widget_add_child(menu, btn_single);

    t_widget *btn_multi = widget_create(BUTTON, 0, 0, 300, 50);
    btn_multi->data.button.label = "Multiplayer";
    btn_multi->on_click = on_btn_multiplayer_click;
    widget_add_child(menu, btn_multi);

    t_widget *btn_score = widget_create(BUTTON, 0, 0, 300, 50);
    btn_score->data.button.label = "Scoreboard";
    btn_score->on_click = on_btn_scoreboard_click;
    widget_add_child(menu, btn_score);

    widget_layout(menu, 30, 100, true);

    return menu;
}

t_widget* widget_create_name_menu(t_gui *gui, uint32_t screen_width, uint32_t screen_height, bool is_multiplayer) {
    t_widget *menu = widget_create(CANVAS, 0, 0, screen_width, screen_height);
    if (!menu) return NULL;
    menu->data.canvas.state = gui;

    t_widget *dlg_prompt = widget_create(DIALOG, 0, 0, 400, 300);
    dlg_prompt->data.dialog.title = is_multiplayer ? "Enter Player Names" : "Enter Player Name";
    dlg_prompt->data.canvas.state = gui;
    widget_add_child(menu, dlg_prompt);
    // Center the dialog on the screen
    uint32_t dlg_x = (screen_width > dlg_prompt->width) ? (screen_width - dlg_prompt->width) / 2 : 0;
    uint32_t dlg_y = (screen_height > dlg_prompt->height) ? (screen_height - dlg_prompt->height) / 2 : 0;
    widget_set_position(dlg_prompt, dlg_x, dlg_y);


    t_widget *input_p1 = widget_create(TEXT_INPUT, 0, 0, 300, 40);
    input_p1->data.text_input.buffer = (char*)malloc(256);
    memset(input_p1->data.text_input.buffer, 0, 256);
    strcpy(input_p1->data.text_input.buffer, "Player 1");
    input_p1->data.text_input.max_length = 255;
    input_p1->on_click = on_text_input_click;
    widget_add_child(dlg_prompt, input_p1);

    if (is_multiplayer) {
        t_widget *input_p2 = widget_create(TEXT_INPUT, 0, 0, 300, 40);
        input_p2->data.text_input.buffer = (char*)malloc(256);
        memset(input_p2->data.text_input.buffer, 0, 256);
        strcpy(input_p2->data.text_input.buffer, "Player 2");
        input_p2->data.text_input.max_length = 255;
        input_p2->on_click = on_text_input_click;
        widget_add_child(dlg_prompt, input_p2);
    }

    t_widget *btn_start = widget_create(BUTTON, 0, 0, 150, 40);
    btn_start->data.button.label = "Start";
    btn_start->on_click = on_btn_start_game_click;
    widget_add_child(dlg_prompt, btn_start);

    widget_layout(dlg_prompt, 20, 40, true);

    return menu;
}
