#ifndef ISR80H_TIMERS_H
#define ISR80H_TIMERS_H

// Timer-related system call interface for ISR80H
#include "drivers/io/timing/timer.h"
#include "task/task.h"
#include "kernel.h"

struct interrupt_frame;
void *isr80h_command10_sleep(struct interrupt_frame *frame);
void *isr80h_command12_usleep(struct interrupt_frame *frame);
void *isr80h_command13_nanosleep(struct interrupt_frame *frame);

#endif