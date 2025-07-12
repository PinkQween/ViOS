#ifndef PS2_MOUSE_H
#define PS2_MOUSE_H

#define PS2_PORT 0x64
#define PS2_COMMAND_ENABLE_SECOND_PORT 0xA8
#define MOUSE_INPUT_PORT 0x60
#define ISR_MOUSE_INTERRUPT 0x2C

struct mouse *ps2_mouse_init();

#endif