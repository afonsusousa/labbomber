#ifndef _WIDGET_H_
#define _WIDGET_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    DIALOG,
    BUTTON,
    TEXT,
    TEXT_INPUT
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
    } data;

    // Function pointers for polymorphic behavior
    void (*draw)(struct s_widget *self);
    void (*on_click)(struct s_widget *self);
    void (*on_key_press)(struct s_widget *self, uint8_t scancode);

} t_widget;

#endif /* _WIDGET_H_ */
