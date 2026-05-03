#ifndef LCOM_PROJECT_WIDGET_H
#define LCOM_PROJECT_WIDGET_H

#include <stdint.h>
#include <stdbool.h>
#include "../lib/vbe/vbe.h"

// Forward declaration
struct s_widget;

typedef enum {
    ALIGN_START,
    ALIGN_CENTER,
    ALIGN_END
} e_alignment;

typedef enum {
    DIALOG,
    BUTTON,
    TEXT,
    TEXT_INPUT,
    CANVAS
} e_widget_type;

typedef enum {
    WIDGET_FLAG_ACTIVE  = 1u << 0,
    WIDGET_FLAG_CLICKED = 1u << 1,
    WIDGET_FLAG_HOVERED = 1u << 2,
    WIDGET_FLAG_FOCUSED = 1u << 4
} e_widget_flag;

#define WIDGET_HAS_FLAG(widget, flag) ((widget)->flags & (flag))
#define WIDGET_SET_FLAG(widget, flag) ((widget)->flags |= (flag))
#define WIDGET_UNSET_FLAG(widget, flag) ((widget)->flags &= ~(flag))
#define WIDGET_ASSIGN_FLAG(widget, flag, value) \
    ((widget)->flags = ((widget)->flags & ~(flag)) | ((value) ? (flag) : 0))

#define WIDGET_IS_ACTIVE(widget) WIDGET_HAS_FLAG((widget), WIDGET_FLAG_ACTIVE)
#define WIDGET_SET_ACTIVE(widget, value) WIDGET_ASSIGN_FLAG((widget), WIDGET_FLAG_ACTIVE, (value))

#define WIDGET_IS_CLICKED(widget) WIDGET_HAS_FLAG((widget), WIDGET_FLAG_CLICKED)
#define WIDGET_SET_CLICKED(widget, value) WIDGET_ASSIGN_FLAG((widget), WIDGET_FLAG_CLICKED, (value))

#define WIDGET_IS_HOVERED(widget) WIDGET_HAS_FLAG((widget), WIDGET_FLAG_HOVERED)
#define WIDGET_SET_HOVERED(widget, value) WIDGET_ASSIGN_FLAG((widget), WIDGET_FLAG_HOVERED, (value))

#define WIDGET_IS_FOCUSED(widget) WIDGET_HAS_FLAG((widget), WIDGET_FLAG_FOCUSED)
#define WIDGET_SET_FOCUSED(widget, value) WIDGET_ASSIGN_FLAG((widget), WIDGET_FLAG_FOCUSED, (value))

#define WIDGET_CAN_RECEIVE_FOCUS(widget) \
    ((widget) != NULL && ((widget)->type == BUTTON || (widget)->type == TEXT_INPUT))

typedef struct s_widget {
    e_widget_type   type;

    int32_t         x, y;
    uint32_t        height, width;

    e_alignment     h_align;
    e_alignment     v_align;

    struct s_widget *parent;
    struct s_widget *children;
    struct s_widget *next;
    struct s_widget *prev;

    uint32_t        flags;

    union {
        struct {
            char        *label;
        } button;
        
        struct {
            char        *text;
        } text_display;
        
        struct {
            char        *buffer;
            uint32_t    max_length;
            uint32_t    cursor_pos;
        } text_input;

        struct {
            char				*title;
            struct s_widget    *close_button;
        } dialog;
        
        struct {
            void *state;
        } canvas;
    } data;

    void (*draw)(struct s_widget *self, hw_video_t *video);
    void (*on_click)(struct s_widget *self, void *state);     // On mouse release
    void (*on_press)(struct s_widget *self, void *state);     // On mouse down
    void (*on_drag)(struct s_widget *self, void *state);      // On mouse move while down
    void (*on_hover)(struct s_widget *self, void *state);
    void (*on_key_press)(struct s_widget *self, uint8_t scancode, void *state);
    void (*on_tick)(struct s_widget *self, void *state);
    void (*on_quit)(struct s_widget *self, void *state);

} t_widget;

t_widget*   widget_create(e_widget_type type, int32_t x, int32_t y, uint32_t width, uint32_t height);
void        widget_add_child(t_widget *parent, t_widget *child);
void        widget_destroy(t_widget *widget);
t_widget*   widget_set_position(t_widget *widget, int32_t x, int32_t y);
t_widget*   widget_get_at(t_widget *root, int32_t x, int32_t y);
int32_t     widget_get_abs_x(t_widget *widget);
int32_t     widget_get_abs_y(t_widget *widget);

void        widget_draw(t_widget *widget, hw_video_t *video);
t_widget*   widget_find_first_focusable(t_widget *root);
void        draw_canvas(struct s_widget *self, hw_video_t *video);
void        draw_button(struct s_widget *self, hw_video_t *video);
void        draw_text(struct s_widget *self, hw_video_t *video);
void        draw_text_input(struct s_widget *self, hw_video_t *video);
void        draw_dialog(struct s_widget *self, hw_video_t *video);
void        draw_game_canvas(struct s_widget *self, hw_video_t *video);

typedef void (*widget_draw_func)(t_widget*, hw_video_t*);
extern widget_draw_func default_draw_funcs[];

#endif /* LCOM_PROJECT_WIDGET_H */
