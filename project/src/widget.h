#ifndef _WIDGET_H_
#define _WIDGET_H_

#include <stdint.h>
#include <stdbool.h>
#include "../lib/vbe/vbe.h"

typedef enum {
    DIALOG,
    BUTTON,
    TEXT,
    TEXT_INPUT,
    CANVAS
} e_widget_type;

typedef struct s_widget {
    e_widget_type   type;
    
    uint32_t        x, y;
    uint32_t        height, width;

    struct s_widget *parent;
    struct s_widget *children;
    struct s_widget *next;
    struct s_widget *prev;

    bool            active;
    bool            is_clicked;
    bool            hovered;
    bool            hittable;

    union {
        struct {
            char *label;
        } button;
        
        struct {
            char *text;
        } text_display;
        
        struct {
            char *buffer;
            uint32_t max_length;
            uint32_t cursor_pos;
        } text_input;

        struct {
            char *title;
        } dialog;
        
        struct {
            void *state;
        } canvas;
    } data;

    void (*draw)(struct s_widget *self, hw_video_t *video);
    void (*on_click)(struct s_widget *self);
    void (*on_hover)(struct s_widget *self);
    void (*on_key_press)(struct s_widget *self, uint8_t scancode);
    void (*on_tick)(struct s_widget *self, void *data);

} t_widget;

t_widget*   widget_create(e_widget_type type, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
void        widget_add_child(t_widget *parent, t_widget *child);
void        widget_destroy(t_widget *widget);
void        widget_hide(t_widget *widget);
t_widget*   widget_get_at(t_widget *root, uint32_t x, uint32_t y);

#endif /* _WIDGET_H_ */
