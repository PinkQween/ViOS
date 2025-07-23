#ifndef IRS80H_KEYBOARD_H
#define IRS80H_KEYBOARD_H

#include "drivers/input/keyboard/keyboard.h"

struct interrupt_frame;
void *isr80h_command_getkey(struct interrupt_frame *frame);

#endif