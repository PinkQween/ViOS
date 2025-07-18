#include "mouse.h"
#include "task/process.h"

static struct mouse *mouse_list_head = 0;
static struct mouse *mouse_list_last = 0;

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

    return mouse->init(mouse);
}

void mouse_init()
{
    extern struct mouse *ps2_mouse_init();
    mouse_insert(ps2_mouse_init());
}

void mouse_handle_interrupt()
{
    if (mouse_list_head && mouse_list_head->handle_packet)
    {
        mouse_list_head->handle_packet(mouse_list_head);
    }
}