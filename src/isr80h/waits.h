#ifndef WAITS_H
#define WAITS_H

#include "rtc/rtc.h"
#include "task/task.h"
#include "kernel.h"

struct interrupt_frame;
void *isr80h_command10_sleep(struct interrupt_frame *frame);

#endif