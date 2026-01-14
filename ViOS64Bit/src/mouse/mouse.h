#ifndef KERNEL_MOUSE_H
#define KERNEL_MOUSE_H

#include "lib/vector/vector.h"

#define MOUSE_GRAPHIC_DEFAULT_WIDTH 10
#define MOUSE_GRAPHIC_DEFAULT_HEIGHT 10
#define MOUSE_GRAPHIC_ZINDEX 100000

enum
{
    MOUSE_NO_CLICK,
    MOUSE_LEFT_BUTTON_CLICKED,
    MOUSE_RIGHT_BUTTON_CLICKED,
    MOUSE_MIDDLE_BUTTON_CLICKED
};

typedef int MOUSE_CLICK_TYPE;

struct mouse;
typedef int (*MOUSE_INIT_FUNCTION)(struct mouse* mouse);
typedef void (*MOUSE_DRAW_FUNCTION)(struct mouse* mouse);

typedef void (*MOUSE_CLICK_EVENT_HANDLER_FUNCTION)(struct mouse* mouse, int clicked_x, int clicked_y, MOUSE_CLICK_TYPE type);
typedef void (*MOUSE_MOVE_EVENT_HANDLER_FUNCTION)(struct mouse* mouse, int moved_to_x, int moved_to_y);

struct window;
struct mouse
{
    MOUSE_INIT_FUNCTION init;
    MOUSE_DRAW_FUNCTION draw;
    char name[20];
    struct
    {
        // Current coordiantes where the mouse graphic is on the screen
        int x;
        int y;
    } coords;

    // Mouse graphics.
    struct
    {
        struct window* window;
        int width;
        int height;
    } graphic;

    struct
    {
        // Vector of MOUSE_CLICK_EVENT_HANDLER_FUNCTION
        struct vector* click_handlers;
        // Vctor of MOUSE_MOVE_EVENT_HANDLER_FUNCTION
        struct vector* move_handlers;
    } event_handlers;

    // THis is the private data for the mouse instance.
    void* private;
};

int mouse_system_load_static_drivers();
void mouse_draw(struct mouse* mouse);
void mouse_register_click_handler(struct mouse* mouse, MOUSE_CLICK_EVENT_HANDLER_FUNCTION click_handler);
void mouse_register_move_handler(struct mouse* mouse, MOUSE_MOVE_EVENT_HANDLER_FUNCTION move_handler);
void mouse_unregister_click_handler(struct mouse* mouse, MOUSE_CLICK_EVENT_HANDLER_FUNCTION click_handler);
void mouse_unregister_move_handler(struct mouse* mouse, MOUSE_MOVE_EVENT_HANDLER_FUNCTION move_handler);
void mouse_moved(struct mouse* mouse);
void mouse_click(struct mouse* mouse, MOUSE_CLICK_TYPE type);
void mouse_position_set(struct mouse* mouse, size_t x, size_t y);
int mouse_register(struct mouse* mouse);
int mouse_system_init();

#endif