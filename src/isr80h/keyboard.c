#include "keyboard.h"

void *isr80h_command_getkey(struct interrupt_frame *frame)
{
    char c = keyboard_pop();
    return (void *)((int)c);
}