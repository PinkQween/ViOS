#ifndef ISR80H_IO_H
#define ISR80H_IO_H

#include "debug/simple_serial.h"
#include "task/task.h"

struct interrupt_frame;
void *isr80h_command_putchar_serial(struct interrupt_frame *frame);
void *isr80h_command_print_serial(struct interrupt_frame *frame);

#endif