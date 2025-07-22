#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

// Forward declaration
struct mouse;

typedef void (*MOUSE_HANDLE_PACKET_FUNCTION)(struct mouse *mouse);
typedef int (*MOUSE_INIT_FUNCTION)(struct mouse *mouse);
typedef void (*MOUSE_SET_POSITION_FUNCTION)(int x, int y);
typedef void (*MOUSE_SET_SPEED_FUNCTION)(float speed_multiplier);

struct mouse
{
    MOUSE_INIT_FUNCTION init;
    MOUSE_HANDLE_PACKET_FUNCTION handle_packet;
    MOUSE_SET_POSITION_FUNCTION set_position;
    MOUSE_SET_SPEED_FUNCTION set_speed;
    char name[20];

    int x, y;
    int left, middle, right;

    struct mouse *next;
};

// Public interface
void mouse_init();
int mouse_insert(struct mouse *mouse);
void mouse_handle_interrupt();
void mouse_set_position(int x, int y);
void mouse_set_speed(float speed_multiplier);

// Expose the global current mouse state
extern struct mouse *global_mouse;

#endif // MOUSE_H