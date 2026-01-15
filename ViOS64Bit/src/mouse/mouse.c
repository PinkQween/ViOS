#include "mouse/mouse.h"
#include "mouse/ps2mouse.h"
#include "lib/vector/vector.h"
#include "graphics/graphics.h"
#include "graphics/window.h"
#include "kernel.h"
#include "status.h"

// Holds all the loaded mouse drivers
struct vector* mouse_driver_vector = NULL;

int mouse_system_load_static_drivers()
{
    int res = 0;
    res = mouse_register(ps2_mouse_get());
    if (res < 0)
    {
        goto out;
    }

out:
    return res;
}

int mouse_system_init()
{
    int res = 0;
    mouse_driver_vector = vector_new(sizeof(struct mouse*), 4, 0);
    if (!mouse_driver_vector)
    {
        res = -ENOMEM;
        goto out;
    }

out:   
    return res;
}

void mouse_draw_default_impl(struct mouse* mouse)
{
    struct framebuffer_pixel pixel_color = {0};
    pixel_color.red = 0xf3;
    
    struct terminal* win_term = window_terminal(mouse->graphic.window);
    terminal_draw_rect(win_term, 0, 0, win_term->bounds.width, win_term->bounds.height, pixel_color);
    
    // Redraw the mouse window to screen
    struct window* win = mouse->graphic.window;
    if (win && win->root_graphics) {
        graphics_redraw_graphics_to_screen(win->root_graphics, 0, 0, win->root_graphics->width, win->root_graphics->height);
    }
}

int mouse_register(struct mouse* mouse)
{
    int res = 0;
    if (!mouse_driver_vector)
    {
        panic("Mouse system was not initialized yet\n");
    }

    if (!mouse)
    {
        res = -EINVARG;
        goto out;
    }

    mouse->event_handlers.click_handlers = vector_new(sizeof(MOUSE_CLICK_EVENT_HANDLER_FUNCTION), 4, 0);
    if (!mouse->event_handlers.click_handlers)
    {
        res = -ENOMEM;
        goto out;
    }

    mouse->event_handlers.move_handlers = vector_new(sizeof(MOUSE_MOVE_EVENT_HANDLER_FUNCTION), 4, 0);
    if (!mouse->event_handlers.move_handlers)
    {
        res = -ENOMEM;
        goto out;
    }

    res = mouse->init(mouse);
    if (res < 0)
    {
        goto out;
    }

    struct graphics_info* screen_graphics = graphics_screen_info();
    mouse->coords.x = screen_graphics->width / 2;
    mouse->coords.y = screen_graphics->height / 2;
    if (!mouse->draw)
    {
        mouse->draw = mouse_draw_default_impl;
        mouse->graphic.width = MOUSE_GRAPHIC_DEFAULT_WIDTH;
        mouse->graphic.height = MOUSE_GRAPHIC_DEFAULT_HEIGHT;
    }

    if (mouse->graphic.width <= 0 ||
        mouse->graphic.height <= 0)
    {
        res = -EINVARG;
        goto out;
    }

    if (!mouse->graphic.window)
    {
        #include "kernel.h"
        kernel_debug_log("[mouse_register] before window_create\n");
        mouse->graphic.window = window_create(screen_graphics, NULL, "", mouse->coords.x, mouse->coords.y, mouse->graphic.width, mouse->graphic.height, WINDOW_FLAG_BACKGROUND_TRANSPARENT | WINDOW_FLAG_BORDERLESS | WINDOW_FLAG_CLICK_THROUGH, -1);
        kernel_debug_log("[mouse_register] after window_create\n");
        window_set_z_index(mouse->graphic.window, MOUSE_GRAPHIC_ZINDEX);
        kernel_debug_log("[mouse_register] after window_set_z_index\n");
    }

    mouse->draw(mouse);
    vector_push(mouse_driver_vector, &mouse);
out:
    return res;
}

void mouse_position_set(struct mouse* mouse, size_t x, size_t y)
{
    mouse->coords.x = x;
    mouse->coords.y = y;
    window_position_set(mouse->graphic.window, x, y);
}

