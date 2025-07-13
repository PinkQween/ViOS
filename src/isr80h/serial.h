#ifndef ISR80H_IO_H
#define ISR80H_IO_H

#include "debug/simple_serial.h"
#include "io.h"
#include "task/task.h"

struct interrupt_frame;
void *isr80h_command4_putchar(struct interrupt_frame *frame);
void *isr80h_command5_print_serial(struct interrupt_frame *frame);
void *isr80h_command6_print_char_serial(struct interrupt_frame *frame);

#endif