#ifndef ISR80H_TIMERS_H
#define ISR80H_TIMERS_H

// Timer-related system call interface for ISR80H
#include "drivers/io/timing/timer.h"
#include "task/task.h"
#include "kernel.h"

struct interrupt_frame;
void *isr80h_command_sleep(struct interrupt_frame *frame);
void *isr80h_command_usleep(struct interrupt_frame *frame);
void *isr80h_command_nanosleep(struct interrupt_frame *frame);

#endif