void mouse_click(struct mouse* mouse, MOUSE_CLICK_TYPE type)
{
    // Loop through every click handler and invoke it
    size_t total_click_handlers = vector_count(mouse->event_handlers.click_handlers);
    for(size_t i = 0; i < total_click_handlers; i++)
    {
        MOUSE_CLICK_EVENT_HANDLER_FUNCTION click_handler = NULL;
        vector_at(mouse->event_handlers.click_handlers, i, &click_handler, sizeof(click_handler));
        if (click_handler)
        {
            click_handler(mouse, mouse->coords.x, mouse->coords.y, type);
        }
    }
}

void mouse_moved(struct mouse* mouse)
{
    size_t total_move_handlers = vector_count(mouse->event_handlers.move_handlers);
    for(size_t i = 0; i < total_move_handlers; i++)
    {
        MOUSE_MOVE_EVENT_HANDLER_FUNCTION move_handler = NULL;
        vector_at(mouse->event_handlers.move_handlers, i, &move_handler, sizeof(move_handler));
        if(move_handler)
        {
            move_handler(mouse, mouse->coords.x, mouse->coords.y);
        }
    }
}

void mouse_unregister_move_handler(struct mouse* mouse, MOUSE_MOVE_EVENT_HANDLER_FUNCTION move_handler)
{
    if (mouse)
    {
        vector_pop_element(mouse->event_handlers.move_handlers, &move_handler, sizeof(move_handler));
        return;
    }

    size_t total_mice = vector_count(mouse_driver_vector);
    if (total_mice == 0)
    {
        panic("NO Mice driers are registered\n");
    }

    for(size_t i = 0; i < total_mice; i++)
    {
        struct mouse* _mouse = NULL;
        vector_at(mouse_driver_vector, i, &_mouse, sizeof(_mouse));
        if (_mouse)
        {
            mouse_unregister_move_handler(_mouse, move_handler);
        }
    }
}



void mouse_unregister_click_handler(struct mouse* mouse, MOUSE_CLICK_EVENT_HANDLER_FUNCTION click_handler)
{
    if (mouse)
    {
        vector_pop_element(mouse->event_handlers.click_handlers, &click_handler, sizeof(click_handler));
        return;
    }

    size_t total_mice = vector_count(mouse_driver_vector);
    if (total_mice == 0)
    {
        panic("NO mice drivers are reigstered\n");
    }

    for(size_t i = 0; i < total_mice; i++)
    {
        struct mouse* _mouse = NULL;
        vector_at(mouse_driver_vector, i, &_mouse, sizeof(_mouse));
        if (_mouse)
        {
            mouse_unregister_click_handler(_mouse, click_handler);
        }
    }
}

void mouse_register_move_handler(struct mouse* mouse, MOUSE_MOVE_EVENT_HANDLER_FUNCTION move_handler)
{
    if (mouse)
    {
        vector_push(mouse->event_handlers.move_handlers, &move_handler);
        return;
    }

    size_t total_mice = vector_count(mouse_driver_vector);
    if(total_mice == 0)
    {
        panic("NO Mice drivers are registered\n");
    }

    for(size_t i = 0; i < total_mice; i++)
    {
        struct mouse* _mouse = NULL;
        vector_at(mouse_driver_vector, i, &_mouse, sizeof(_mouse));
        if (_mouse)
        {
            mouse_register_move_handler(_mouse, move_handler);
        }
    }
}

void mouse_register_click_handler(struct mouse* mouse, MOUSE_CLICK_EVENT_HANDLER_FUNCTION click_handler)
{
    if(mouse)
    {
        vector_push(mouse->event_handlers.click_handlers, &click_handler);
        return;
    }

    size_t total_mice = vector_count(mouse_driver_vector);
    if (total_mice == 0)
    {
        panic("NO mice drivers installed\n");
    }

    for(size_t i = 0; i < total_mice; i++)
    {
        struct mouse* _mouse = NULL;
        vector_at(mouse_driver_vector, i, &_mouse, sizeof(_mouse));
        if (_mouse)
        {
            mouse_register_click_handler(_mouse, click_handler);
        }
    }
}

void mouse_draw(struct mouse* mouse)
{
    mouse->draw(mouse);
}