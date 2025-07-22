#include "mouse.h"
#include "task/process.h"
#include "debug/simple_serial.h"

struct mouse *mouse_list_head = 0;
struct mouse *mouse_list_last = 0;
struct mouse *global_mouse = 0; // Global mouse pointer for direct access

int mouse_insert(struct mouse *mouse)
{
    if (!mouse || !mouse->init)
        return -1;

    if (mouse_list_last)
    {
        mouse_list_last->next = mouse;
        mouse_list_last = mouse;
    }
    else
    {
        mouse_list_head = mouse;
        mouse_list_last = mouse;
    }

    // Set as primary global mouse if this is the first one
    if (!global_mouse)
        global_mouse = mouse;

    return mouse->init(mouse);
}

void mouse_init()
{
    simple_serial_puts("DEBUG: mouse_init() called\n");
    extern struct mouse *ps2_mouse_init(); // Provided by your PS/2 driver
    struct mouse *ps2_mouse = ps2_mouse_init();
    simple_serial_puts("DEBUG: Got PS/2 mouse instance\n");
    int result = mouse_insert(ps2_mouse);
    simple_serial_puts("DEBUG: mouse_insert result: ");
    simple_serial_putc('0' + (result & 0xF));
    simple_serial_putc('\n');
    simple_serial_puts("DEBUG: mouse_init() complete\n");
}

void mouse_handle_interrupt()
{
    simple_serial_puts("DEBUG: mouse_handle_interrupt() called\n");
    if (mouse_list_head && mouse_list_head->handle_packet)
    {
        mouse_list_head->handle_packet(mouse_list_head);
    }
}

void mouse_set_position(int x, int y)
{
    simple_serial_puts("DEBUG: set_mouse_position() called\n");
    if (global_mouse && global_mouse->set_position)
    {
        global_mouse->set_position(x, y);
    }
}

void mouse_set_speed(float speed_multiplier)
{
    simple_serial_puts("DEBUG: set_mouse_speed() called\n");
    if (global_mouse && global_mouse->set_speed)
    {
        global_mouse->set_speed(speed_multiplier);
    }
}