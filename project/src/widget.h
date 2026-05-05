#ifndef LCOM_PROJECT_WIDGET_H
#define LCOM_PROJECT_WIDGET_H

#include <stdint.h>
#include <stdbool.h>
#include "../lib/vbe/vbe.h"

// Forward declaration
// Win95 16-bit RGB 5:6:5 Color Palette
#define W95_TEAL       0x0410 
#define W95_GRAY       0xC618 
#define W95_LIGHT_GRAY 0xDEFB 
#define W95_DARK_GRAY  0x8410 
#define W95_WHITE      0xFFFF 
#define W95_BLACK      0x0000 

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
    CANVAS,
    GAME,
    OVERLAY
} e_widget_type;

struct s_game_state;

typedef enum {
    WIDGET_FLAG_ACTIVE  = 1u << 0,
    WIDGET_FLAG_CLICKED = 1u << 1,
    WIDGET_FLAG_HOVERED = 1u << 2,
    WIDGET_FLAG_FOCUSED = 1u << 4,
    WIDGET_FLAG_NO_LAYOUT = 1u << 5
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

    char            *name;

    struct s_widget *parent;
    struct s_widget *children;
    struct s_widget *next;
    struct s_widget *prev;

    uint32_t        flags;

    union {

        struct {
            struct s_game_state *state;
        } game;

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
            int32_t     selection_start; // -1 if no selection
            bool        cursor_visible;
            uint32_t    blink_timer;
        } text_input;

        struct {
            char            *title;
            struct s_widget *close_button;
        } dialog;
    } data;

    void (*draw)(struct s_widget *self, hw_video_t *video);
    void (*on_click)(struct s_widget *self, void *state);
    void (*on_press)(struct s_widget *self, void *state);
    void (*on_drag)(struct s_widget *self, void *state);
    void (*on_hover)(struct s_widget *self, void *state);
    void (*on_key_press)(struct s_widget *self, uint8_t scancode, void *state);
    void (*on_tick)(struct s_widget *self, void *state);
    void (*on_quit)(struct s_widget *self, void *state);

    // Lifecycle hook for cleanup
    void (*on_destroy)(struct s_widget *self);

} t_widget;

t_widget*   widget_create(e_widget_type type, int32_t x, int32_t y, uint32_t width, uint32_t height, const char *name);
void        widget_add_child(t_widget *parent, t_widget *child);
void        widget_destroy(t_widget *widget);
t_widget*   widget_set_position(t_widget *widget, int32_t x, int32_t y);
t_widget*   widget_get_at(t_widget *root, int32_t x, int32_t y);
t_widget*   widget_find_by_name(t_widget *root, const char *name);
int32_t     get_abs_x(t_widget *widget);
int32_t     get_abs_y(t_widget *widget);

void        draw_win95_border(hw_video_t *video, int32_t x, int32_t y, uint16_t w, uint16_t h, bool sunken);

void        widget_draw(t_widget *widget, hw_video_t *video);
void        widget_tick(t_widget *widget, void *state);
t_widget*   widget_first_focusable(t_widget *root);
void        draw_canvas(struct s_widget *self, hw_video_t *video);
void        draw_button(struct s_widget *self, hw_video_t *video);
void        draw_text(struct s_widget *self, hw_video_t *video);
void        draw_text_input(struct s_widget *self, hw_video_t *video);
void        draw_dialog(struct s_widget *self, hw_video_t *video);
void        draw_game_canvas(struct s_widget *self, hw_video_t *video);

// --- BUILDER HELPERS ---
t_widget* widget_add_button(t_widget *parent, int32_t x, int32_t y, uint32_t w, uint32_t h, const char *label, void (*on_click)(t_widget*, void*), const char *name);
t_widget* widget_add_text_input(t_widget *parent, int32_t x, int32_t y, uint32_t w, uint32_t h, const char *default_text, void (*on_click)(t_widget*, void*), const char *name);
t_widget* widget_add_text(t_widget *parent, int32_t x, int32_t y, uint32_t w, uint32_t h, const char *text, const char *name);

// --- OVERLAY / DIALOG HELPERS ---
t_widget* widget_add_dialog(t_widget *parent, const char *title, uint32_t w, uint32_t h, uint32_t screen_w, uint32_t screen_h, void (*on_close)(t_widget*, void*), const char *name);


typedef void (*widget_draw_func)(t_widget*, hw_video_t*);
extern widget_draw_func default_draw_funcs[];

#endif /* LCOM_PROJECT_WIDGET_H */
