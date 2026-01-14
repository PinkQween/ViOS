#ifndef  KERNEL_PS2_MOUSE_H
#define KERNEL_PS2_MOUSE_H

#include "mouse/mouse.h"
#include <stdint.h>

#define PS2_COMMUNICATION_PORT 0x60
#define ISR_MOUSE_INTERRUPT 0x2c
#define PS2_STATUS_PORT 0x64
#define PS2_COMMAND_ENABLE_SECOND_PORT 0xA8
#define PS2_WRITE_TO_MOUSE 0xD4
#define PS2_READ_CONFIG_BYTE 0x20
#define PS2_UPDATE_CONFIG_BYTE 0x60
#define PS2_MOUSE_ENABLE_PACKET_STREAMING 0xF4
#define PS2_WAIT_FOR_INPUT_TO_CLEAR 0
#define PS2_WAIT_FOR_OUTPUT_TO_BE_SET 1

#define PS2_MOUSE_RESET 0xFF
#define PS2_MOUSE_ACK 0xFA
#define PS2_MOUSE_SELF_TEST_PASS 0xAA

#define PS2_STANDARD_MOUSE_DEVICE_ID 0x00
#define PS2_SCROLL_WHEEL_MOUSE_DEVICE_ID 0x03

struct ps2_mouse
{
    uint8_t device_id;
    int mouse_packet_size;
};

struct mouse* ps2_mouse_get();

#endif