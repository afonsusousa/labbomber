#include "gui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void on_btn_singleplayer_click(t_widget *self) {
    t_gui *gui = (t_gui*)self->parent->data.canvas.state;
    if (gui->single_name_menu != NULL) {
        gui->current_menu = gui->single_name_menu;
    }
}

void on_btn_multiplayer_click(t_widget *self) {
    t_gui *gui = (t_gui*)self->parent->data.canvas.state;
    if (gui->multi_name_menu != NULL) {
        gui->current_menu = gui->multi_name_menu;
    }
}

void on_btn_scoreboard_click(t_widget *self) {
    printf("Scoreboard Button Clicked!\n");
}

void on_btn_start_game_click(t_widget *self) {
    printf("Start Game Button Clicked!\n");
}

void on_text_input_click(t_widget *self) {
    printf("Text Input Clicked!\n");
}

void gui_init(t_gui *gui, uint32_t screen_width, uint32_t screen_height) {
    gui->start_menu = widget_create_start_menu(gui, screen_width, screen_height);
    gui->single_name_menu = widget_create_name_menu(gui, screen_width, screen_height, false);
    gui->multi_name_menu = widget_create_name_menu(gui, screen_width, screen_height, true);
    gui->current_menu = gui->start_menu;
    gui->hovered_widget = NULL;
}

void gui_destroy(t_gui *gui) {
    if (gui->start_menu != NULL) widget_destroy(gui->start_menu);
    if (gui->single_name_menu != NULL) widget_destroy(gui->single_name_menu);
    if (gui->multi_name_menu != NULL) widget_destroy(gui->multi_name_menu);
}

void widget_layout(t_widget *widget, uint32_t spacing, uint32_t padding, bool is_vertical) {
    if (widget == NULL || !widget->active) return;

    uint32_t total_w = 0;
    uint32_t total_h = 0;

    // first pass: calculate total width and height
    t_widget *child = widget->children;
    while (child != NULL) {
        if (child->active) {
            if (is_vertical) {
                total_w = (child->width > total_w) ? child->width : total_w;
                total_h += child->height;
                if (child->next != NULL) {
                    total_h += spacing;
                }
            } else {
                total_h = (child->height > total_h) ? child->height : total_h;
                total_w += child->width;
                if (child->next != NULL) {
                    total_w += spacing;
                }
            }
        }
        child = child->next;
    }

    uint32_t current_x = 0;
    uint32_t current_y = 0;

    if (is_vertical) {
        if (widget->v_align == ALIGN_CENTER) {
            current_y = (widget->height > total_h) ? (widget->height - total_h) / 2 : 0;
        } else if (widget->v_align == ALIGN_END) {
            current_y = (widget->height > total_h) ? widget->height - total_h - padding : 0;
        } else {
            current_y = padding;
        }
    } else {
        if (widget->h_align == ALIGN_CENTER) {
            current_x = (widget->width > total_w) ? (widget->width - total_w) / 2 : 0;
        } else if (widget->h_align == ALIGN_END) {
            current_x = (widget->width > total_w) ? widget->width - total_w - padding : 0;
        } else {
            current_x = padding;
        }
    }

    // second pass: set positions
    child = widget->children;
    while (child != NULL) {
        if (child->active) {
            if (is_vertical) {
                child->y = current_y;
                if (child->h_align == ALIGN_CENTER) {
                    child->x = (widget->width > child->width) ? (widget->width - child->width) / 2 : 0;
                } else if (child->h_align == ALIGN_END) {
                    child->x = (widget->width > child->width) ? widget->width - child->width - padding : 0;
                } else {
                    child->x = padding;
                }
                current_y += child->height + spacing;
            } else {
                child->x = current_x;
                if (child->v_align == ALIGN_CENTER) {
                    child->y = (widget->height > child->height) ? (widget->height - child->height) / 2 : 0;
                } else if (child->v_align == ALIGN_END) {
                    child->y = (widget->height > child->height) ? widget->height - child->height - padding : 0;
                } else {
                    child->y = padding;
                }
                current_x += child->width + spacing;
            }
        }
        child = child->next;
    }

    child = widget->children;
    while (child != NULL) {
        // hardcoded spacing and padding - look into child laater
        widget_layout(child, 10, 10, true);
        child = child->next;
    }
}

