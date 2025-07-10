#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

struct mouse;

typedef void (*MOUSE_HANDLE_PACKET_FUNCTION)(struct mouse *mouse);
typedef int (*MOUSE_INIT_FUNCTION)(struct mouse *mouse);

struct mouse
{
    MOUSE_INIT_FUNCTION init;
    MOUSE_HANDLE_PACKET_FUNCTION handle_packet;
    char name[20];

    int x, y;
    int left, middle, right;

    struct mouse *next;
};

void mouse_init();
int mouse_insert(struct mouse *mouse);
void mouse_handle_interrupt();

#endif
