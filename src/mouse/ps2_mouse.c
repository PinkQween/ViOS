#include "mouse.h"
#include "io/io.h"
#include "idt/idt.h"
#include "task/task.h"
#include "kernel.h"
#include "graphics/graphics.h"

static struct mouse ps2_mouse;

// Forward declaration
void ps2_mouse_handle_packet(struct mouse *mouse);

// PS/2 constants
#define PS2_MOUSE_IRQ 12
#define PS2_COMMAND_PORT 0x64
#define PS2_DATA_PORT 0x60

static uint8_t packet[3];
static uint8_t packet_index = 0;

static int32_t mouse_x = 0;
static int32_t mouse_y = 0;
static int mouse_left = 0;
static int mouse_right = 0;
static int mouse_middle = 0;

void ps2_mouse_handle_interrupt()
{
    uint8_t data = insb(PS2_DATA_PORT);

    // Byte 1 must always have bit 3 set
    if (packet_index == 0 && !(data & 0x08))
        return;

    packet[packet_index++] = data;

    if (packet_index < 3)
        return;

    packet_index = 0;

    int dx = (int8_t)packet[1];
    int dy = (int8_t)packet[2];

    if (packet[0] & 0x10)
        dx |= 0xFFFFFF00;
    if (packet[0] & 0x20)
        dy |= 0xFFFFFF00;

    mouse_x += dx;
    mouse_y -= dy; // Y is inverted

    VBEInfoBlock *vbe = (VBEInfoBlock *)VBEInfoAddress;
    if (mouse_x < 0)
        mouse_x = 0;
    if (mouse_y < 0)
        mouse_y = 0;
    if (mouse_x >= vbe->x_resolution)
        mouse_x = vbe->x_resolution - 1;
    if (mouse_y >= vbe->y_resolution)
        mouse_y = vbe->y_resolution - 1;

    mouse_left = packet[0] & 0x01;
    mouse_right = packet[0] & 0x02;
    mouse_middle = packet[0] & 0x04;

    // Call handle_packet to update the struct
    ps2_mouse_handle_packet(&ps2_mouse);
}

int ps2_mouse_init_driver(struct mouse *mouse)
{
    outb(PS2_COMMAND_PORT, 0xA8); // Enable auxiliary device
    outb(PS2_COMMAND_PORT, 0x20); // Get current state
    uint8_t status = insb(PS2_DATA_PORT) | 2;
    outb(PS2_COMMAND_PORT, 0x60);
    outb(PS2_DATA_PORT, status);

    // Enable streaming packets
    outb(PS2_COMMAND_PORT, 0xD4);
    outb(PS2_DATA_PORT, 0xF4);
    insb(PS2_DATA_PORT); // ACK

    // Register interrupt handler
    // IRQ 12 maps to interrupt 0x2C after PIC remapping (0x28 + 4)
    idt_register_interrupt_callback(0x2C, ps2_mouse_handle_interrupt);

    // Enable IRQ 12 (mouse) on the slave PIC
    // Read current mask, clear bit 4 (IRQ 12), write back
    uint8_t mask = insb(0xA1);
    mask &= ~(1 << 4); // Clear bit 4 for IRQ 12
    outb(0xA1, mask);

    return 0;
}

void ps2_mouse_handle_packet(struct mouse *mouse)
{
    mouse->x = mouse_x;
    mouse->y = mouse_y;
    mouse->left = mouse_left;
    mouse->right = mouse_right;
    mouse->middle = mouse_middle;
}

struct mouse *ps2_mouse_init()
{
    ps2_mouse.init = ps2_mouse_init_driver;
    ps2_mouse.handle_packet = ps2_mouse_handle_packet;
    ps2_mouse.name[0] = 'P';
    ps2_mouse.name[1] = 'S';
    ps2_mouse.name[2] = '/';
    ps2_mouse.name[3] = '2';
    ps2_mouse.name[4] = '\0';

    return &ps2_mouse;
}