t_widget* widget_create_start_menu(t_gui *gui, uint32_t screen_width, uint32_t screen_height) {
    t_widget *root = widget_create(CANVAS, 0, 0, screen_width, screen_height);
    if (!root) return NULL;
    root->data.canvas.state = gui;
    root->h_align = ALIGN_CENTER;
    root->v_align = ALIGN_CENTER;

    t_widget *btn_single = widget_create(BUTTON, 0, 0, 300, 50);
    btn_single->data.button.label = "Singleplayer";
    btn_single->on_click = on_btn_singleplayer_click;
    btn_single->h_align = ALIGN_CENTER;

    t_widget *btn_multi = widget_create(BUTTON, 0, 0, 300, 50);
    btn_multi->data.button.label = "Multiplayer";
    btn_multi->on_click = on_btn_multiplayer_click;
    btn_multi->h_align = ALIGN_CENTER;

    t_widget *btn_score = widget_create(BUTTON, 0, 0, 300, 50);
    btn_score->data.button.label = "Scoreboard";
    btn_score->on_click = on_btn_scoreboard_click;
    btn_score->h_align = ALIGN_CENTER;

    widget_add_child(root, btn_single);
    widget_add_child(root, btn_multi);
    widget_add_child(root, btn_score);

    widget_layout(root, 30, 100, true);

    return root;
}

t_widget* widget_create_name_menu(t_gui *gui, uint32_t screen_width, uint32_t screen_height, bool is_multiplayer) {
    t_widget *root = widget_create(CANVAS, 0, 0, screen_width, screen_height);
    if (!root) return NULL;
    root->data.canvas.state = gui;
    root->h_align = ALIGN_CENTER;
    root->v_align = ALIGN_CENTER;

    t_widget *dlg_prompt = widget_create(DIALOG, 0, 0, 400, 300);
    dlg_prompt->data.dialog.title = is_multiplayer ? "Enter Player Names" : "Enter Player Name";
    dlg_prompt->h_align = ALIGN_CENTER;
    dlg_prompt->v_align = ALIGN_CENTER;
    dlg_prompt->data.canvas.state = gui;

    t_widget *input_p1 = widget_create(TEXT_INPUT, 0, 0, 300, 40);
    input_p1->data.text_input.buffer = (char*)malloc(256);
    memset(input_p1->data.text_input.buffer, 0, 256);
    strcpy(input_p1->data.text_input.buffer, "Player 1");
    input_p1->data.text_input.max_length = 255;
    input_p1->on_click = on_text_input_click;
    input_p1->h_align = ALIGN_CENTER;

    widget_add_child(dlg_prompt, input_p1);

    if (is_multiplayer) {
        t_widget *input_p2 = widget_create(TEXT_INPUT, 0, 0, 300, 40);
        input_p2->data.text_input.buffer = (char*)malloc(256);
        memset(input_p2->data.text_input.buffer, 0, 256);
        strcpy(input_p2->data.text_input.buffer, "Player 2");
        input_p2->data.text_input.max_length = 255;
        input_p2->on_click = on_text_input_click;
        input_p2->h_align = ALIGN_CENTER;
        widget_add_child(dlg_prompt, input_p2);
    }

    t_widget *btn_start = widget_create(BUTTON, 0, 0, 150, 40);
    btn_start->data.button.label = "Start";
    btn_start->on_click = on_btn_start_game_click;
    btn_start->h_align = ALIGN_CENTER;

    widget_add_child(dlg_prompt, btn_start);
    widget_layout(dlg_prompt, 20, 40, true);

    widget_add_child(root, dlg_prompt);
    widget_layout(root, 0, 0, true);

    return root;
}
